# simple-dhcpd Deployment

This directory contains deployment configurations and examples for simple-dhcpd.

## Directory Structure

```
deployment/
├── systemd/                    # Linux systemd service files
│   └── simple-dhcpd.service
├── launchd/                    # macOS launchd service files
│   └── com.simple-dhcpd.simple-dhcpd.plist
├── logrotate.d/                # Linux log rotation configuration
│   └── simple-dhcpd
├── windows/                    # Windows service management
│   └── simple-dhcpd.service.bat
└── examples/                   # Deployment examples
    └── docker/                 # Docker deployment examples
        ├── docker-compose.yml
        └── README.md
```

## Platform-Specific Deployment

### Linux (systemd)

1. **Install the service file:**
   ```bash
   sudo cp deployment/systemd/simple-dhcpd.service /etc/systemd/system/
   sudo systemctl daemon-reload
   ```

2. **Create user and group:**
   ```bash
   sudo useradd --system --no-create-home --shell /bin/false simple-dhcpd
   ```

3. **Enable and start the service:**
   ```bash
   sudo systemctl enable simple-dhcpd
   sudo systemctl start simple-dhcpd
   ```

4. **Check status:**
   ```bash
   sudo systemctl status simple-dhcpd
   sudo journalctl -u simple-dhcpd -f
   ```

### macOS (launchd)

1. **Install the plist file:**
   ```bash
   sudo cp deployment/launchd/com.simple-dhcpd.simple-dhcpd.plist /Library/LaunchDaemons/
   sudo chown root:wheel /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist
   ```

2. **Load and start the service:**
   ```bash
   sudo launchctl load /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist
   sudo launchctl start com.simple-dhcpd.simple-dhcpd
   ```

3. **Check status:**
   ```bash
   sudo launchctl list | grep simple-dhcpd
   tail -f /var/log/simple-dhcpd.log
   ```

### Windows

1. **Run as Administrator:**
   ```cmd
   # Install service
   deployment\windows\simple-dhcpd.service.bat install
   
   # Start service
   deployment\windows\simple-dhcpd.service.bat start
   
   # Check status
   deployment\windows\simple-dhcpd.service.bat status
   ```

2. **Service management:**
   ```cmd
   # Stop service
   deployment\windows\simple-dhcpd.service.bat stop
   
   # Restart service
   deployment\windows\simple-dhcpd.service.bat restart
   
   # Uninstall service
   deployment\windows\simple-dhcpd.service.bat uninstall
   ```

## Log Rotation (Linux)

1. **Install logrotate configuration:**
   ```bash
   sudo cp deployment/logrotate.d/simple-dhcpd /etc/logrotate.d/
   ```

2. **Test logrotate configuration:**
   ```bash
   sudo logrotate -d /etc/logrotate.d/simple-dhcpd
   ```

3. **Force log rotation:**
   ```bash
   sudo logrotate -f /etc/logrotate.d/simple-dhcpd
   ```

## Docker Deployment

See [examples/docker/README.md](examples/docker/README.md) for detailed Docker deployment instructions.

### Quick Start

```bash
# Build and run with Docker Compose
cd deployment/examples/docker
docker-compose up -d

# Check status
docker-compose ps
docker-compose logs simple-dhcpd
```

## Configuration

### Service Configuration

Each platform has specific configuration requirements:

- **Linux**: Edit `/etc/systemd/system/simple-dhcpd.service`
- **macOS**: Edit `/Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist`
- **Windows**: Edit the service binary path in the batch file

### Application Configuration

Place your application configuration in:
- **Linux/macOS**: `/etc/simple-dhcpd/simple-dhcpd.conf`
- **Windows**: `%PROGRAMFILES%\simple-dhcpd\simple-dhcpd.conf`

## Security Considerations

### User and Permissions

1. **Create dedicated user:**
   ```bash
   # Linux
   sudo useradd --system --no-create-home --shell /bin/false simple-dhcpd
   
   # macOS
   sudo dscl . -create /Users/_simple-dhcpd UserShell /usr/bin/false
   sudo dscl . -create /Users/_simple-dhcpd UniqueID 200
   sudo dscl . -create /Users/_simple-dhcpd PrimaryGroupID 200
   sudo dscl . -create /Groups/_simple-dhcpd GroupID 200
   ```

