# Enterprise Version Configuration Guide

**Version:** Foundation Complete (v1.0.0 Alpha in development)
**License:** BSL 1.1

---

## Overview

This guide covers Enterprise-specific configuration options. All Production Version configuration options are also available.

## Enterprise Configuration Structure

```json
{
  "dhcp": {
    "enterprise": {
      "high_availability": {},
      "management": {},
      "integrations": {},
      "security": {},
      "performance": {}
    }
  }
}
```

## High Availability Configuration

### Basic HA Setup

```json
{
  "dhcp": {
    "enterprise": {
      "high_availability": {
        "enabled": true,
        "mode": "active-passive",
        "peer": "192.168.1.2",
        "failover_port": 647,
        "mclt": 3600,
        "max_response_delay": 60
      }
    }
  }
}
```

### Active-Active HA Setup

```json
{
  "dhcp": {
    "enterprise": {
      "high_availability": {
        "enabled": true,
        "mode": "active-active",
        "peer": "192.168.1.2",
        "failover_port": 647,
        "load_balance": true,
        "split": 50
      }
    }
  }
}
```

### HA Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enabled` | boolean | `false` | Enable high availability |
| `mode` | string | `"active-passive"` | HA mode (active-passive, active-active) |
| `peer` | string | `""` | Peer server IP address |
| `failover_port` | integer | `647` | Failover communication port |
| `mclt` | integer | `3600` | Maximum Client Lead Time (seconds) |
| `max_response_delay` | integer | `60` | Maximum response delay (seconds) |
| `load_balance` | boolean | `false` | Enable load balancing (active-active) |
| `split` | integer | `50` | Load split percentage (active-active) |

## Management Interface Configuration

### Web UI Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "management": {
        "web_ui": {
          "enabled": true,
          "port": 8080,
          "ssl": true,
          "ssl_cert": "/etc/ssl/certs/simple-dhcpd.crt",
          "ssl_key": "/etc/ssl/private/simple-dhcpd.key",
          "session_timeout": 3600,
          "max_sessions": 100
        }
      }
    }
  }
}
```

### REST API Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "management": {
        "rest_api": {
          "enabled": true,
          "port": 8081,
          "ssl": true,
          "authentication": true,
          "api_key_auth": true,
          "rate_limit": 1000,
          "cors": {
            "enabled": true,
            "allowed_origins": ["https://example.com"]
          }
        }
      }
    }
  }
}
```

### Management Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `web_ui.enabled` | boolean | `false` | Enable web UI |
| `web_ui.port` | integer | `8080` | Web UI port |
| `web_ui.ssl` | boolean | `true` | Enable SSL/TLS |
| `rest_api.enabled` | boolean | `false` | Enable REST API |
| `rest_api.port` | integer | `8081` | REST API port |
| `rest_api.authentication` | boolean | `true` | Require authentication |
| `rest_api.rate_limit` | integer | `1000` | API rate limit (requests/hour) |

## Integration Configuration

### DNS Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "dns": {
          "enabled": true,
          "servers": ["192.168.1.10", "192.168.1.11"],
          "zone": "example.com",
          "update_mode": "secure",
          "tsig_key": "dhcp-update-key",
          "ttl": 3600
        }
      }
    }
  }
}
```

### LDAP/AD Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "ldap": {
          "enabled": true,
          "server": "ldap://ldap.example.com",
          "port": 389,
          "base_dn": "dc=example,dc=com",
          "bind_dn": "cn=dhcp,ou=services,dc=example,dc=com",
          "bind_password": "password",
          "user_search": "(&(objectClass=user)(sAMAccountName={username}))",
          "group_search": "(&(objectClass=group)(member={dn}))"
        }
      }
    }
  }
}
```

### SNMP Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "snmp": {
          "enabled": true,
          "port": 161,
          "community": "public",
          "v3": {
            "enabled": true,
            "username": "snmp-user",
            "auth_protocol": "SHA",
            "auth_password": "auth-pass",
            "priv_protocol": "AES",
            "priv_password": "priv-pass"
          }
        }
      }
    }
  }
}
```

## Advanced Security Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "security": {
        "advanced_snooping": {
          "enabled": true,
          "strict_validation": true,
          "binding_verification": true
        },
        "authentication": {
          "enabled": true,
          "method": "hmac-sha256",
          "key_rotation": 86400
        },
        "access_control": {
          "enabled": true,
          "policies": [
            {
              "name": "restricted",
              "mac_patterns": ["00:11:22:*:*:*"],
              "action": "deny"
            }
          ]
        }
      }
    }
  }
}
```

## Performance Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "performance": {
        "thread_pool": {
          "size": 8,
          "queue_size": 1000
        },
        "lease_database": {
          "connection_pool": 10,
          "query_cache": true,
          "cache_size": 10000
        },
        "optimization": {
          "batch_processing": true,
          "batch_size": 100,
          "async_operations": true
        }
      }
    }
  }
}
```

## Configuration Validation

```bash
# Validate Enterprise configuration
simple-dhcpd-enterprise -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check Enterprise features
simple-dhcpd-enterprise --check-features
```

## Configuration Examples

See [Configuration Examples](../shared/configuration/examples.md) for more examples.

## Related Documentation

- [High Availability](high-availability.md) - HA configuration details
- [Management Interface](management.md) - Management setup
- [Integrations](integrations.md) - Integration configuration
- [Security](security.md) - Security configuration

---

**Last Updated:** December 2024
**Version:** Foundation Complete
