# Production Version Security Guide

**Version:** 1.0.0
**License:** Apache 2.0

---

## Overview

This guide covers security features and best practices for the Production Version of Simple DHCP Daemon.

## Security Features

### Option 82 (Relay Agent Information)

Option 82 support allows validation of relay agent information:

```json
{
  "dhcp": {
    "security": {
      "option82": {
        "enabled": true,
        "trusted_agents": ["192.168.1.1", "192.168.1.2"]
      }
    }
  }
}
```

### Rate Limiting

Protect against DHCP exhaustion attacks:

```json
{
  "dhcp": {
    "security": {
      "rate_limiting": {
        "enabled": true,
        "max_requests_per_second": 100
      }
    }
  }
}
```

### Security Logging

Enable security event logging:

```json
{
  "dhcp": {
    "logging": {
      "level": "INFO",
      "security_events": true
    }
  }
}
```

## Security Best Practices

1. **Use Option 82** - Enable Option 82 validation in relay environments
2. **Enable Rate Limiting** - Protect against DoS attacks
3. **Monitor Logs** - Regularly review security logs
4. **Restrict Access** - Use firewall rules to restrict access
5. **Regular Updates** - Keep software updated

See [Security Best Practices](../shared/user-guide/security-best-practices.md) for detailed guidelines.

---

**Last Updated:** December 2024
**Version:** 1.0.0
