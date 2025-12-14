/*
 * tftp_util.h
 * TFTP Server Utilities - Logging and helper functions
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#ifndef TFTP_UTIL_H
#define TFTP_UTIL_H

#include "tftp_handler.h"

/*
 * Print error message from handler error buffer
 */
void tftp_print_error(tftp_handler *handler);

/*
 * Print usage information
 */
void tftp_print_usage(void);

/*
 * Verbose logging (only prints if verbose mode enabled)
 */
void tftp_log_verbose(tftp_handler *handler, const char *format, ...);

/*
 * Parse port string to integer
 * Returns 1 on success, 0 on failure
 */
int parse_port(const char *str, int *port);

#endif /* TFTP_UTIL_H */
