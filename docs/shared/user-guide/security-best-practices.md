# Security Best Practices

This guide covers security best practices for Simple DHCP Daemon.

## Overview

Security is critical for DHCP servers. This guide provides recommendations for securing your Simple DHCP Daemon installation.

## General Security Principles

### Defense in Depth

- Use multiple layers of security
- Don't rely on a single security measure
- Implement security at multiple levels

### Least Privilege

- Run service with minimal required privileges
- Restrict file and directory permissions
- Limit network access

### Regular Updates

- Keep software updated
- Monitor security advisories
- Apply security patches promptly

## Installation Security

### Service Account

```bash
# Create dedicated service account
sudo useradd -r -s /bin/false -d /var/lib/simple-dhcpd dhcpd

# Set ownership
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd
sudo chown -R dhcpd:dhcpd /var/lib/simple-dhcpd
sudo chown -R dhcpd:dhcpd /var/log/simple-dhcpd
```

### File Permissions

```bash
# Configuration files
sudo chmod 640 /etc/simple-dhcpd/simple-dhcpd.conf
sudo chmod 750 /etc/simple-dhcpd

# Data files
sudo chmod 640 /var/lib/simple-dhcpd/leases.db
sudo chmod 750 /var/lib/simple-dhcpd

# Log files
sudo chmod 640 /var/log/simple-dhcpd/*.log
sudo chmod 750 /var/log/simple-dhcpd
```

## Network Security

### Firewall Configuration

```bash
# Allow only DHCP traffic
sudo ufw allow 67/udp
sudo ufw allow 68/udp

# Block other ports
sudo ufw deny 67/tcp
sudo ufw deny 68/tcp
```

### Network Isolation

- Run DHCP server on isolated network segment
- Use VLANs to separate traffic
- Implement network access controls

### Interface Binding

```json
{
  "dhcp": {
    "listen": ["192.168.1.1:67"]
  }
}
```

**Recommendations:**
- Bind to specific interfaces
- Avoid binding to 0.0.0.0 in production
- Use interface-specific IP addresses

## Configuration Security

### MAC Address Filtering

```json
{
  "dhcp": {
    "security": {
      "mac_filtering": {
        "enabled": true,
        "mode": "allow",
        "allow_list": [
          "00:11:22:33:44:55",
          "00:11:22:33:44:56"
        ]
      }
    }
  }
}
```

**Recommendations:**
- Enable MAC filtering for production
- Use allow list for strict control
- Regularly review and update lists

### IP Address Filtering

```json
{
  "dhcp": {
    "security": {
      "ip_filtering": {
        "enabled": true,
        "allowed_ranges": [
          "192.168.1.0/24"
        ],
        "denied_ranges": [
          "10.0.0.0/8"
        ]
      }
    }
  }
}
```

### Rate Limiting

```json
{
  "dhcp": {
    "security": {
      "rate_limiting": {
        "enabled": true,
        "requests_per_minute": 1000,
        "burst_size": 100,
        "per_mac": true,
        "per_ip": true,
        "block_duration": 300
      }
    }
  }
}
```

**Recommendations:**
- Enable rate limiting to prevent DoS attacks
- Set appropriate limits based on network size
- Monitor for rate limit violations

## Option 82 Security

### Relay Agent Validation

```json
{
  "dhcp": {
    "security": {
      "option_82": {
        "enabled": true,
        "validation": true,
        "required": false,
        "trusted_agents": [
          "192.168.1.1",
          "192.168.1.2"
        ]
      }
    }
  }
}
```

**Recommendations:**
- Enable Option 82 validation
- Configure trusted relay agents
- Log Option 82 violations

## Authentication

### Client Authentication

```json
{
  "dhcp": {
    "security": {
      "authentication": {
        "enabled": true,
        "method": "hmac-sha256",
        "key_rotation": true,
        "key_rotation_interval": 86400
      }
    }
  }
}
```

**Recommendations:**
- Enable authentication for sensitive networks
- Use strong authentication methods
- Rotate keys regularly

## Logging and Monitoring

### Security Event Logging

```json
{
  "dhcp": {
    "logging": {
      "enable": true,
      "level": "info",
      "security_log": "/var/log/simple-dhcpd/security.log",
      "log_security_events": true,
      "log_authentication": true,
      "log_access_control": true
    }
  }
}
```

**Recommendations:**
- Enable security event logging
- Use separate log files for security events
- Monitor security logs regularly

### Audit Logging

```json
{
  "dhcp": {
    "logging": {
      "audit_log": "/var/log/simple-dhcpd/audit.log",
      "log_config_changes": true,
      "log_lease_changes": true,
      "log_admin_actions": true
    }
  }
}
```

## Access Control

### Configuration Access

- Restrict configuration file access
- Use version control for configurations
- Implement change management process

### Administrative Access

- Limit administrative access
- Use strong authentication
- Implement role-based access control

## Data Protection

### Lease Database Security

```json
{
  "dhcp": {
    "performance": {
      "lease_database": {
        "encryption": true,
        "backup_encryption": true,
        "access_control": true
      }
    }
  }
}
```

### Backup Security

- Encrypt backups
- Store backups securely
- Test backup restoration

## Incident Response

### Security Incident Procedures

1. **Detection**: Monitor for security events
2. **Containment**: Isolate affected systems
3. **Investigation**: Analyze security events
4. **Remediation**: Fix security issues
5. **Recovery**: Restore normal operations
6. **Documentation**: Document incidents and responses

### Security Monitoring

```bash
# Monitor security logs
tail -f /var/log/simple-dhcpd/security.log | grep -i "denied\|violation\|attack"

# Check for suspicious activity
grep -i "unauthorized" /var/log/simple-dhcpd/security.log

# Review authentication failures
grep -i "authentication failed" /var/log/simple-dhcpd/security.log
```

## Compliance

### Security Standards

- Follow industry security standards
- Implement compliance requirements
- Regular security audits

### Security Documentation

- Document security configuration
- Maintain security procedures
- Regular security reviews

## Additional Resources

- [Configuration Reference](../configuration/README.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
- [Security Configuration](../config/SECURITY_CONFIG.md)
