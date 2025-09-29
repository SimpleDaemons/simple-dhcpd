# Simple Environment Configuration

This configuration is designed for small home networks or basic office environments with minimal requirements.

## Overview

- **Network**: Single subnet (192.168.1.0/24)
- **IP Range**: 192.168.1.100 - 192.168.1.200
- **Lease Time**: 24 hours (86400 seconds)
- **Security**: Basic (no MAC filtering or rate limiting)
- **Logging**: Simple text format
- **Database**: File-based SQLite

## Features

### Network Configuration
- Single home network subnet
- 100 IP addresses available for dynamic allocation
- Static reservations for router and NAS
- IP exclusions for infrastructure devices

### DHCP Options
- Subnet mask (255.255.255.0)
- Default gateway (192.168.1.1)
- DNS servers (8.8.8.8, 8.8.4.4)
- Domain name (home.local)
- Broadcast address
- NTP servers (pool.ntp.org)

### Static Reservations
- **Router**: 192.168.1.10 (00:11:22:33:44:55)
- **NAS**: 192.168.1.20 (00:11:22:33:44:56)

### IP Exclusions
- **Infrastructure**: 192.168.1.1 - 192.168.1.10
- **Reserved**: 192.168.1.250 - 192.168.1.254

## Usage

### Installation
```bash
# Copy configuration
sudo cp config/examples/simple/simple-dhcpd.conf /etc/simple-dhcpd/

# Start service
sudo systemctl start simple-dhcpd
sudo systemctl enable simple-dhcpd
```

### Testing
```bash
# Test configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check status
sudo systemctl status simple-dhcpd

# View logs
sudo journalctl -u simple-dhcpd -f
```

## Customization

### Adding More Reservations
```json
{
  "mac_address": "AA:BB:CC:DD:EE:FF",
  "ip_address": "192.168.1.30",
  "hostname": "printer",
  "description": "Network Printer"
}
```

### Changing Lease Time
```json
"lease_time": 43200,  // 12 hours
"max_lease_time": 86400  // 24 hours
```

### Adding Custom Options
```json
{
  "name": "custom-option-100",
  "value": "custom-value"
}
```

## Monitoring

### Basic Monitoring
```bash
# Show active leases
sudo simple-dhcpd --show-leases

# Show statistics
sudo simple-dhcpd --stats

# Check log file
tail -f /var/log/simple-dhcpd.log
```

### Log Rotation
The configuration includes automatic log rotation:
- Maximum file size: 50MB
- Maximum files: 5
- Compression enabled

## Troubleshooting

### Common Issues
1. **Permission denied**: Check file permissions
2. **Port in use**: Stop conflicting DHCP services
3. **No IPs assigned**: Check network connectivity

### Debug Commands
```bash
# Run in foreground with verbose logging
sudo simple-dhcpd -f -v

# Test network connectivity
ping 192.168.1.1

# Check port binding
sudo netstat -tulpn | grep :67
```

## Next Steps

For more advanced features, consider:
- [Advanced Configuration](../advanced/README.md)
- [Production Configuration](../production/README.md)
- [Security Configuration](../../../docs/configuration/security.md)
