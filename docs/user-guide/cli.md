# Command Line Interface

This guide covers all command line options and usage patterns for Simple DHCP Daemon.

## Basic Usage

```bash
simple-dhcpd [OPTIONS]
```

## Command Line Options

### Configuration Options

| Option | Long Option | Description | Default |
|--------|-------------|-------------|---------|
| `-c` | `--config` | Configuration file path | `/etc/simple-dhcpd/simple-dhcpd.conf` |
| `-t` | `--test-config` | Test configuration and exit | - |
| `-v` | `--validate` | Validate configuration syntax | - |

### Daemon Options

| Option | Long Option | Description | Default |
|--------|-------------|-------------|---------|
| `-d` | `--daemon` | Run as daemon | - |
| `-p` | `--pid-file` | PID file path | `/var/run/simple-dhcpd.pid` |
| `-f` | `--foreground` | Run in foreground | - |

### Logging Options

| Option | Long Option | Description | Default |
|--------|-------------|-------------|---------|
| `-l` | `--log-file` | Log file path | `/var/log/simple-dhcpd.log` |
| `-v` | `--verbose` | Verbose logging | - |
| `-q` | `--quiet` | Quiet mode | - |

### Information Options

| Option | Long Option | Description | Default |
|--------|-------------|-------------|---------|
| `-h` | `--help` | Show help message | - |
| `-V` | `--version` | Show version information | - |
| `-s` | `--show-config` | Show current configuration | - |
| `-L` | `--show-leases` | Show active leases | - |
| `-S` | `--stats` | Show statistics | - |

## Usage Examples

### Basic Server Start

```bash
# Start with default configuration
sudo simple-dhcpd

# Start with custom configuration
sudo simple-dhcpd -c /etc/simple-dhcpd/custom.conf

# Start in foreground with verbose logging
sudo simple-dhcpd -f -v
```

### Daemon Mode

```bash
# Start as daemon
sudo simple-dhcpd -d

# Start as daemon with custom PID file
sudo simple-dhcpd -d -p /var/run/custom-dhcpd.pid

# Start as daemon with custom log file
sudo simple-dhcpd -d -l /var/log/custom-dhcpd.log
```

### Configuration Testing

```bash
# Test configuration syntax
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --validate

# Show parsed configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --show-config
```

### Information and Monitoring

```bash
# Show version information
simple-dhcpd --version

# Show help
simple-dhcpd --help

# Show active leases
sudo simple-dhcpd --show-leases

# Show statistics
sudo simple-dhcpd --stats
```

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `SIMPLE_DHCPD_CONFIG` | Configuration file path | `/etc/simple-dhcpd/simple-dhcpd.conf` |
| `SIMPLE_DHCPD_LOG_LEVEL` | Log level | `info` |
| `SIMPLE_DHCPD_LOG_FILE` | Log file path | `/var/log/simple-dhcpd.log` |
| `SIMPLE_DHCPD_PID_FILE` | PID file path | `/var/run/simple-dhcpd.pid` |

### Using Environment Variables

```bash
# Set configuration via environment
export SIMPLE_DHCPD_CONFIG=/etc/simple-dhcpd/production.conf
export SIMPLE_DHCPD_LOG_LEVEL=debug
sudo simple-dhcpd

# Or use inline
SIMPLE_DHCPD_LOG_LEVEL=debug sudo simple-dhcpd -f
```

## Signal Handling

Simple DHCP Daemon responds to the following signals:

| Signal | Action | Description |
|--------|--------|-------------|
| `SIGTERM` | Graceful shutdown | Stop accepting new requests, finish current requests |
| `SIGINT` | Graceful shutdown | Same as SIGTERM |
| `SIGHUP` | Reload configuration | Reload configuration without restart |
| `SIGUSR1` | Rotate logs | Rotate log files |
| `SIGUSR2` | Show statistics | Display current statistics |

### Signal Examples

```bash
# Graceful shutdown
sudo kill -TERM $(pidof simple-dhcpd)

# Reload configuration
sudo kill -HUP $(pidof simple-dhcpd)

# Rotate logs
sudo kill -USR1 $(pidof simple-dhcpd)

# Show statistics
sudo kill -USR2 $(pidof simple-dhcpd)
```

## Systemd Integration

### Service Commands

