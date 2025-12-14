/*
 * tftp_file.h
 * TFTP File Operations - Secure path validation and file access
 *
 * Copyright (c) 2025 Pedro
 * BSD 3-Clause License
 */

#ifndef TFTP_FILE_H
#define TFTP_FILE_H

#include <stddef.h>

/*
 * Validate and resolve file path
 * Prevents directory traversal attacks and ensures file is within root directory
 *
 * Parameters:
 *   root_dir: Root directory for file serving
 *   filename: Requested filename from client
 *   safe_path: Output buffer for validated path
 *   safe_path_len: Size of safe_path buffer
 *
 * Returns:
 *   1 if path is valid and safe
 *   0 if path is invalid or escapes root directory
 */
int tftp_validate_path(const char *root_dir, const char *filename,
                      char *safe_path, size_t safe_path_len);

#endif /* TFTP_FILE_H */
