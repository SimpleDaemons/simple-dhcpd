# Enterprise Version Integrations Guide

**Version:** Foundation Complete (v1.0.0 Alpha in development)  
**License:** BSL 1.1

---

## Overview

The Enterprise Version supports integrations with DNS, LDAP/Active Directory, SNMP, and cloud services.

## DNS Integration

### Dynamic DNS Updates

Automatically update DNS records when leases are assigned:

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
          "ttl": 3600,
          "reverse_zones": ["1.168.192.in-addr.arpa"]
        }
      }
    }
  }
}
```

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enabled` | boolean | `false` | Enable DNS integration |
| `servers` | array | `[]` | DNS server IP addresses |
| `zone` | string | `""` | Primary DNS zone |
| `update_mode` | string | `"secure"` | Update mode (secure, insecure) |
| `tsig_key` | string | `""` | TSIG key name |
| `ttl` | integer | `3600` | DNS record TTL |

### TSIG Key Setup

```bash
# Generate TSIG key
dnssec-keygen -a HMAC-SHA256 -b 256 -n HOST dhcp-update-key

# Add to DNS server configuration
key "dhcp-update-key" {
    algorithm hmac-sha256;
    secret "base64-encoded-key";
};
```

## LDAP/Active Directory Integration

### LDAP Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "ldap": {
          "enabled": true,
          "server": "ldap://ldap.example.com",
          "port": 389,
          "ssl": false,
          "base_dn": "dc=example,dc=com",
          "bind_dn": "cn=dhcp,ou=services,dc=example,dc=com",
          "bind_password": "password",
          "user_search": "(&(objectClass=user)(sAMAccountName={username}))",
          "group_search": "(&(objectClass=group)(member={dn}))",
          "attribute_mapping": {
            "mac": "macAddress",
            "hostname": "cn"
          }
        }
      }
    }
  }
}
```

### Active Directory Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "ldap": {
          "enabled": true,
          "server": "ldap://ad.example.com",
          "port": 389,
          "ssl": false,
          "base_dn": "dc=example,dc=com",
          "bind_dn": "CN=dhcp,CN=Users,DC=example,DC=com",
          "bind_password": "password",
          "user_search": "(&(objectClass=user)(sAMAccountName={username}))",
          "group_search": "(&(objectClass=group)(member={dn}))"
        }
      }
    }
  }
}
```

### LDAP Authentication

Use LDAP for client authentication:

```json
{
  "dhcp": {
    "enterprise": {
      "security": {
        "authentication": {
          "enabled": true,
          "method": "ldap",
          "ldap": {
            "server": "ldap://ldap.example.com",
            "base_dn": "dc=example,dc=com"
          }
        }
      }
    }
  }
}
```

## SNMP Integration

### SNMP v2 Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "snmp": {
          "enabled": true,
          "port": 161,
          "community": "public",
          "v2": {
            "enabled": true
          }
        }
      }
    }
  }
}
```

### SNMP v3 Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "snmp": {
          "enabled": true,
          "port": 161,
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

### SNMP MIB

The Enterprise Version provides a custom MIB for monitoring:
- Lease statistics
- Request rates
- Subnet utilization
- Security events
- Performance metrics

## Cloud Integrations

### AWS Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "aws": {
          "enabled": true,
          "region": "us-east-1",
          "vpc_id": "vpc-12345678",
          "route53": {
            "enabled": true,
            "hosted_zone": "example.com"
          },
          "cloudwatch": {
            "enabled": true,
            "namespace": "SimpleDHCPD"
          }
        }
      }
    }
  }
}
```

### Azure Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "azure": {
          "enabled": true,
          "subscription_id": "subscription-id",
          "resource_group": "dhcp-rg",
          "dns": {
            "enabled": true,
            "zone": "example.com"
          },
          "monitor": {
            "enabled": true,
            "workspace": "workspace-id"
          }
        }
      }
    }
  }
}
```

### GCP Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "gcp": {
          "enabled": true,
          "project_id": "project-id",
          "network": "default",
          "dns": {
            "enabled": true,
            "zone": "example-com"
          },
          "monitoring": {
            "enabled": true
          }
        }
      }
    }
  }
}
```

## SIEM Integration

### Syslog Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "syslog": {
          "enabled": true,
          "servers": ["192.168.1.100:514"],
          "format": "CEF",
          "facility": "local0",
          "severity": "INFO"
        }
      }
    }
  }
}
```

### Splunk Integration

```json
{
  "dhcp": {
    "enterprise": {
      "integrations": {
        "splunk": {
          "enabled": true,
          "server": "https://splunk.example.com:8088",
          "token": "splunk-token",
          "index": "dhcp"
        }
      }
    }
  }
}
```

## Testing Integrations

### Test DNS Integration

```bash
# Test DNS update
simple-dhcpd-enterprise --test-dns

# Check DNS records
dig @192.168.1.10 server1.example.com
```

### Test LDAP Integration

```bash
# Test LDAP connection
simple-dhcpd-enterprise --test-ldap

# Test LDAP authentication
simple-dhcpd-enterprise --test-ldap-auth username
```

### Test SNMP Integration

```bash
# Test SNMP
snmpwalk -v2c -c public localhost 1.3.6.1.4.1.12345.1
```

## Best Practices

1. **Security:** Use secure protocols (TSIG for DNS, SNMP v3)
2. **Monitoring:** Monitor integration health
3. **Backup:** Backup integration configurations
4. **Testing:** Test integrations before production use
5. **Documentation:** Document integration configurations

## Troubleshooting

### DNS Issues

```bash
# Check DNS server connectivity
dig @192.168.1.10 example.com

# Check TSIG key
dnssec-keygen -l -K /etc/bind/keys dhcp-update-key
```

### LDAP Issues

```bash
# Test LDAP connection
ldapsearch -H ldap://ldap.example.com -x -D "cn=dhcp,ou=services,dc=example,dc=com" -w password -b "dc=example,dc=com"
```

### SNMP Issues

```bash
# Test SNMP
snmpget -v2c -c public localhost 1.3.6.1.2.1.1.1.0
```

## Related Documentation

- [Configuration Guide](configuration.md) - Integration configuration
- [Security Guide](security.md) - Integration security
- [Operations Guide](operations.md) - Integration operations

---

**Last Updated:** December 2024  
**Version:** Foundation Complete
