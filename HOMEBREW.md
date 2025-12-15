# Homebrew Tap Setup Guide

This guide will help you publish this TFTP server to your own Homebrew tap.

## Prerequisites

1. A GitHub account
2. Homebrew installed on your Mac
3. This project pushed to a GitHub repository

## Step 1: Create a GitHub Release

First, create a release on GitHub with a version tag (e.g., `v1.0.0`):

```bash
# Tag the current commit
git tag v1.0.0
git push origin v1.0.0

# Create a release on GitHub (via web interface or GitHub CLI)
gh release create v1.0.0 --title "v1.0.0" --notes "Initial release"
```

This will create a tarball at: `https://github.com/YOUR_USERNAME/tftp-server/archive/refs/tags/v1.0.0.tar.gz`

## Step 2: Calculate SHA256

Get the SHA256 checksum of the release tarball:

```bash
curl -sL https://github.com/YOUR_USERNAME/tftp-server/archive/refs/tags/v1.0.0.tar.gz | shasum -a 256
```

## Step 3: Create Homebrew Tap Repository

First, create a new repository on GitHub named `homebrew-<tapname>` (e.g., `homebrew-tools`):

```bash
# Create the repository on GitHub (using GitHub CLI)
gh repo create homebrew-tools --public --description "Homebrew tap for tools"

# Or create it manually via https://github.com/new
# Repository name must be: homebrew-tools
# Make it public
# Do NOT initialize with README, .gitignore, or license
```

Then create the tap locally and connect it to GitHub:

```bash
# Create the tap locally
brew tap-new YOUR_USERNAME/tools

# Navigate to the tap directory
cd $(brew --repository YOUR_USERNAME/tools)

# Add the remote and push to GitHub
git remote add origin https://github.com/YOUR_USERNAME/homebrew-tools.git
git push --set-upstream origin main
```

## Step 4: Create the Formula

Copy the formula template and update it:

```bash
# Copy the formula to your tap
cp Formula/tftpd.rb $(brew --repository YOUR_USERNAME/tools)/Formula/tftpd.rb

# Edit the formula
code $(brew --repository YOUR_USERNAME/tools)/Formula/tftpd.rb
```

Update these fields in the formula:
- `homepage`: Your repository URL
- `url`: The release tarball URL
- `sha256`: The SHA256 checksum from Step 2
- `head`: Your repository URL with branch

## Step 5: Test the Formula

Test the formula locally:

```bash
# Install from your local tap
brew install --build-from-source YOUR_USERNAME/tools/tftpd

# Test it works
tftpd -h

# Uninstall to test a clean install
brew uninstall tftpd
```

## Step 6: Commit and Push

```bash
cd $(brew --repository YOUR_USERNAME/tools)
git add Formula/tftpd.rb
git commit -m "tftpd 1.0.0 (new formula)"
git push
```

## Step 7: Install from Tap

Users can now install your formula:

```bash
brew tap YOUR_USERNAME/tools
brew install tftpd
```

Or in one command:

```bash
brew install YOUR_USERNAME/tools/tftpd
```

## Updating the Formula

When you release a new version:

1. Create a new GitHub release with a new tag
2. Update the `url` and `sha256` in the formula
3. Update the version number in the formula
4. Commit and push the changes

```bash
cd $(brew --repository YOUR_USERNAME/tools)
# Edit Formula/tftpd.rb with new version and SHA256
git add Formula/tftpd.rb
git commit -m "tftpd 1.1.0"
git push
```

## Formula File Location

The formula template is located at: `Formula/tftpd.rb`

Copy this file to your tap's Formula directory and update the placeholders.

