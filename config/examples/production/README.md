# Production Environment Configuration

This configuration is designed for large enterprise environments with high availability, security, and performance requirements.

## Overview

- **Networks**: 4 subnets (corporate LAN, guest WiFi, IoT devices, DMZ servers)
- **IP Ranges**: Multiple ranges across different subnets
- **Lease Times**: Varying based on subnet purpose and security requirements
- **Security**: Comprehensive security with MAC filtering, rate limiting, Option 82, authentication, and access control
- **Logging**: JSON format with multiple specialized log files and custom formats
- **Database**: MySQL with connection pooling and optimization
- **Monitoring**: Prometheus metrics, health checks, and alerting
- **High Availability**: Active-passive failover configuration

## Features

### Network Configuration

#### Corporate LAN Subnet (172.16.1.0/24)
- **Range**: 172.16.1.100 - 172.16.1.200
- **Lease Time**: 6 hours (21600 seconds)
- **Purpose**: Corporate workstations and servers
- **DNS**: Internal DNS servers with NTP support
- **Features**: PXE boot support, TFTP server integration

#### Guest WiFi Subnet (172.16.2.0/24)
- **Range**: 172.16.2.100 - 172.16.2.200
- **Lease Time**: 30 minutes (1800 seconds)
- **Purpose**: Guest WiFi access
- **DNS**: Public DNS servers only
- **Security**: Isolated from corporate network

#### IoT Devices Subnet (172.16.3.0/24)
- **Range**: 172.16.3.100 - 172.16.3.200
- **Lease Time**: 7 days (604800 seconds)
- **Purpose**: IoT devices and sensors
- **DNS**: Internal DNS servers
- **Security**: Restricted access with static reservations

#### DMZ Servers Subnet (172.16.4.0/24)
- **Range**: 172.16.4.100 - 172.16.4.200
- **Lease Time**: 24 hours (86400 seconds)
- **Purpose**: DMZ servers and services
- **DNS**: Internal DNS servers
- **Security**: Controlled access with reservations

### Security Features

#### MAC Address Filtering
- **Mode**: Allow list
- **Management**: Centralized device management
- **Audit**: Regular security audits

#### Rate Limiting
- **Requests per minute**: 2000
- **Burst size**: 200
- **Per MAC/IP**: Enabled
- **Purpose**: Prevent DHCP flooding attacks

#### Option 82 Support
- **Validation**: Enabled
- **Logging**: Enabled
- **Required**: True
- **Circuit ID**: Enabled
- **Remote ID**: Enabled
- **Vendor ID**: Enabled

#### Authentication
- **Method**: Option 82
- **Fallback**: Disabled
- **Purpose**: Ensure only authorized relay agents

#### Access Control Lists
- **Corporate LAN**: Allow
- **Guest WiFi**: Allow
- **IoT Devices**: Allow
- **DMZ Servers**: Allow
- **Default**: Deny

### Performance Optimization

#### Database Configuration
- **Type**: MySQL
- **Host**: 172.16.1.50
- **Port**: 3306
- **Database**: dhcp_leases
- **Backup**: Every 30 minutes
- **Retention**: 30 days
- **Connection Pool**: 10-100 connections

#### Caching
- **Size**: 1GB
- **TTL**: 30 minutes
- **Compression**: Enabled
- **Cache Types**: Leases, options, reservations, global options

#### Connection Pooling
- **Max connections**: 5000
- **Idle timeout**: 5 minutes
- **Keep alive**: Enabled
- **Max requests per connection**: 1000

#### Threading
- **Worker threads**: 16
- **IO threads**: 8
- **Queue size**: 10,000

### Monitoring and Alerting

#### Metrics
- **Format**: Prometheus
- **Interval**: 30 seconds
- **Exporters**: Prometheus, InfluxDB
- **Custom metrics**: Enabled
- **Business metrics**: Enabled

#### Health Checks
- **Memory threshold**: 85%
- **CPU threshold**: 90%
- **Lease threshold**: 50,000
- **Database threshold**: 1000ms
- **Disk space threshold**: 90%
- **Interval**: 15 seconds

#### Alerting
- **Email**: admin@company.com, ops@company.com
- **Slack**: #alerts channel
- **PagerDuty**: Integration for critical alerts

### High Availability

#### Failover Configuration
- **Mode**: Active-passive
- **Peer server**: 172.16.1.2
- **Timeout**: 30 seconds
- **Retry interval**: 10 seconds
- **Max retries**: 3

#### Load Balancing
- **Enabled**: False (active-passive mode)
- **Purpose**: Single active server with standby

## Usage

### Prerequisites

#### Database Setup
```sql
-- Create database
CREATE DATABASE dhcp_leases;

-- Create user
CREATE USER 'dhcp_user'@'%' IDENTIFIED BY 'secure_password';
GRANT ALL PRIVILEGES ON dhcp_leases.* TO 'dhcp_user'@'%';
FLUSH PRIVILEGES;
```

