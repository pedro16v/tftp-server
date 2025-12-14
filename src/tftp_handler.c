/*
 * tftp_handler.c
 * TFTP Server Handler - Core server implementation
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#include "tftp_handler.h"
#include "tftp_protocol.h"
#include "tftp_file.h"
#include "tftp_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <limits.h>

/* Forward declarations */
static int tftp_main_loop(tftp_handler *handler);
static void setup_signal_handlers(tftp_handler *handler);
static void signal_handler(int signum);
static void handle_read_request(tftp_handler *handler, tftp_packet *packet,
                               struct sockaddr_in *client_addr, socklen_t addr_len);
static void handle_transfer(tftp_handler *handler, tftp_packet *packet,
                           struct sockaddr_in *client_addr, socklen_t addr_len);
static void send_file(int sockfd, FILE *file, struct sockaddr_in *client_addr,
                     socklen_t addr_len, int verbose, const char *filename);
static void send_error(int sockfd, struct sockaddr_in *client_addr,
                      socklen_t addr_len, uint16_t error_code, const char *msg);

/* Global handler pointer for signal handler */
static tftp_handler *g_handler = NULL;

int tftp_initialize(tftp_handler *handler) {
    memset(handler, 0, sizeof(tftp_handler));

    /* Create UDP socket */
    handler->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (handler->sockfd < 0) {
        tftp_error(handler, "Failed to create socket: %s", strerror(errno));
        return 0;
    }

    /* Set socket options (SO_REUSEADDR for development) */
    int reuse = 1;
    if (setsockopt(handler->sockfd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) < 0) {
        tftp_error(handler, "Failed to set SO_REUSEADDR: %s", strerror(errno));
        close(handler->sockfd);
        return 0;
    }

    return 1;
}

int tftp_start(tftp_handler *handler) {
    /* Bind socket to port */
    memset(&handler->server_addr, 0, sizeof(handler->server_addr));
    handler->server_addr.sin_family = AF_INET;
    handler->server_addr.sin_addr.s_addr = INADDR_ANY;
    handler->server_addr.sin_port = htons(handler->config.port);

    if (bind(handler->sockfd, (struct sockaddr*)&handler->server_addr,
             sizeof(handler->server_addr)) < 0) {
        tftp_error(handler, "Failed to bind to port %d: %s",
                  handler->config.port, strerror(errno));
        return 0;
    }

    tftp_log_verbose(handler, "[+] TFTP server listening on port %d\n",
                    handler->config.port);
    tftp_log_verbose(handler, "[+] Serving files from: %s\n",
                    handler->config.root_dir);

    /* Main server loop */
    handler->is_running = 1;
    return tftp_main_loop(handler);
}

int tftp_finalize(tftp_handler *handler) {
    if (handler->sockfd >= 0) {
        close(handler->sockfd);
        handler->sockfd = -1;
    }

    tftp_log_verbose(handler, "[+] Server shutdown complete\n");
    return 1;
}

static void signal_handler(int signum) {
    if (g_handler) {
        g_handler->is_running = 0;

        if (g_handler->config.is_verbose) {
            printf("\n[!] Received signal %d, shutting down...\n", signum);
        }
    }
}

static void setup_signal_handlers(tftp_handler *handler) {
    g_handler = handler;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Ignore SIGCHLD to prevent zombie processes */
    signal(SIGCHLD, SIG_IGN);
}

