# config.Makefile
# TFTP Server Build Configuration
#
# Copyright (c) 2025 Pedro
# BSD 3-Clause License

# Source files
SOURCES        := $(wildcard src/*.c)
EXECUTABLES    := src/tftp.c

# Compiler and linker flags
COMPILER_FLAGS := -Wall -std=c99 -D_POSIX_C_SOURCE=200809L
LINKER_FLAGS   :=
LIBRARIES      :=

# Build profiles
BUILD_PROFILES := release debug

# Release profile: optimized
release: COMPILER_FLAGS += -O2 -Os

# Debug profile: debugging symbols and sanitizers
debug:   COMPILER_FLAGS += -ggdb3 -Werror -pedantic -DDEBUG -fsanitize=address

# Test target
.PHONY: test
test: debug
	@echo "Running TFTP server tests..."
	./tftpd -h
	@echo ""
	@echo "To manually test:"
	@echo "  Terminal 1: ./tftpd -p 6969 -d . -v"
	@echo "  Terminal 2: echo 'test' > test.txt && tftp localhost 6969 -c get test.txt"
