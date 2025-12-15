# typed: false
# frozen_string_literal: true

# This file was generated automatically. Do not edit it directly.
# Instead, copy this file to your Homebrew tap repository:
#   ~/homebrew-tap/Formula/tftpd.rb
# and update the url and homepage fields with your actual repository URL.

class Tftpd < Formula
  desc "Simple, RFC 1350 compliant read-only TFTP server implementation in C for macOS"
  homepage "https://github.com/pedro16v/tftp-server"
  url "https://github.com/pedro16v/tftp-server/archive/refs/tags/v1.0.1.tar.gz"
  sha256 "b4466308761ffe42119ac7a978634573f86384ca707992847091c0117f295c0c"
  license "BSD-3-Clause"
  head "https://github.com/pedro16v/tftp-server.git", branch: "main"

  depends_on "make" => :build
  depends_on "curl" => :build

  def install
    system "make", "release"
    raise "Build failed: src/tftp not found" unless File.exist?("src/tftp")
    system "make", "install", "PREFIX=#{prefix}"
  end

  test do
    # Test that the binary exists and shows help
    output = shell_output("#{bin}/tftpd -h 2>&1", 1)
    assert_match "Usage: tftpd", output
  end
end

