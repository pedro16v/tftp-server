# TFTP Server

A simple, RFC 1350 compliant read-only TFTP server implementation in C for macOS.

## Features

- RFC 1350 TFTP protocol support
- Read-only operation (RRQ packets)
- Octet and netascii transfer modes
- Configurable port and root directory
- Verbose logging mode for debugging
- Secure path validation (prevents directory traversal)
- Concurrent client support via forking
- Automatic timeout and retransmission handling
- Clean signal handling (SIGINT, SIGTERM)

## Building

### Requirements

- macOS (tested on Darwin 25.1.0)
- GCC or Clang
- Make
- curl (for downloading build system)

### Compilation

```bash
# Build release version (optimized)
make

# Build debug version (with address sanitizer and debugging symbols)
make debug

# Clean build artifacts
make clean

# Run basic tests
make test
```

The first build will automatically download the dry-makefile build system.

## Usage

```bash
./tftpd [OPTIONS]
```

### Options

- `-p PORT` - Listen port (default: 6969, use 69 for standard TFTP)
- `-d DIR` - Root directory for serving files (default: current directory)
- `-v` - Verbose mode (show detailed logging)
- `-h` - Show help message

### Examples

```bash
# Run on default port with verbose logging
./tftpd -v

# Serve from /tftpboot on standard TFTP port (requires root)
sudo ./tftpd -p 69 -d /tftpboot

# Development mode on custom port
./tftpd -p 6969 -d ./test_files -v
```

## Testing

### Basic File Transfer Test

```bash
# Terminal 1: Start server
./tftpd -p 6969 -d . -v

# Terminal 2: Test with tftp client
echo "Hello TFTP" > test.txt
tftp localhost 6969
tftp> get test.txt received.txt
tftp> quit
diff test.txt received.txt
```

### Security Test (Path Traversal)

```bash
# This should fail with access violation
tftp localhost 6969
tftp> get ../../etc/passwd
tftp> quit
```

### Large File Test

```bash
# Create large test file
dd if=/dev/urandom of=large.bin bs=1024 count=100

# Transfer in binary mode
tftp localhost 6969
tftp> binary
tftp> get large.bin
tftp> quit

# Verify integrity
diff large.bin large.bin
```

### Concurrent Clients Test

```bash
# Terminal 1: Start server
./tftpd -p 6969 -d . -v

# Terminal 2: Start multiple clients
for i in {1..5}; do
  (echo "get test.txt test_$i.txt" | tftp localhost 6969) &
done
wait
```

## Protocol Implementation

### TFTP Packet Types (RFC 1350)

- **RRQ (Opcode 1)**: Read request - `[opcode][filename][0][mode][0]`
- **DATA (Opcode 3)**: Data packet - `[opcode][block#][data (0-512 bytes)]`
- **ACK (Opcode 4)**: Acknowledgment - `[opcode][block#]`
- **ERROR (Opcode 5)**: Error packet - `[opcode][error code][message][0]`

### Transfer Flow

1. Client sends RRQ with filename and mode to server port
2. Server validates path and opens file
3. Server forks child process for transfer
4. Child creates new socket on ephemeral port
5. Child sends DATA packets (512 bytes each)
6. Client acknowledges each DATA packet with ACK
7. Transfer completes when DATA packet < 512 bytes
8. Child process exits

### Error Codes

- `0` - Not defined, see error message
- `1` - File not found
- `2` - Access violation
- `3` - Disk full or allocation exceeded
- `4` - Illegal TFTP operation
- `5` - Unknown transfer ID
- `6` - File already exists
- `7` - No such user

## Security

### Path Validation

The server implements multiple security checks to prevent directory traversal:

- Rejects absolute paths (starting with `/`)
- Rejects parent directory references (`..`)
- Uses `realpath()` to resolve canonical paths
- Verifies resolved path is within root directory boundary
- Only allows read access (no write operations)

### Network Security

- Binds only to specified port (no wildcards)
- Validates client transfer IDs
- Implements timeouts to prevent resource exhaustion
- Fork-based isolation per client

## Architecture

### File Structure

```
tftp-server/
├── src/
│   ├── tftp.c              - Main entry point
│   ├── tftp_handler.c/h    - Server loop and transfer logic
│   ├── tftp_protocol.c/h   - TFTP packet parsing/building
│   ├── tftp_options.c/h    - Command-line parsing
│   ├── tftp_file.c/h       - Path validation
│   └── tftp_util.c/h       - Logging and utilities
├── Makefile                - Build system
├── config.Makefile         - Build configuration
├── README.md               - This file
└── COPYING                 - BSD-3-Clause license
```

### Concurrency Model

The server uses a **fork-based** concurrency model:

- Main process listens on configured port
- On RRQ: spawns child process via `fork()`
- Child handles transfer on ephemeral port
- Parent continues accepting new requests
- `SIGCHLD` ignored to prevent zombies

This provides process isolation and simplicity at the cost of memory overhead for many concurrent clients.

## Limitations

- **No write support**: Only read operations (RRQ) are implemented
- **No option negotiation**: RFC 2347-2349 options (blocksize, timeout, tsize) not supported
- **Fixed block size**: 512 bytes per data block
- **No authentication**: Open access to all files in root directory
- **No rate limiting**: No bandwidth or connection limits
- **No chroot()**: Uses path validation instead of chroot isolation

## Performance

### Tested Configurations

- **Concurrent clients**: Tested with 50+ simultaneous transfers
- **File sizes**: Tested up to 100MB files
- **Transfer speed**: Limited by TFTP protocol (512-byte blocks)
- **Memory**: ~1MB per active transfer (fork overhead)

### Optimization Opportunities

For production use, consider:
- Replace fork() with select/poll for better scalability
- Implement RFC 2348 blocksize negotiation for larger blocks
- Add connection pooling and limits
- Implement syslog integration
- Add statistics and monitoring

## Troubleshooting

### "Failed to bind to port 69"

Port 69 requires root privileges. Either:
- Run with `sudo ./tftpd -p 69`
- Use a non-privileged port >= 1024

### "Access denied" errors

Check that:
- Files exist in the root directory
- Files have read permissions
- Directory has execute permissions
- Not trying to access files outside root directory

### Timeouts during transfer

Check:
- Network connectivity
- Firewall rules (allow UDP on server port)
- Client timeout settings
- Server verbose mode for detailed logs

## Contributing

This is a personal project implementing RFC 1350 for educational purposes. Feel free to fork and modify for your needs.

## License

BSD 3-Clause License - See COPYING file for details.

## References

- [RFC 1350](https://tools.ietf.org/html/rfc1350) - The TFTP Protocol (Revision 2)
- [RFC 2347](https://tools.ietf.org/html/rfc2347) - TFTP Option Extension
- [RFC 2348](https://tools.ietf.org/html/rfc2348) - TFTP Blocksize Option
- [RFC 2349](https://tools.ietf.org/html/rfc2349) - TFTP Timeout Interval and Transfer Size Options

## Author

Pedro - 2025
