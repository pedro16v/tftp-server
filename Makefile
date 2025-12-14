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
	@curl -sL https://raw.githubusercontent.com/cyrus-and/dry-makefile/master/Makefile -o $(DRY_MAKEFILE)

.PHONY: help
help:
	@echo "TFTP Server Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make           - Build release version"
	@echo "  make debug     - Build debug version with sanitizers"
	@echo "  make clean     - Clean build artifacts"
	@echo "  make test      - Build and run basic tests"
	@echo "  make help      - Show this help message"