static int tftp_main_loop(tftp_handler *handler) {
    uint8_t buffer[TFTP_PACKET_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    ssize_t recv_len;
    tftp_packet packet;

    /* Signal handling for graceful shutdown */
    setup_signal_handlers(handler);

    while (handler->is_running) {
        client_addr_len = sizeof(client_addr);

        /* Receive packet with timeout using select */
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(handler->sockfd, &readfds);
        tv.tv_sec = 1;  /* 1 second timeout for signal checking */
        tv.tv_usec = 0;

        int activity = select(handler->sockfd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0 && errno != EINTR) {
            tftp_error(handler, "select() failed: %s", strerror(errno));
            return 0;
        }

        if (activity == 0) {
            continue;  /* Timeout, check is_running flag */
        }

        /* Receive packet */
        recv_len = recvfrom(handler->sockfd, buffer, sizeof(buffer), 0,
                           (struct sockaddr*)&client_addr, &client_addr_len);

        if (recv_len < 0) {
            if (errno == EINTR) continue;
            tftp_error(handler, "recvfrom() failed: %s", strerror(errno));
            continue;
        }

        /* Parse packet */
        if (!tftp_parse_packet(buffer, recv_len, &packet)) {
            tftp_log_verbose(handler, "[-] Failed to parse packet from %s\n",
                           inet_ntoa(client_addr.sin_addr));
            continue;
        }

        /* Handle packet based on opcode */
        switch (packet.opcode) {
            case TFTP_OPCODE_RRQ:
                handle_read_request(handler, &packet, &client_addr, client_addr_len);
                break;

            case TFTP_OPCODE_WRQ:
                /* Not implemented - send error */
                tftp_log_verbose(handler, "[-] WRQ from %s rejected (write not supported)\n",
                               inet_ntoa(client_addr.sin_addr));
                send_error(handler->sockfd, &client_addr, client_addr_len,
                          TFTP_ERROR_ILLEGAL_OP, "Write not supported");
                break;

            default:
                tftp_log_verbose(handler, "[-] Unexpected opcode %d from %s\n",
                               packet.opcode, inet_ntoa(client_addr.sin_addr));
                break;
        }
    }

    return 1;
}

static void handle_read_request(tftp_handler *handler, tftp_packet *packet,
                               struct sockaddr_in *client_addr, socklen_t addr_len) {

    tftp_log_verbose(handler, "[+] RRQ from %s:%d - file: %s, mode: %s\n",
                    inet_ntoa(client_addr->sin_addr),
                    ntohs(client_addr->sin_port),
                    packet->payload.request.filename,
                    packet->payload.request.mode);

    /* Validate transfer mode */
    if (strcmp(packet->payload.request.mode, TFTP_MODE_OCTET) != 0 &&
        strcmp(packet->payload.request.mode, TFTP_MODE_NETASCII) != 0) {
        tftp_log_verbose(handler, "[-] Invalid mode '%s'\n",
                        packet->payload.request.mode);
        send_error(handler->sockfd, client_addr, addr_len,
                  TFTP_ERROR_ILLEGAL_OP, "Invalid mode");
        return;
    }

    /* Fork a child process to handle this transfer */
    pid_t pid = fork();

    if (pid < 0) {
        tftp_error(handler, "fork() failed: %s", strerror(errno));
        send_error(handler->sockfd, client_addr, addr_len,
                  TFTP_ERROR_NOT_DEFINED, "Server error");
        return;
    }

    if (pid == 0) {
        /* Child process - handle the transfer */
        handle_transfer(handler, packet, client_addr, addr_len);
        exit(EXIT_SUCCESS);
    }

    /* Parent process continues */
}

static void handle_transfer(tftp_handler *handler, tftp_packet *packet,
                           struct sockaddr_in *client_addr, socklen_t addr_len) {

    /* Create new socket for this transfer (ephemeral port) */
    int transfer_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (transfer_sock < 0) {
        send_error(handler->sockfd, client_addr, addr_len,
                  TFTP_ERROR_NOT_DEFINED, "Server error");
        return;
    }

    /* Bind to ephemeral port (port 0) */
    struct sockaddr_in transfer_addr;
    memset(&transfer_addr, 0, sizeof(transfer_addr));
    transfer_addr.sin_family = AF_INET;
    transfer_addr.sin_addr.s_addr = INADDR_ANY;
    transfer_addr.sin_port = 0;  /* Let OS assign port */

    if (bind(transfer_sock, (struct sockaddr*)&transfer_addr,
             sizeof(transfer_addr)) < 0) {
        send_error(handler->sockfd, client_addr, addr_len,
                  TFTP_ERROR_NOT_DEFINED, "Server error");
        close(transfer_sock);
        return;
    }

    /* Validate and open file */
    char safe_path[PATH_MAX];
    if (!tftp_validate_path(handler->config.root_dir,
                           packet->payload.request.filename,
                           safe_path, sizeof(safe_path))) {
        tftp_log_verbose(handler, "[-] Access denied: %s\n",
                        packet->payload.request.filename);
        send_error(transfer_sock, client_addr, addr_len,
                  TFTP_ERROR_ACCESS_VIOLATION, "Access denied");
        close(transfer_sock);
        return;
    }

    FILE *file = fopen(safe_path, "rb");
    if (!file) {
        int error_code = (errno == ENOENT) ? TFTP_ERROR_FILE_NOT_FOUND
                                            : TFTP_ERROR_ACCESS_VIOLATION;
        tftp_log_verbose(handler, "[-] Failed to open %s: %s\n",
                        safe_path, strerror(errno));
        send_error(transfer_sock, client_addr, addr_len, error_code,
                  strerror(errno));
        close(transfer_sock);
        return;
    }

    /* Perform file transfer */
    send_file(transfer_sock, file, client_addr, addr_len,
             handler->config.is_verbose, packet->payload.request.filename);

    fclose(file);
    close(transfer_sock);
}

static void send_file(int sockfd, FILE *file, struct sockaddr_in *client_addr,
                     socklen_t addr_len, int verbose, const char *filename) {

    uint8_t send_buffer[TFTP_PACKET_SIZE];
    uint8_t recv_buffer[TFTP_PACKET_SIZE];
    uint8_t data_buffer[TFTP_DATA_SIZE];
    uint16_t block_number = 1;
    size_t bytes_read;
    int retries;
    size_t total_bytes = 0;

    while (1) {
        /* Read next block from file */
        bytes_read = fread(data_buffer, 1, TFTP_DATA_SIZE, file);

        /* Build DATA packet */
        int packet_len = tftp_build_data_packet(send_buffer, block_number,
                                               data_buffer, bytes_read);
        if (packet_len == 0) {
            if (verbose) {
                printf("[-] Failed to build DATA packet\n");
            }
            return;
        }

        retries = 0;
        while (retries < TFTP_MAX_RETRIES) {
            /* Send DATA packet */
            ssize_t sent = sendto(sockfd, send_buffer, packet_len, 0,
                                 (struct sockaddr*)client_addr, addr_len);

            if (sent < 0) {
                if (verbose) {
                    printf("[-] Send failed: %s\n", strerror(errno));
                }
                return;
            }

            if (verbose && retries == 0) {
                printf("[>] Sent block %d (%zu bytes)\n", block_number, bytes_read);
            }

            /* Wait for ACK with timeout */
            fd_set readfds;
            struct timeval tv;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            tv.tv_sec = TFTP_TIMEOUT_SECONDS;
            tv.tv_usec = 0;

            int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);

            if (activity < 0) {
                if (verbose) {
                    printf("[-] select() failed: %s\n", strerror(errno));
                }
                return;
            }

            if (activity == 0) {
                /* Timeout - retransmit */
                retries++;
                if (verbose) {
                    printf("[!] Timeout waiting for ACK %d (retry %d/%d)\n",
                          block_number, retries, TFTP_MAX_RETRIES);
                }
                continue;
            }

            /* Receive ACK */
            struct sockaddr_in ack_addr;
            socklen_t ack_addr_len = sizeof(ack_addr);
            ssize_t recv_len = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer),
                                       0, (struct sockaddr*)&ack_addr,
                                       &ack_addr_len);

            if (recv_len < 0) {
                if (verbose) {
                    printf("[-] recvfrom() failed: %s\n", strerror(errno));
                }
                return;
            }

            /* Verify it's from correct client */
            if (ack_addr.sin_addr.s_addr != client_addr->sin_addr.s_addr ||
                ack_addr.sin_port != client_addr->sin_port) {
                /* Wrong client - send error and ignore */
                send_error(sockfd, &ack_addr, ack_addr_len,
                          TFTP_ERROR_UNKNOWN_TID, "Unknown transfer ID");
                retries++;
                continue;
            }

            /* Parse ACK */
            tftp_packet ack_packet;
            if (!tftp_parse_packet(recv_buffer, recv_len, &ack_packet) ||
                ack_packet.opcode != TFTP_OPCODE_ACK) {
                if (verbose) {
                    printf("[-] Invalid ACK packet\n");
                }
                retries++;
                continue;
            }

            /* Verify block number */
            if (ack_packet.payload.ack.block_number != block_number) {
                if (verbose) {
                    printf("[!] Unexpected ACK block %d (expected %d)\n",
                          ack_packet.payload.ack.block_number, block_number);
                }
                retries++;
                continue;
            }

            if (verbose) {
                printf("[<] Received ACK %d\n", block_number);
            }

            /* ACK received successfully */
            break;
        }

        if (retries >= TFTP_MAX_RETRIES) {
            if (verbose) {
                printf("[-] Max retries exceeded for block %d\n", block_number);
            }
            return;
        }

        total_bytes += bytes_read;

        /* Check if transfer complete (last block < 512 bytes) */
        if (bytes_read < TFTP_DATA_SIZE) {
            if (verbose) {
                printf("[+] Transfer complete: %s (%zu bytes)\n",
                      filename, total_bytes);
            }
            break;
        }

        block_number++;
    }
}

static void send_error(int sockfd, struct sockaddr_in *client_addr,
                      socklen_t addr_len, uint16_t error_code, const char *msg) {
    uint8_t buffer[TFTP_PACKET_SIZE];
    int len = tftp_build_error_packet(buffer, error_code, msg);

    if (len > 0) {
        sendto(sockfd, buffer, len, 0, (struct sockaddr*)client_addr, addr_len);
    }
}