```bash
# Start service
sudo systemctl start simple-dhcpd

# Stop service
sudo systemctl stop simple-dhcpd

# Restart service
sudo systemctl restart simple-dhcpd

# Reload configuration
sudo systemctl reload simple-dhcpd

# Enable auto-start
sudo systemctl enable simple-dhcpd

# Disable auto-start
sudo systemctl disable simple-dhcpd

# Check status
sudo systemctl status simple-dhcpd
```

### Service Configuration

The systemd service file is typically located at `/lib/systemd/system/simple-dhcpd.service`:

```ini
[Unit]
Description=Simple DHCP Daemon
After=network.target

[Service]
Type=notify
ExecStart=/usr/bin/simple-dhcpd -d
ExecReload=/bin/kill -HUP $MAINPID
PIDFile=/var/run/simple-dhcpd.pid
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

## Docker Usage

### Basic Docker Run

```bash
# Run with default configuration
docker run -d --name simple-dhcpd --net host simpledaemons/simple-dhcpd:latest

# Run with custom configuration
docker run -d --name simple-dhcpd --net host \
  -v /etc/simple-dhcpd:/etc/simple-dhcpd \
  simpledaemons/simple-dhcpd:latest

# Run with environment variables
docker run -d --name simple-dhcpd --net host \
  -e SIMPLE_DHCPD_LOG_LEVEL=debug \
  -e SIMPLE_DHCPD_CONFIG=/etc/simple-dhcpd/debug.conf \
  simpledaemons/simple-dhcpd:latest
```

### Docker Compose

```yaml
version: '3.8'
services:
  simple-dhcpd:
    image: simpledaemons/simple-dhcpd:latest
    network_mode: host
    volumes:
      - ./config:/etc/simple-dhcpd
      - ./logs:/var/log/simple-dhcpd
    environment:
      - SIMPLE_DHCPD_LOG_LEVEL=info
    restart: unless-stopped
```

## Troubleshooting

### Common Issues

#### Permission Denied

```bash
# Check file permissions
ls -la /etc/simple-dhcpd/
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd/

# Check if running as root
sudo simple-dhcpd -f
```

#### Port Already in Use

```bash
# Check what's using port 67
sudo netstat -tulpn | grep :67
sudo lsof -i :67

# Stop conflicting service
sudo systemctl stop isc-dhcp-server
```

#### Configuration Errors

```bash
# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check JSON syntax
python -m json.tool /etc/simple-dhcpd/simple-dhcpd.conf
```

### Debug Mode

```bash
# Run with debug logging
sudo simple-dhcpd -f -v

# Run with debug configuration
sudo simple-dhcpd -c /etc/simple-dhcpd/debug.conf -f -v

# Check system logs
sudo journalctl -u simple-dhcpd -f
```

### Performance Monitoring

```bash
# Show statistics
sudo simple-dhcpd --stats

# Show active leases
sudo simple-dhcpd --show-leases

# Monitor in real-time
watch -n 1 'sudo simple-dhcpd --stats'
```

## Advanced Usage

### Custom Configuration Directory

```bash
# Use custom configuration directory
sudo simple-dhcpd -c /opt/simple-dhcpd/config/production.conf
```

### Multiple Instances

```bash
# Run multiple instances on different ports
sudo simple-dhcpd -c /etc/simple-dhcpd/instance1.conf -p /var/run/simple-dhcpd-1.pid
sudo simple-dhcpd -c /etc/simple-dhcpd/instance2.conf -p /var/run/simple-dhcpd-2.pid
```

### Log Rotation

```bash
# Rotate logs manually
sudo kill -USR1 $(pidof simple-dhcpd)

# Configure logrotate
sudo vim /etc/logrotate.d/simple-dhcpd
```

## Best Practices

1. **Always test configuration**: Use `--test-config` before starting
2. **Use daemon mode in production**: Use `-d` flag for production deployments
3. **Set appropriate log levels**: Use `info` for production, `debug` for troubleshooting
4. **Use PID files**: Specify PID file for process management
5. **Monitor performance**: Use `--stats` to monitor server performance
6. **Backup configurations**: Keep configuration backups
7. **Use systemd**: Use systemd for service management in production

## Next Steps

- [Service Management](service.md)
- [Monitoring](monitoring.md)
- [Configuration Guide](../configuration/README.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
