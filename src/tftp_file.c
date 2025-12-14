/*
 * tftp_file.c
 * TFTP File Operations - Implementation
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#include "tftp_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

int tftp_validate_path(const char *root_dir, const char *filename,
                      char *safe_path, size_t safe_path_len) {
    char temp_path[PATH_MAX];
    char root_canonical[PATH_MAX];

    /* Reject absolute paths */
    if (filename[0] == '/') {
        return 0;
    }

    /* Reject parent directory references */
    if (strstr(filename, "..") != NULL) {
        return 0;
    }

    /* Reject empty filename */
    if (filename[0] == '\0') {
        return 0;
    }

    /* Build full path */
    if (snprintf(temp_path, sizeof(temp_path), "%s/%s", root_dir, filename) >= sizeof(temp_path)) {
        return 0;  /* Path too long */
    }

    /* Resolve to canonical path (resolves symlinks and ..) */
    char *resolved = realpath(temp_path, safe_path);
    if (!resolved) {
        return 0;  /* Path doesn't exist or can't be resolved */
    }

    /* Get canonical path of root directory */
    if (!realpath(root_dir, root_canonical)) {
        return 0;
    }

    /* Ensure resolved path is still within root directory */
    size_t root_len = strlen(root_canonical);
    if (strncmp(safe_path, root_canonical, root_len) != 0) {
        return 0;  /* Path escapes root directory */
    }

    /* Additional check: next char must be '/' or '\0' */
    if (safe_path[root_len] != '\0' && safe_path[root_len] != '/') {
        return 0;
    }

    /* Verify file exists and is readable */
    if (access(safe_path, R_OK) != 0) {
        return 0;  /* File not readable */
    }

    return 1;
}
