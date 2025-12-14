/*
 * tftp_handler.h
 * TFTP Server Handler - Core state and lifecycle functions
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#ifndef TFTP_HANDLER_H
#define TFTP_HANDLER_H

#include <netinet/in.h>
#include <sys/socket.h>

#define TFTP_ERROR_BUFFER_SIZE 1024
#define TFTP_DATA_SIZE 512
#define TFTP_PACKET_SIZE 516  /* 2 (opcode) + 2 (block#) + 512 (data) */

/*
 * Main TFTP handler structure
 * Contains all server state and configuration
 */
typedef struct {
    struct {
        char *root_dir;           /* Root directory for file serving */
        int port;                 /* Server port (default 6969) */
        unsigned is_verbose:1;    /* Verbose logging flag */
    } config;

    int sockfd;                   /* UDP socket file descriptor */
    struct sockaddr_in server_addr;

    unsigned is_running:1;        /* Server running flag */

    char error_buffer[TFTP_ERROR_BUFFER_SIZE];
} tftp_handler;

/*
 * Lifecycle functions
 */
int tftp_initialize(tftp_handler *handler);
int tftp_start(tftp_handler *handler);
int tftp_finalize(tftp_handler *handler);

/*
 * Error handling
 */
int tftp_error(tftp_handler *handler, const char *format, ...);

#endif /* TFTP_HANDLER_H */
