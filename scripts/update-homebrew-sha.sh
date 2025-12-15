#!/bin/bash
# update-homebrew-sha.sh - Helper script to update Homebrew formula SHA256
#
# Usage: ./scripts/update-homebrew-sha.sh <version> <github-username>
# Example: ./scripts/update-homebrew-sha.sh 1.0.0 pedro

set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <version> <github-username>"
    echo "Example: $0 1.0.0 pedro"
    exit 1
fi

VERSION=$1
USERNAME=$2
URL="https://github.com/${USERNAME}/tftp-server/archive/refs/tags/v${VERSION}.tar.gz"

echo "Fetching SHA256 for version ${VERSION}..."
SHA256=$(curl -sL "${URL}" | shasum -a 256 | awk '{print $1}')

if [ -z "$SHA256" ]; then
    echo "Error: Could not fetch tarball. Make sure the release exists at:"
    echo "  ${URL}"
    exit 1
fi

echo "SHA256: ${SHA256}"
echo ""
echo "Update your Homebrew formula with:"
echo "  url \"${URL}\""
echo "  sha256 \"${SHA256}\""

