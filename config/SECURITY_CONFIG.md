# Security Configuration Reference

This document provides a comprehensive reference for all security features available in Simple DHCP Daemon, including configuration examples and best practices.

## Table of Contents

1. [DHCP Snooping](#dhcp-snooping)
2. [MAC Address Filtering](#mac-address-filtering)
3. [IP Address Filtering](#ip-address-filtering)
4. [Rate Limiting](#rate-limiting)
5. [Option 82 Support](#option-82-support)
6. [Client Authentication](#client-authentication)
7. [Security Event Logging](#security-event-logging)
8. [Best Practices](#best-practices)

## DHCP Snooping

DHCP snooping validates DHCP messages against trusted bindings to prevent rogue DHCP servers and unauthorized DHCP traffic.

### Configuration

```json
"dhcp_snooping": {
  "enabled": true,
  "trusted_interfaces": ["eth0", "eth1"],
  "logging": true,
  "validation": true,
  "bindings": [
    {
      "mac_address": "00:11:22:33:44:55",
      "ip_address": "10.0.1.10",
      "interface": "eth0",
      "trusted": true
    }
  ]
}
```

### Parameters

- **enabled**: Enable/disable DHCP snooping
- **trusted_interfaces**: List of trusted network interfaces
- **logging**: Enable logging of snooping events
- **validation**: Enable strict validation of DHCP messages
- **bindings**: Pre-configured trusted MAC-to-IP bindings

## MAC Address Filtering

MAC address filtering controls which devices can obtain DHCP leases based on their MAC addresses.

### Configuration

```json
"mac_filtering": {
  "enabled": true,
  "mode": "allow",
  "rules": [
    {
      "mac_address": "00:11:22:33:44:55",
      "allow": true,
      "description": "Trusted server",
      "enabled": true
    },
    {
      "mac_address": "00:11:22:33:44:*",
      "allow": true,
      "description": "Corporate devices",
      "enabled": true
    }
  ]
}
```

### Parameters

- **enabled**: Enable/disable MAC filtering
- **mode**: "allow" or "deny" (default behavior when no rules match)
- **rules**: Array of MAC filtering rules
  - **mac_address**: MAC address or pattern (supports wildcards)
  - **allow**: true to allow, false to deny
  - **description**: Human-readable description
  - **enabled**: Enable/disable this specific rule

### Wildcard Patterns

- `*`: Matches any characters
- `00:11:22:33:44:*`: Matches any device with OUI 00:11:22:33:44
- `aa:bb:*`: Matches any device with OUI aa:bb

## IP Address Filtering

IP address filtering controls which IP addresses can be assigned to clients.

### Configuration

```json
"ip_filtering": {
  "enabled": true,
  "mode": "deny",
  "rules": [
    {
      "ip_address": "10.0.1.10",
      "allow": true,
      "description": "Trusted IP",
      "enabled": true
    }
  ]
}
```

### Parameters

- **enabled**: Enable/disable IP filtering
- **mode**: "allow" or "deny" (default behavior when no rules match)
- **rules**: Array of IP filtering rules
  - **ip_address**: IP address to filter
  - **allow**: true to allow, false to deny
  - **description**: Human-readable description
  - **enabled**: Enable/disable this specific rule

## Rate Limiting

Rate limiting prevents DHCP exhaustion attacks by limiting the number of requests per time window.

### Configuration

```json
"rate_limiting": {
  "enabled": true,
  "rules": [
    {
      "identifier": "*",
      "identifier_type": "mac",
      "max_requests": 100,
      "time_window": 60,
      "block_duration": 300,
      "enabled": true
    },
    {
      "identifier": "00:11:22:33:44:55",
      "identifier_type": "mac",
      "max_requests": 200,
      "time_window": 60,
      "block_duration": 300,
      "enabled": true
    }
  ]
}
```

### Parameters

- **enabled**: Enable/disable rate limiting
- **rules**: Array of rate limiting rules
  - **identifier**: MAC address, IP address, or "*" for all
  - **identifier_type**: "mac", "ip", or "interface"
  - **max_requests**: Maximum requests allowed in time window
  - **time_window**: Time window in seconds
  - **block_duration**: Block duration in seconds after limit exceeded
  - **enabled**: Enable/disable this specific rule

## Option 82 Support

Option 82 (Relay Agent Information) provides additional security by validating relay agent information.

### Configuration

```json
"option_82": {
  "enabled": true,
  "validation": true,
  "rules": [
    {
      "interface": "eth1",
      "required": true,
      "enabled": true
    }
  ],
  "trusted_relay_agents": [
    {
      "circuit_id": "circuit-001",
      "remote_id": "relay-001",
      "enabled": true
    }
  ]
}
```

### Parameters

- **enabled**: Enable/disable Option 82 support
- **validation**: Enable validation of Option 82 data
- **rules**: Interface-specific Option 82 rules
  - **interface**: Network interface name
  - **required**: Whether Option 82 is required for this interface
  - **enabled**: Enable/disable this specific rule
- **trusted_relay_agents**: Pre-configured trusted relay agent combinations

## Client Authentication

Client authentication provides additional security by requiring clients to authenticate before receiving leases.

### Configuration

```json
"authentication": {
  "enabled": true,
  "key": "your-secret-key-here",
  "client_credentials": [
    {
      "client_id": "00:11:22:33:44:55",
      "password_hash": "hashed-password",
      "salt": "random-salt",
      "enabled": true
    }
  ]
}
```

### Parameters

- **enabled**: Enable/disable client authentication
- **key**: Shared secret key for HMAC generation
- **client_credentials**: Array of client authentication credentials
  - **client_id**: Client identifier (usually MAC address)
  - **password_hash**: HMAC-SHA256 hash of the password
  - **salt**: Random salt used in hash generation
  - **enabled**: Enable/disable this specific credential

## Security Event Logging

Security event logging provides comprehensive audit trails and monitoring capabilities.

### Configuration

```json
"security_events": {
  "enable_logging": true,
  "log_file": "/var/log/simple-dhcpd/security.log",
  "event_callback": null,
  "retention_days": 30,
  "alert_thresholds": {
    "rate_limit_exceeded": 10,
    "mac_filter_blocked": 5,
    "ip_filter_blocked": 5,
    "option_82_invalid": 3,
    "authentication_failed": 5
  }
}
```

### Parameters

- **enable_logging**: Enable/disable security event logging
- **log_file**: Path to security log file
- **event_callback**: Optional callback function for real-time events
- **retention_days**: Number of days to retain security logs
- **alert_thresholds**: Thresholds for security alerts

## Best Practices

### 1. Layered Security
- Enable multiple security features for defense in depth
- Use MAC filtering for device-level control
- Implement rate limiting to prevent attacks
- Enable Option 82 validation for relay environments

### 2. Monitoring and Alerting
- Enable comprehensive security event logging
- Set appropriate alert thresholds
- Monitor security logs regularly
- Implement automated alerting for critical events

### 3. Access Control
- Use allow-lists instead of deny-lists when possible
- Implement strict MAC filtering for sensitive networks
- Use client authentication for high-security environments
- Regularly review and update filtering rules

### 4. Performance Considerations
- Balance security features with performance requirements
- Use appropriate rate limiting thresholds
- Monitor system performance with security features enabled
- Consider hardware acceleration for high-throughput environments

### 5. Compliance
- Enable audit logging for compliance requirements
- Implement data retention policies
- Use encrypted storage for sensitive data
- Maintain detailed access control documentation

## Security Event Types

The following security events are logged and can be monitored:

- **MAC_FILTER_ALLOWED**: MAC address allowed by filtering rules
- **MAC_FILTER_BLOCKED**: MAC address blocked by filtering rules
- **IP_FILTER_ALLOWED**: IP address allowed by filtering rules
- **IP_FILTER_BLOCKED**: IP address blocked by filtering rules
- **RATE_LIMIT_ALLOWED**: Request allowed by rate limiting
- **RATE_LIMIT_EXCEEDED**: Request blocked by rate limiting
- **OPTION_82_VALID**: Option 82 validation passed
- **OPTION_82_INVALID**: Option 82 validation failed
- **OPTION_82_MISSING**: Option 82 required but missing
- **AUTH_SUCCESS**: Client authentication successful
- **AUTH_FAILED**: Client authentication failed
- **SNOOPING_VIOLATION**: DHCP snooping violation detected

## Troubleshooting

### Common Issues

1. **Legitimate clients blocked**: Check MAC/IP filtering rules
2. **Rate limiting too aggressive**: Adjust rate limiting thresholds
3. **Option 82 validation failures**: Verify relay agent configuration
4. **Authentication failures**: Check client credentials and keys

### Debug Commands

```bash
# Check security statistics
simple-dhcpd --security-stats

# View security events
tail -f /var/log/simple-dhcpd/security.log

# Test MAC filtering
simple-dhcpd --test-mac-filter 00:11:22:33:44:55

# Test rate limiting
simple-dhcpd --test-rate-limit 00:11:22:33:44:55
```
