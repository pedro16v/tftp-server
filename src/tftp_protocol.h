/*
 * tftp_protocol.h
 * TFTP Protocol Definitions - RFC 1350
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#ifndef TFTP_PROTOCOL_H
#define TFTP_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/* TFTP opcodes (RFC 1350) */
#define TFTP_OPCODE_RRQ   1  /* Read request */
#define TFTP_OPCODE_WRQ   2  /* Write request (not implemented) */
#define TFTP_OPCODE_DATA  3  /* Data packet */
#define TFTP_OPCODE_ACK   4  /* Acknowledgment */
#define TFTP_OPCODE_ERROR 5  /* Error packet */

/* TFTP error codes */
#define TFTP_ERROR_NOT_DEFINED      0  /* Not defined, see error message */
#define TFTP_ERROR_FILE_NOT_FOUND   1  /* File not found */
#define TFTP_ERROR_ACCESS_VIOLATION 2  /* Access violation */
#define TFTP_ERROR_DISK_FULL        3  /* Disk full or allocation exceeded */
#define TFTP_ERROR_ILLEGAL_OP       4  /* Illegal TFTP operation */
#define TFTP_ERROR_UNKNOWN_TID      5  /* Unknown transfer ID */
#define TFTP_ERROR_FILE_EXISTS      6  /* File already exists */
#define TFTP_ERROR_NO_SUCH_USER     7  /* No such user */

/* Transfer modes */
#define TFTP_MODE_NETASCII "netascii"
#define TFTP_MODE_OCTET    "octet"
#define TFTP_MODE_MAIL     "mail"  /* Obsolete, not implemented */

/* Protocol constants */
#define TFTP_DATA_SIZE         512
#define TFTP_MAX_RETRIES       5
#define TFTP_TIMEOUT_SECONDS   5
#define TFTP_DEFAULT_PORT      6969
#define TFTP_STANDARD_PORT     69

/*
 * TFTP packet structure
 */
typedef struct {
    uint16_t opcode;
    union {
        struct {
            char filename[256];
            char mode[16];
        } request;

        struct {
            uint16_t block_number;
            uint8_t data[TFTP_DATA_SIZE];
            size_t data_length;
        } data;

        struct {
            uint16_t block_number;
        } ack;

        struct {
            uint16_t error_code;
            char error_message[256];
        } error;
    } payload;
} tftp_packet;

/*
 * Parse packet from buffer
 * Returns 1 on success, 0 on failure
 */
int tftp_parse_packet(const uint8_t *buffer, size_t len, tftp_packet *packet);

/*
 * Build DATA packet
 * Returns packet length on success, 0 on failure
 */
int tftp_build_data_packet(uint8_t *buffer, uint16_t block_num,
                           const uint8_t *data, size_t data_len);

/*
 * Build ACK packet
 * Returns packet length on success, 0 on failure
 */
int tftp_build_ack_packet(uint8_t *buffer, uint16_t block_num);

/*
 * Build ERROR packet
 * Returns packet length on success, 0 on failure
 */
int tftp_build_error_packet(uint8_t *buffer, uint16_t error_code,
                            const char *error_msg);

#endif /* TFTP_PROTOCOL_H */
