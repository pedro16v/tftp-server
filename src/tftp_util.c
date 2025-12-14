/*
 * tftp_util.c
 * TFTP Server Utilities - Implementation
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#include "tftp_util.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void tftp_print_error(tftp_handler *handler) {
    if (handler->error_buffer[0] != '\0') {
        fprintf(stderr, "Error: %s\n", handler->error_buffer);
    }
}

void tftp_print_usage(void) {
    printf("TFTP Server - RFC 1350 compliant read-only TFTP server\n\n");
    printf("Usage: tftpd [OPTIONS]\n\n");
    printf("Options:\n");
    printf("  -p PORT      Listen port (default: 6969)\n");
    printf("  -d DIR       Root directory for serving files (default: current directory)\n");
    printf("  -v           Verbose mode - show detailed logging\n");
    printf("  -h           Show this help message\n\n");
    printf("Examples:\n");
    printf("  tftpd -p 69 -d /tftpboot -v\n");
    printf("  tftpd -p 6969 -d ./files\n\n");
    printf("Note: Port 69 requires root privileges on most systems.\n");
    printf("      Use a port >= 1024 to run without root.\n");
}

void tftp_log_verbose(tftp_handler *handler, const char *format, ...) {
    if (!handler->config.is_verbose) {
        return;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

int parse_port(const char *str, int *port) {
    char *endptr;
    long val;

    errno = 0;
    val = strtol(str, &endptr, 10);

    /* Check for various parsing errors */
    if (errno != 0 || endptr == str || *endptr != '\0') {
        return 0;
    }

    /* Check port range */
    if (val < 1 || val > 65535) {
        return 0;
    }

    *port = (int)val;
    return 1;
}
