# Makefile
# TFTP Server - Download and use dry-makefile
#
# Copyright (c) 2025 Pedro
# BSD 3-Clause License
#
# Usage:
#   make           - Download dry-makefile if needed, then build release
#   make debug     - Build debug version with sanitizers
#   make clean     - Clean build artifacts
#   make test      - Build and run basic tests

# Default PREFIX for Homebrew compatibility
PREFIX ?= /usr/local
DESTDIR ?=

# Override INSTALL_PATH to prevent dry-makefile's install from being used
# Use override to ensure it can't be changed by dry-makefile
# Set it to an invalid path so if dry-makefile's install is accidentally used, it will fail
override INSTALL_PATH = /INVALID/PATH/TO/PREVENT/DRY/MAKEFILE/INSTALL

# Check if dry-makefile needs to be downloaded
DRY_MAKEFILE := .dry-makefile

# If dry-makefile exists, include it
ifneq ($(wildcard $(DRY_MAKEFILE)),)
    include $(DRY_MAKEFILE)
else
    # Download dry-makefile on first run
    $(info Downloading dry-makefile...)
    $(shell curl -sL https://raw.githubusercontent.com/cyrus-and/dry-makefile/master/Makefile -o $(DRY_MAKEFILE))
    include $(DRY_MAKEFILE)
endif

# Mark this file as up-to-date
$(DRY_MAKEFILE):
	@echo "Downloading dry-makefile..."
	@curl -sL https://raw.githubusercontent.com/cyrus-and/dry-makefile/master/Makefile -o $(DRY_MAKEFILE) || (echo "Error: Failed to download dry-makefile" && exit 1)
	@if [ ! -f $(DRY_MAKEFILE) ]; then \
		echo "Error: dry-makefile download failed"; \
		exit 1; \
	fi

# Install target for Homebrew (must override dry-makefile's install)
# This must be defined AFTER including dry-makefile to override its install target
.PHONY: install
install: release
	@if [ ! -f src/tftp ]; then \
		echo "Error: src/tftp not found. Build may have failed."; \
		exit 1; \
	fi
	@echo "Installing tftpd to $(DESTDIR)$(PREFIX)/bin/tftpd..."
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp src/tftp $(DESTDIR)$(PREFIX)/bin/tftpd
	@chmod +x $(DESTDIR)$(PREFIX)/bin/tftpd
	@echo "Installed to $(DESTDIR)$(PREFIX)/bin/tftpd"

.PHONY: help
help:
	@echo "TFTP Server Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make           - Build release version"
	@echo "  make debug     - Build debug version with sanitizers"
	@echo "  make clean     - Clean build artifacts"
	@echo "  make test      - Build and run basic tests"
	@echo "  make install   - Install tftpd to PREFIX/bin (default: /usr/local/bin)"
	@echo "  make help      - Show this help message"
