# Configuration Guide

This guide covers all configuration options available in Simple DHCP Daemon.

## Configuration File Format

Simple DHCP Daemon uses JSON format for configuration files. The configuration file is typically located at `/etc/simple-dhcpd/simple-dhcpd.conf`.

## Basic Configuration Structure

```json
{
  "dhcp": {
    "listen": ["0.0.0.0:67"],
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0/24",
        "range": "192.168.1.100-192.168.1.200",
        "gateway": "192.168.1.1",
        "dns_servers": ["8.8.8.8", "8.8.4.4"],
        "domain_name": "local",
        "lease_time": 86400
      }
    ],
    "logging": {
      "enable": true,
      "level": "info"
    }
  }
}
```

## Configuration Sections

### Global Settings

#### `listen`
Array of addresses and ports to listen on.

```json
"listen": [
  "0.0.0.0:67",      // IPv4 on all interfaces
  "[::]:67",         // IPv6 on all interfaces
  "192.168.1.1:67"   // Specific interface
]
```

#### `subnets`
Array of subnet configurations.

#### `global_options`
Global DHCP options applied to all subnets.

```json
"global_options": [
  {
    "name": "domain-name-servers",
    "value": "8.8.8.8,8.8.4.4"
  },
  {
    "name": "ntp-servers",
    "value": "pool.ntp.org"
  }
]
```

### Subnet Configuration

#### Basic Subnet Settings

```json
{
  "name": "main-subnet",                    // Subnet identifier
  "network": "192.168.1.0/24",             // Network address and prefix
  "range": "192.168.1.100-192.168.1.200",  // IP address range
  "gateway": "192.168.1.1",                // Default gateway
  "dns_servers": ["8.8.8.8", "8.8.4.4"],   // DNS servers
  "domain_name": "local",                  // Domain name
  "lease_time": 86400,                     // Lease time in seconds
  "max_lease_time": 172800                 // Maximum lease time
}
```

#### Subnet Options

```json
"options": [
  {
    "name": "subnet-mask",
    "value": "255.255.255.0"
  },
  {
    "name": "routers",
    "value": "192.168.1.1"
  },
  {
    "name": "domain-name-servers",
    "value": "8.8.8.8,8.8.4.4"
  },
  {
    "name": "domain-name",
    "value": "local"
  }
]
```

#### Static Leases (Reservations)

```json
"reservations": [
  {
    "mac_address": "00:11:22:33:44:55",
    "ip_address": "192.168.1.10",
    "hostname": "server1",
    "description": "Web Server",
    "options": [
      {
        "name": "host-name",
        "value": "server1"
      }
    ]
  }
]
```

#### IP Exclusions

```json
"exclusions": [
  {
    "start": "192.168.1.1",
    "end": "192.168.1.10"
  },
  {
    "start": "192.168.1.250",
    "end": "192.168.1.254"
  }
]
```

### Security Configuration

#### MAC Address Filtering

```json
"security": {
  "mac_filtering": {
    "enabled": true,
    "mode": "allow",  // "allow" or "deny"
    "allow_list": [
      "00:11:22:33:44:55",
      "00:11:22:33:44:56"
    ],
    "deny_list": [
      "00:11:22:33:44:99"
    ]
  }
}
```

#### Rate Limiting

```json
"security": {
  "rate_limiting": {
    "enabled": true,
    "requests_per_minute": 1000,
    "burst_size": 100,
    "per_mac": true,
    "per_ip": true
  }
}
```

#### Option 82 Support

```json
"security": {
  "option_82": {
    "enabled": true,
    "validation": true,
    "logging": true,
    "required": false,
    "circuit_id": true,
    "remote_id": true,
    "vendor_id": true
  }
}
```

### Performance Configuration

#### Lease Database

```json
"performance": {
  "lease_database": {
    "type": "file",  // "file", "sqlite", "mysql", "postgresql"
    "path": "/var/lib/simple-dhcpd/leases.db",
    "backup": true,
    "backup_interval": 3600,
    "backup_retention": 7,
    "compression": true,
    "optimization": true
  }
}
```

#### Connection Pooling

```json
"performance": {
  "connection_pooling": {
    "enabled": true,
    "max_connections": 1000,
    "idle_timeout": 300,
    "connection_timeout": 30,
    "keep_alive": true,
    "keep_alive_timeout": 65
  }
}
```

#### Caching

```json
"performance": {
  "caching": {
    "enabled": true,
    "cache_size": "200MB",
    "ttl": 3600,
    "compression": true,
    "optimization": true,
    "cache_leases": true,
    "cache_options": true,
    "cache_reservations": true
  }
}
```

### Logging Configuration

#### Basic Logging

```json
"logging": {
  "enable": true,
  "level": "info",  // "debug", "info", "warn", "error", "fatal"
  "log_file": "/var/log/simple-dhcpd.log",
  "format": "json",  // "json" or "text"
  "rotation": true,
  "max_size": "100MB",
  "max_files": 10,
  "compression": true
}
```

#### Multiple Log Files

```json
"logging": {
  "enable": true,
  "level": "info",
  "access_log": "/var/log/simple-dhcpd/access.log",
  "error_log": "/var/log/simple-dhcpd/error.log",
  "lease_log": "/var/log/simple-dhcpd/leases.log",
  "security_log": "/var/log/simple-dhcpd/security.log",
  "audit_log": "/var/log/simple-dhcpd/audit.log"
}
```

#### Custom Log Formats

