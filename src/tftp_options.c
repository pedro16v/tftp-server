/*
 * tftp_options.c
 * TFTP Server Options - Command-line parsing implementation
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#include "tftp_options.h"
#include "tftp_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int validate_directory(const char *path) {
    struct stat st;

    if (stat(path, &st) != 0) {
        return 0;  /* Doesn't exist */
    }

    if (!S_ISDIR(st.st_mode)) {
        return 0;  /* Not a directory */
    }

    /* Check if readable and executable (needed to access directory) */
    if (access(path, R_OK | X_OK) != 0) {
        return 0;  /* Not accessible */
    }

    return 1;
}

int tftp_parse_options(tftp_handler *handler, int argc, char *argv[]) {
    int opt;

    /* Set defaults */
    handler->config.port = 6969;  /* Non-privileged port by default */
    handler->config.root_dir = ".";
    handler->config.is_verbose = 0;

    while ((opt = getopt(argc, argv, "p:d:vh")) != -1) {
        switch (opt) {
            case 'p':  /* Port number */
                if (!parse_port(optarg, &handler->config.port)) {
                    tftp_error(handler, "Invalid port '%s'", optarg);
                    return 0;
                }
                break;

            case 'd':  /* Root directory */
                handler->config.root_dir = optarg;
                break;

            case 'v':  /* Verbose mode */
                handler->config.is_verbose = 1;
                break;

            case 'h':  /* Help */
                tftp_print_usage();
                exit(EXIT_SUCCESS);

            case '?':
                if (optopt == 'p' || optopt == 'd') {
                    tftp_error(handler, "Option -%c requires an argument", optopt);
                } else {
                    tftp_error(handler, "Unknown option -%c", optopt);
                }
                return 0;

            default:
                tftp_error(handler, "Unexpected getopt result");
                return 0;
        }
    }

    /* Validate root directory exists */
    if (!validate_directory(handler->config.root_dir)) {
        tftp_error(handler, "Directory '%s' does not exist or is not accessible",
                  handler->config.root_dir);
        return 0;
    }

    return 1;
}
