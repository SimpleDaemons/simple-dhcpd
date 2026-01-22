# Production Version Configuration Guide

**Version:** 1.0.0
**License:** Apache 2.0

---

## Overview

This guide covers configuration for the Production Version of Simple DHCP Daemon. The Production Version supports JSON, YAML, and INI configuration formats.

## Configuration File Location

Default configuration file locations:
- **Linux:** `/etc/simple-dhcpd/simple-dhcpd.conf`
- **macOS:** `/usr/local/etc/simple-dhcpd/simple-dhcpd.conf`
- **Windows:** `C:\Program Files\Simple DHCP Daemon\config\simple-dhcpd.conf`

## Basic Configuration

### Minimal Configuration

```json
{
  "dhcp": {
    "listen_addresses": ["0.0.0.0"],
    "port": 67,
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0",
        "prefix_length": 24,
        "range_start": "192.168.1.100",
        "range_end": "192.168.1.200",
        "lease_time": 3600
      }
    ]
  }
}
```

### Complete Configuration Example

```json
{
  "dhcp": {
    "listen_addresses": ["0.0.0.0"],
    "port": 67,
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0",
        "prefix_length": 24,
        "range_start": "192.168.1.100",
        "range_end": "192.168.1.200",
        "lease_time": 3600,
        "max_lease_time": 7200,
        "static_leases": [
          {
            "mac": "00:11:22:33:44:55",
            "ip": "192.168.1.50",
            "hostname": "server1"
          }
        ]
      }
    ],
    "options": {
      "global": {
        "3": "192.168.1.1",
        "6": ["192.168.1.10", "192.168.1.11"],
        "15": "example.com"
      }
    },
    "security": {
      "enable_security": true,
      "option82": {
        "enabled": true,
        "trusted_agents": ["192.168.1.1"]
      },
      "rate_limiting": {
        "enabled": true,
        "max_requests_per_second": 100
      }
    },
    "logging": {
      "level": "INFO",
      "format": "STANDARD",
      "file": "/var/log/simple-dhcpd/simple-dhcpd.log"
    }
  }
}
```

## Configuration Reference

### DHCP Server Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `listen_addresses` | array | `["0.0.0.0"]` | IP addresses to listen on |
| `port` | integer | `67` | UDP port to listen on |
| `subnets` | array | `[]` | Subnet configurations |

### Subnet Configuration

| Option | Type | Required | Description |
|--------|------|----------|-------------|
| `name` | string | Yes | Subnet name/identifier |
| `network` | string | Yes | Network address (CIDR notation) |
| `prefix_length` | integer | Yes | Subnet prefix length |
| `range_start` | string | Yes | Start of IP range |
| `range_end` | string | Yes | End of IP range |
| `lease_time` | integer | Yes | Default lease time (seconds) |
| `max_lease_time` | integer | No | Maximum lease time (seconds) |
| `static_leases` | array | No | Static IP reservations |

### Security Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_security` | boolean | `false` | Enable security features |
| `option82.enabled` | boolean | `false` | Enable Option 82 support |
| `option82.trusted_agents` | array | `[]` | Trusted relay agent IPs |
| `rate_limiting.enabled` | boolean | `false` | Enable rate limiting |
| `rate_limiting.max_requests_per_second` | integer | `100` | Max requests per second |

### Logging Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `level` | string | `INFO` | Log level (DEBUG, INFO, WARN, ERROR) |
| `format` | string | `STANDARD` | Log format (STANDARD, JSON, EXTENDED) |
| `file` | string | `""` | Log file path (empty = stdout) |

## Advanced Configuration

### Multiple Subnets

```json
{
  "dhcp": {
    "subnets": [
      {
        "name": "subnet-1",
        "network": "192.168.1.0",
        "prefix_length": 24,
        "range_start": "192.168.1.100",
        "range_end": "192.168.1.200",
        "lease_time": 3600
      },
      {
        "name": "subnet-2",
        "network": "192.168.2.0",
        "prefix_length": 24,
        "range_start": "192.168.2.100",
        "range_end": "192.168.2.200",
        "lease_time": 7200
      }
    ]
  }
}
```

### Static Leases

```json
{
  "dhcp": {
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0",
        "prefix_length": 24,
        "range_start": "192.168.1.100",
        "range_end": "192.168.1.200",
        "static_leases": [
          {
            "mac": "00:11:22:33:44:55",
            "ip": "192.168.1.50",
            "hostname": "server1",
            "lease_time": 86400
          },
          {
            "mac": "AA:BB:CC:DD:EE:FF",
            "ip": "192.168.1.51",
            "hostname": "server2"
          }
        ]
      }
    ]
  }
}
```

### DHCP Options

```json
{
  "dhcp": {
    "options": {
      "global": {
        "3": "192.168.1.1",
        "6": ["192.168.1.10", "192.168.1.11"],
        "15": "example.com",
        "51": 3600
      },
      "subnet": {
        "main-subnet": {
          "3": "192.168.1.1",
          "6": ["192.168.1.10"]
        }
      }
    }
  }
}
```

## Configuration Validation

### Validate Configuration

```bash
# Test configuration file
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check for errors
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --validate
```

### Common Configuration Errors

1. **Invalid IP Addresses**
   - Ensure all IP addresses are valid
   - Check CIDR notation is correct
   - Verify range_start < range_end

2. **Overlapping Ranges**
   - Ensure subnet ranges don't overlap
   - Check static leases are within range

3. **Invalid Options**
   - Verify DHCP option codes are valid (1-255)
   - Check option values match expected format

## Hot Reloading

The Production Version supports hot reloading via SIGHUP:

```bash
# Reload configuration
sudo killall -HUP simple-dhcpd

# Or using systemd
sudo systemctl reload simple-dhcpd
```

## Configuration Examples

See [Configuration Examples](../shared/configuration/examples.md) for more examples.

## Related Documentation

- [Installation Guide](../shared/getting-started/installation.md)
- [Deployment Guide](deployment.md)
- [Security Guide](security.md)
- [Performance Tuning](performance.md)

---

**Last Updated:** December 2024
**Version:** 1.0.0
