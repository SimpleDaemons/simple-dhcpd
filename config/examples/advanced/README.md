# Advanced Environment Configuration

This configuration is designed for medium to large office environments with multiple subnets, security requirements, and performance optimization.

## Overview

- **Networks**: 3 subnets (main office, guest, IoT)
- **IP Ranges**: Multiple ranges across different subnets
- **Lease Times**: Varying based on subnet purpose
- **Security**: MAC filtering, rate limiting, Option 82 support
- **Logging**: JSON format with multiple log files
- **Database**: SQLite with optimization and caching

## Features

### Network Configuration

#### Main Office Subnet (10.0.1.0/24)
- **Range**: 10.0.1.100 - 10.0.1.200
- **Lease Time**: 12 hours (43200 seconds)
- **Purpose**: Corporate workstations and servers
- **DNS**: Internal DNS servers + public fallback

#### Guest Network Subnet (10.0.2.0/24)
- **Range**: 10.0.2.100 - 10.0.2.200
- **Lease Time**: 1 hour (3600 seconds)
- **Purpose**: Guest WiFi access
- **DNS**: Public DNS servers only

#### IoT Devices Subnet (10.0.3.0/24)
- **Range**: 10.0.3.100 - 10.0.3.200
- **Lease Time**: 7 days (604800 seconds)
- **Purpose**: IoT devices and sensors
- **DNS**: Internal DNS servers

### Security Features

#### MAC Address Filtering
- **Mode**: Allow list
- **Purpose**: Only allow known devices
- **Management**: Centralized device management

#### Rate Limiting
- **Requests per minute**: 1000
- **Burst size**: 100
- **Per MAC/IP**: Enabled
- **Purpose**: Prevent DHCP flooding attacks

#### Option 82 Support
- **Validation**: Enabled
- **Logging**: Enabled
- **Required**: Optional
- **Purpose**: Relay agent information validation

### Performance Optimization

#### Database Configuration
- **Type**: SQLite
- **Backup**: Every hour
- **Retention**: 7 days
- **Compression**: Enabled
- **Optimization**: Enabled

#### Caching
- **Size**: 200MB
- **TTL**: 1 hour
- **Compression**: Enabled
- **Cache Types**: Leases, options, reservations

#### Connection Pooling
- **Max connections**: 1000
- **Idle timeout**: 5 minutes
- **Keep alive**: Enabled
- **Purpose**: Handle high request volumes

### Monitoring

#### Metrics
- **Format**: Prometheus
- **Interval**: 60 seconds
- **Custom metrics**: Enabled
- **Business metrics**: Enabled

#### Health Checks
- **Memory threshold**: 80%
- **CPU threshold**: 90%
- **Lease threshold**: 10,000
- **Interval**: 30 seconds

## Usage

### Installation
```bash
# Copy configuration
sudo cp config/examples/advanced/simple-dhcpd.conf /etc/simple-dhcpd/

# Create log directories
sudo mkdir -p /var/log/simple-dhcpd

# Set permissions
sudo chown -R dhcpd:dhcpd /var/log/simple-dhcpd

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

### Monitoring
```bash
# Show active leases
sudo simple-dhcpd --show-leases

# Show statistics
sudo simple-dhcpd --stats

# Check metrics endpoint
curl http://localhost:8080/metrics

# Check health endpoint
curl http://localhost:8080/health
```

## Customization

### Adding New Subnets
```json
{
  "name": "new-subnet",
  "network": "10.0.4.0/24",
  "range": "10.0.4.100-10.0.4.200",
  "gateway": "10.0.4.1",
  "dns_servers": ["10.0.1.10", "10.0.1.11"],
  "domain_name": "new.company.local",
  "lease_time": 86400
}
```

### Adding MAC Addresses
```json
"allow_list": [
  "00:11:22:33:44:55",
  "00:11:22:33:44:56",
  "AA:BB:CC:DD:EE:FF"  // Add new MAC
]
```

### Adjusting Performance Settings
```json
"caching": {
  "cache_size": "500MB",  // Increase cache size
  "ttl": 7200  // Increase TTL to 2 hours
}
```

## Security Considerations

### MAC Address Management
- Maintain allow list of authorized devices
- Regular audit of connected devices
- Remove unused MAC addresses

### Rate Limiting
- Monitor for rate limit violations
- Adjust limits based on usage patterns
- Consider per-subnet rate limiting

### Option 82 Validation
- Ensure relay agents are properly configured
- Monitor for validation failures
- Consider making Option 82 required

## Performance Tuning

### Database Optimization
- Regular database maintenance
- Monitor backup performance
- Consider migration to MySQL/PostgreSQL for large deployments

### Caching
- Monitor cache hit rates
- Adjust cache size based on usage
- Consider cache warming strategies

### Connection Pooling
- Monitor connection usage
- Adjust pool size based on load
- Consider connection timeouts

## Troubleshooting

### Common Issues
1. **MAC filtering too restrictive**: Check allow list
2. **Rate limiting too aggressive**: Adjust limits
3. **Performance issues**: Check database and caching
4. **Option 82 validation failures**: Check relay configuration

### Debug Commands
```bash
# Run with debug logging
sudo simple-dhcpd -f -v

# Check specific subnet
sudo simple-dhcpd --show-leases --subnet main-office

# Monitor network traffic
sudo tcpdump -i eth0 port 67
```

### Log Analysis
```bash
# Check security logs
grep -i "security" /var/log/simple-dhcpd/security.log

# Check rate limiting
grep -i "rate" /var/log/simple-dhcpd/access.log

# Check Option 82
grep -i "option.82" /var/log/simple-dhcpd/access.log
```

## Next Steps

For enterprise-level features, consider:
- [Production Configuration](../production/README.md)
- [High Availability Setup](../../../docs/deployment/ha.md)
- [Security Hardening](../../../docs/configuration/security.md)
