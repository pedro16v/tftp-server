/*
 * tftp_options.h
 * TFTP Server Options - Command-line argument parsing
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#ifndef TFTP_OPTIONS_H
#define TFTP_OPTIONS_H

#include "tftp_handler.h"

/*
 * Parse command-line options and populate handler config
 * Returns 1 on success, 0 on failure
 */
int tftp_parse_options(tftp_handler *handler, int argc, char *argv[]);

/*
 * Validate that directory exists and is accessible
 * Returns 1 on success, 0 on failure
 */
int validate_directory(const char *path);

#endif /* TFTP_OPTIONS_H */