```json
"logging": {
  "log_format": {
    "access": "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %D",
    "error": "%t [%l] %P: %m",
    "lease": "%t %h %u %{REQUEST_METHOD} %{REQUEST_URI} %s %b",
    "security": "%t %h %u \"%r\" %s %b \"%{Referer}i\" \"%{User-Agent}i\"",
    "audit": "%t %h %u %{REQUEST_METHOD} %{REQUEST_URI} %s %b"
  }
}
```

### Monitoring Configuration

#### Metrics

```json
"monitoring": {
  "metrics": {
    "enabled": true,
    "endpoint": "/metrics",
    "format": "prometheus",
    "interval": 60,
    "custom_metrics": true,
    "business_metrics": true
  }
}
```

#### Health Checks

```json
"monitoring": {
  "health_checks": {
    "enabled": true,
    "endpoint": "/health",
    "interval": 30,
    "timeout": 5,
    "checks": [
      {
        "name": "memory",
        "threshold": 80,
        "action": "warn"
      },
      {
        "name": "cpu",
        "threshold": 90,
        "action": "warn"
      },
      {
        "name": "leases",
        "threshold": 10000,
        "action": "warn"
      }
    ]
  }
}
```

## DHCP Options Reference

### Standard Options

| Code | Name | Description | Format |
|------|------|-------------|---------|
| 1 | subnet-mask | Subnet mask | IP address |
| 3 | routers | Default gateway | IP address(es) |
| 6 | domain-name-servers | DNS servers | IP address(es) |
| 15 | domain-name | Domain name | String |
| 51 | ip-address-lease-time | Lease time | 32-bit integer |
| 53 | dhcp-message-type | Message type | 8-bit integer |
| 54 | server-identifier | Server IP | IP address |
| 58 | renewal-time | Renewal time | 32-bit integer |
| 59 | rebinding-time | Rebinding time | 32-bit integer |

### Custom Options

```json
"options": [
  {
    "name": "custom-option-100",
    "value": "custom-value"
  },
  {
    "name": "vendor-encapsulated-options",
    "value": "01:04:00:00:00:00"
  }
]
```

## Configuration Validation

### Command Line Validation

```bash
# Validate configuration file
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check configuration syntax
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --validate
```

### Common Validation Errors

1. **Invalid JSON syntax**: Check for missing commas, brackets, or quotes
2. **Invalid IP addresses**: Ensure IP addresses are in correct format
3. **Invalid MAC addresses**: Use format `XX:XX:XX:XX:XX:XX`
4. **Invalid ranges**: Start IP must be less than end IP
5. **Missing required fields**: Ensure all required fields are present

## Configuration Examples

### Home Network

```json
{
  "dhcp": {
    "listen": ["0.0.0.0:67"],
    "subnets": [
      {
        "name": "home-network",
        "network": "192.168.1.0/24",
        "range": "192.168.1.100-192.168.1.200",
        "gateway": "192.168.1.1",
        "dns_servers": ["8.8.8.8", "8.8.4.4"],
        "domain_name": "home.local",
        "lease_time": 86400,
        "reservations": [
          {
            "mac_address": "00:11:22:33:44:55",
            "ip_address": "192.168.1.10",
            "hostname": "router"
          }
        ]
      }
    ],
    "logging": {
      "enable": true,
      "level": "info"
    }
  }
}
```

### Enterprise Network

```json
{
  "dhcp": {
    "listen": ["0.0.0.0:67", "[::]:67"],
    "subnets": [
      {
        "name": "main-office",
        "network": "10.0.1.0/24",
        "range": "10.0.1.100-10.0.1.200",
        "gateway": "10.0.1.1",
        "dns_servers": ["10.0.1.10", "10.0.1.11"],
        "domain_name": "company.local",
        "lease_time": 43200,
        "reservations": [
          {
            "mac_address": "00:11:22:33:44:55",
            "ip_address": "10.0.1.10",
            "hostname": "dc1"
          }
        ]
      }
    ],
    "security": {
      "mac_filtering": {
        "enabled": true,
        "mode": "allow"
      },
      "rate_limiting": {
        "enabled": true,
        "requests_per_minute": 1000
      }
    },
    "logging": {
      "enable": true,
      "level": "info",
      "format": "json"
    }
  }
}
```

## Hot Reloading

Simple DHCP Daemon supports configuration hot reloading:

```bash
# Reload configuration without restart
sudo systemctl reload simple-dhcpd

# Or send SIGHUP signal
sudo kill -HUP $(pidof simple-dhcpd)
```

## Best Practices

1. **Use descriptive subnet names**: Make subnet names meaningful
2. **Set appropriate lease times**: Balance between performance and flexibility
3. **Use static leases for servers**: Reserve IP addresses for critical devices
4. **Enable logging**: Configure appropriate log levels for your environment
5. **Use exclusions**: Exclude IP addresses used by infrastructure
6. **Test configurations**: Always validate configurations before deployment
7. **Backup configurations**: Keep configuration backups
8. **Monitor performance**: Use metrics and health checks

## Troubleshooting Configuration

### Common Issues

1. **Configuration not loading**: Check file permissions and JSON syntax
2. **Subnet not working**: Verify network configuration and routing
3. **Clients not getting IPs**: Check logs and network connectivity
4. **Performance issues**: Review lease database and caching settings

### Debug Commands

```bash
# Check configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Verbose logging
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf -v

# Show current configuration
simple-dhcpd --show-config

# Show active leases
simple-dhcpd --show-leases
```

## Next Steps

- [Configuration Examples](examples.md)
- [Security Configuration](security.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