2. **Set proper permissions:**
   ```bash
   # Configuration files
   sudo chown root:simple-dhcpd /etc/simple-dhcpd/simple-dhcpd.conf
   sudo chmod 640 /etc/simple-dhcpd/simple-dhcpd.conf
   
   # Log files
   sudo chown simple-dhcpd:simple-dhcpd /var/log/simple-dhcpd/
   sudo chmod 755 /var/log/simple-dhcpd/
   ```

### Firewall Configuration

Configure firewall rules as needed:

```bash
# Linux (ufw)
sudo ufw allow 67/tcp

# Linux (firewalld)
sudo firewall-cmd --permanent --add-port=67/tcp
sudo firewall-cmd --reload

# macOS
sudo pfctl -f /etc/pf.conf
```

## Monitoring

### Health Checks

1. **Service status:**
   ```bash
   # Linux
   sudo systemctl is-active simple-dhcpd
   
   # macOS
   sudo launchctl list | grep simple-dhcpd
   
   # Windows
   sc query simple-dhcpd
   ```

2. **Port availability:**
   ```bash
   netstat -tlnp | grep 67
   ss -tlnp | grep 67
   ```

3. **Process monitoring:**
   ```bash
   ps aux | grep simple-dhcpd
   top -p $(pgrep simple-dhcpd)
   ```

### Log Monitoring

1. **Real-time logs:**
   ```bash
   # Linux
   sudo journalctl -u simple-dhcpd -f
   
   # macOS
   tail -f /var/log/simple-dhcpd.log
   
   # Windows
   # Use Event Viewer or PowerShell Get-WinEvent
   ```

2. **Log analysis:**
   ```bash
   # Search for errors
   sudo journalctl -u simple-dhcpd --since "1 hour ago" | grep -i error
   
   # Count log entries
   sudo journalctl -u simple-dhcpd --since "1 day ago" | wc -l
   ```

## Troubleshooting

### Common Issues

1. **Service won't start:**
   - Check configuration file syntax
   - Verify user permissions
   - Check port availability
   - Review service logs

2. **Permission denied:**
   - Ensure service user exists
   - Check file permissions
   - Verify directory ownership

3. **Port already in use:**
   - Check what's using the port: `netstat -tlnp | grep 67`
   - Stop conflicting service or change port

4. **Service stops unexpectedly:**
   - Check application logs
   - Verify resource limits
   - Review system logs

### Debug Mode

Run the service in debug mode for troubleshooting:

```bash
# Linux/macOS
sudo -u simple-dhcpd /usr/local/bin/simple-dhcpd --debug

# Windows
simple-dhcpd.exe --debug
```

### Log Levels

Adjust log level for more verbose output:

```bash
# Set log level in configuration
log_level = debug

# Or via environment variable
export SIMPLE-DHCPD_LOG_LEVEL=debug
```

## Backup and Recovery

### Configuration Backup

```bash
# Backup configuration
sudo tar -czf simple-dhcpd-config-backup-$(date +%Y%m%d).tar.gz /etc/simple-dhcpd/

# Backup logs
sudo tar -czf simple-dhcpd-logs-backup-$(date +%Y%m%d).tar.gz /var/log/simple-dhcpd/
```

### Service Recovery

```bash
# Stop service
sudo systemctl stop simple-dhcpd

# Restore configuration
sudo tar -xzf simple-dhcpd-config-backup-YYYYMMDD.tar.gz -C /

# Start service
sudo systemctl start simple-dhcpd
```

## Updates

### Service Update Process

1. **Stop service:**
   ```bash
   sudo systemctl stop simple-dhcpd
   ```

2. **Backup current version:**
   ```bash
   sudo cp /usr/local/bin/simple-dhcpd /usr/local/bin/simple-dhcpd.backup
   ```

3. **Install new version:**
   ```bash
   sudo cp simple-dhcpd /usr/local/bin/
   sudo chmod +x /usr/local/bin/simple-dhcpd
   ```

4. **Start service:**
   ```bash
   sudo systemctl start simple-dhcpd
   ```

5. **Verify update:**
   ```bash
   sudo systemctl status simple-dhcpd
   simple-dhcpd --version
   ```
