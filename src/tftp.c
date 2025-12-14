/*
 * tftp.c
 * TFTP Server - Main entry point
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#include "tftp_handler.h"
#include "tftp_options.h"
#include "tftp_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int tftp_error(tftp_handler *handler, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(handler->error_buffer, sizeof(handler->error_buffer), format, args);
    va_end(args);
    return 0;
}

int main(int argc, char *argv[]) {
    tftp_handler handler;

    /* Initialize handler to zero state */
    if (!tftp_initialize(&handler)) {
        tftp_print_error(&handler);
        return EXIT_FAILURE;
    }

    /* Parse command-line options */
    if (!tftp_parse_options(&handler, argc, argv)) {
        tftp_print_usage();
        tftp_print_error(&handler);
        return EXIT_FAILURE;
    }

    /* Start the TFTP server (main loop) */
    if (!tftp_start(&handler)) {
        tftp_print_error(&handler);
        tftp_finalize(&handler);
        return EXIT_FAILURE;
    }

    /* Cleanup and finalize */
    if (!tftp_finalize(&handler)) {
        tftp_print_error(&handler);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
