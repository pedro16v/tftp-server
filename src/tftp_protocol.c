/*
 * tftp_protocol.c
 * TFTP Protocol Implementation - Packet parsing and building
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#include "tftp_protocol.h"
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

/* Forward declarations for internal functions */
static int parse_request_packet(const uint8_t *buffer, size_t len,
                                tftp_packet *packet);
static int parse_ack_packet(const uint8_t *buffer, size_t len,
                            tftp_packet *packet);
static int parse_data_packet(const uint8_t *buffer, size_t len,
                             tftp_packet *packet);
static int parse_error_packet(const uint8_t *buffer, size_t len,
                              tftp_packet *packet);

int tftp_parse_packet(const uint8_t *buffer, size_t len, tftp_packet *packet) {
    if (len < 2) {
        return 0;  /* Too short for opcode */
    }

    /* Parse opcode (network byte order) */
    packet->opcode = ntohs(*((uint16_t*)buffer));

    switch (packet->opcode) {
        case TFTP_OPCODE_RRQ:
        case TFTP_OPCODE_WRQ:
            return parse_request_packet(buffer + 2, len - 2, packet);

        case TFTP_OPCODE_ACK:
            return parse_ack_packet(buffer + 2, len - 2, packet);

        case TFTP_OPCODE_DATA:
            return parse_data_packet(buffer + 2, len - 2, packet);

        case TFTP_OPCODE_ERROR:
            return parse_error_packet(buffer + 2, len - 2, packet);

        default:
            return 0;  /* Unknown opcode */
    }
}

static int parse_request_packet(const uint8_t *buffer, size_t len,
                                tftp_packet *packet) {
    /* Format: [filename][0][mode][0] */

    const char *str = (const char*)buffer;
    size_t offset = 0;

    /* Extract filename */
    size_t filename_len = strnlen(str, len);
    if (filename_len >= len || filename_len >= sizeof(packet->payload.request.filename)) {
        return 0;
    }
    strncpy(packet->payload.request.filename, str,
            sizeof(packet->payload.request.filename) - 1);
    packet->payload.request.filename[sizeof(packet->payload.request.filename) - 1] = '\0';

    offset += filename_len + 1;
    if (offset >= len) {
        return 0;
    }

    /* Extract mode */
    str = (const char*)(buffer + offset);
    size_t mode_len = strnlen(str, len - offset);
    if (mode_len >= len - offset || mode_len >= sizeof(packet->payload.request.mode)) {
        return 0;
    }
    strncpy(packet->payload.request.mode, str,
            sizeof(packet->payload.request.mode) - 1);
    packet->payload.request.mode[sizeof(packet->payload.request.mode) - 1] = '\0';

    /* Convert mode to lowercase for comparison */
    for (size_t i = 0; packet->payload.request.mode[i]; i++) {
        packet->payload.request.mode[i] = tolower((unsigned char)packet->payload.request.mode[i]);
    }

    return 1;
}

static int parse_ack_packet(const uint8_t *buffer, size_t len,
                            tftp_packet *packet) {
    /* Format: [block#] */

    if (len < 2) {
        return 0;
    }

    packet->payload.ack.block_number = ntohs(*((uint16_t*)buffer));
    return 1;
}

static int parse_data_packet(const uint8_t *buffer, size_t len,
                             tftp_packet *packet) {
    /* Format: [block#][data] */

    if (len < 2) {
        return 0;
    }

    packet->payload.data.block_number = ntohs(*((uint16_t*)buffer));

    /* Copy data (may be 0 bytes for last packet) */
    size_t data_len = len - 2;
    if (data_len > TFTP_DATA_SIZE) {
        data_len = TFTP_DATA_SIZE;
    }

    if (data_len > 0) {
        memcpy(packet->payload.data.data, buffer + 2, data_len);
    }
    packet->payload.data.data_length = data_len;

    return 1;
}

static int parse_error_packet(const uint8_t *buffer, size_t len,
                              tftp_packet *packet) {
    /* Format: [error code][error message][0] */

    if (len < 2) {
        return 0;
    }

    packet->payload.error.error_code = ntohs(*((uint16_t*)buffer));

    /* Extract error message */
    if (len > 2) {
        size_t msg_len = strnlen((const char*)(buffer + 2), len - 2);
        if (msg_len > sizeof(packet->payload.error.error_message) - 1) {
            msg_len = sizeof(packet->payload.error.error_message) - 1;
        }
        memcpy(packet->payload.error.error_message, buffer + 2, msg_len);
        packet->payload.error.error_message[msg_len] = '\0';
    } else {
        packet->payload.error.error_message[0] = '\0';
    }

    return 1;
}

int tftp_build_data_packet(uint8_t *buffer, uint16_t block_num,
                          const uint8_t *data, size_t data_len) {
    /* Format: [opcode=3][block#][data] */

    if (data_len > TFTP_DATA_SIZE) {
        return 0;
    }

    *((uint16_t*)buffer) = htons(TFTP_OPCODE_DATA);
    *((uint16_t*)(buffer + 2)) = htons(block_num);

    if (data_len > 0) {
        memcpy(buffer + 4, data, data_len);
    }

    return 4 + data_len;
}

int tftp_build_ack_packet(uint8_t *buffer, uint16_t block_num) {
    /* Format: [opcode=4][block#] */

    *((uint16_t*)buffer) = htons(TFTP_OPCODE_ACK);
    *((uint16_t*)(buffer + 2)) = htons(block_num);

    return 4;
}

int tftp_build_error_packet(uint8_t *buffer, uint16_t error_code,
                           const char *error_msg) {
    /* Format: [opcode=5][error code][error message][0] */

    *((uint16_t*)buffer) = htons(TFTP_OPCODE_ERROR);
    *((uint16_t*)(buffer + 2)) = htons(error_code);

    size_t msg_len = strlen(error_msg);
    if (msg_len > 255) {
        msg_len = 255;
    }

    memcpy(buffer + 4, error_msg, msg_len);
    buffer[4 + msg_len] = '\0';

    return 4 + msg_len + 1;
}