#### Log Directories
```bash
# Create log directories
sudo mkdir -p /var/log/simple-dhcpd
sudo chown -R dhcpd:dhcpd /var/log/simple-dhcpd
sudo chmod 755 /var/log/simple-dhcpd
```

### Installation
```bash
# Copy configuration
sudo cp config/examples/production/simple-dhcpd.conf /etc/simple-dhcpd/

# Set permissions
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd
sudo chmod 644 /etc/simple-dhcpd/simple-dhcpd.conf

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
  "network": "172.16.5.0/24",
  "range": "172.16.5.100-172.16.5.200",
  "gateway": "172.16.5.1",
  "dns_servers": ["172.16.1.10", "172.16.1.11"],
  "domain_name": "new.company.com",
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
  "cache_size": "2GB",  // Increase cache size
  "ttl": 1800  // Decrease TTL to 30 minutes
}
```

### Adding Alerting Endpoints
```json
"alerting": {
  "endpoints": [
    {
      "type": "webhook",
      "url": "https://hooks.slack.com/services/...",
      "channel": "#alerts"
    }
  ]
}
```

## Security Considerations

### MAC Address Management
- Maintain allow list of authorized devices
- Regular audit of connected devices
- Remove unused MAC addresses
- Consider device registration process

### Rate Limiting
- Monitor for rate limit violations
- Adjust limits based on usage patterns
- Consider per-subnet rate limiting
- Implement progressive penalties

### Option 82 Validation
- Ensure relay agents are properly configured
- Monitor for validation failures
- Consider making Option 82 required
- Implement relay agent authentication

### Access Control
- Regular review of access control rules
- Monitor for unauthorized access attempts
- Implement network segmentation
- Consider zero-trust networking

## Performance Tuning

### Database Optimization
- Regular database maintenance
- Monitor backup performance
- Consider read replicas for large deployments
- Implement connection pooling

### Caching
- Monitor cache hit rates
- Adjust cache size based on usage
- Consider cache warming strategies
- Implement cache invalidation

### Connection Pooling
- Monitor connection usage
- Adjust pool size based on load
- Consider connection timeouts
- Implement connection health checks

### Threading
- Monitor thread usage
- Adjust thread counts based on load
- Consider CPU affinity
- Implement thread monitoring

## High Availability

### Failover Testing
- Regular failover tests
- Monitor failover times
- Test network connectivity
- Verify data consistency

### Load Balancing
- Consider active-active mode
- Implement health checks
- Monitor load distribution
- Test failover scenarios

### Data Replication
- Ensure data consistency
- Monitor replication lag
- Test disaster recovery
- Implement backup strategies

## Troubleshooting

### Common Issues
1. **MAC filtering too restrictive**: Check allow list
2. **Rate limiting too aggressive**: Adjust limits
3. **Performance issues**: Check database and caching
4. **Option 82 validation failures**: Check relay configuration
5. **High availability issues**: Check failover configuration

### Debug Commands
```bash
# Run with debug logging
sudo simple-dhcpd -f -v

# Check specific subnet
sudo simple-dhcpd --show-leases --subnet corporate-lan

# Monitor network traffic
sudo tcpdump -i eth0 port 67

# Check database connectivity
mysql -h 172.16.1.50 -u dhcp_user -p dhcp_leases
```

### Log Analysis
```bash
# Check security logs
grep -i "security" /var/log/simple-dhcpd/security.log

# Check rate limiting
grep -i "rate" /var/log/simple-dhcpd/access.log

# Check Option 82
grep -i "option.82" /var/log/simple-dhcpd/access.log

# Check audit logs
grep -i "audit" /var/log/simple-dhcpd/audit.log
```

### Performance Monitoring
```bash
# Check metrics
curl http://localhost:8080/metrics | grep dhcp

# Check health
curl http://localhost:8080/health | jq

# Monitor database
mysqladmin -h 172.16.1.50 -u dhcp_user -p status
```

## Maintenance

### Regular Tasks
- **Daily**: Check logs for errors
- **Weekly**: Review security logs
- **Monthly**: Audit MAC address list
- **Quarterly**: Performance review

### Backup Procedures
- **Database**: Automated every 30 minutes
- **Configuration**: Version controlled
- **Logs**: Rotated and compressed
- **Disaster Recovery**: Tested quarterly

### Updates
- **Security**: Apply security patches immediately
- **Features**: Test in staging environment
- **Database**: Plan maintenance windows
- **Monitoring**: Update alerting rules

## Next Steps

For additional enterprise features, consider:
- [High Availability Setup](../../../docs/deployment/ha.md)
- [Security Hardening](../../../docs/configuration/security.md)
- [Performance Tuning](../../../docs/deployment/production.md)
- [Disaster Recovery](../../../docs/deployment/disaster-recovery.md)
