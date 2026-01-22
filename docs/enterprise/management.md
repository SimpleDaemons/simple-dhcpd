# Enterprise Version Management Interface Guide

**Version:** Foundation Complete (v1.0.0 Alpha in development)  
**License:** BSL 1.1

---

## Overview

The Enterprise Version includes a web-based management interface and REST API for managing and monitoring the DHCP server.

## Web Management Interface

### Accessing the Web UI

```bash
# Default URL
https://your-server:8080

# Default credentials (change on first login)
Username: admin
Password: admin
```

### Features

- **Dashboard:** Real-time statistics and monitoring
- **Lease Management:** View and manage DHCP leases
- **Subnet Configuration:** Configure subnets and IP ranges
- **Security Settings:** Configure security features
- **High Availability:** Monitor and manage HA setup
- **Logs:** View and search logs
- **Reports:** Generate reports and analytics

### Configuration

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
          "max_sessions": 100,
          "theme": "dark"
        }
      }
    }
  }
}
```

## REST API

### API Endpoints

#### Status

```bash
# Get server status
GET /api/v1/status

curl -k https://your-server:8081/api/v1/status \
  -u admin:password
```

#### Leases

```bash
# List all leases
GET /api/v1/leases

# Get specific lease
GET /api/v1/leases/{mac}

# Create static lease
POST /api/v1/leases
{
  "mac": "00:11:22:33:44:55",
  "ip": "192.168.1.50",
  "hostname": "server1"
}

# Delete lease
DELETE /api/v1/leases/{mac}
```

#### Subnets

```bash
# List subnets
GET /api/v1/subnets

# Get subnet details
GET /api/v1/subnets/{name}

# Create subnet
POST /api/v1/subnets
{
  "name": "subnet-1",
  "network": "192.168.1.0",
  "prefix_length": 24,
  "range_start": "192.168.1.100",
  "range_end": "192.168.1.200"
}

# Update subnet
PUT /api/v1/subnets/{name}

# Delete subnet
DELETE /api/v1/subnets/{name}
```

#### Statistics

```bash
# Get statistics
GET /api/v1/statistics

# Get lease statistics
GET /api/v1/statistics/leases

# Get performance metrics
GET /api/v1/statistics/performance
```

#### High Availability

```bash
# Get HA status
GET /api/v1/ha/status

# Initiate failover
POST /api/v1/ha/failover

# Get HA health
GET /api/v1/ha/health
```

### Authentication

#### Basic Authentication

```bash
curl -k https://your-server:8081/api/v1/status \
  -u username:password
```

#### API Key Authentication

```bash
curl -k https://your-server:8081/api/v1/status \
  -H "X-API-Key: your-api-key"
```

### API Configuration

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

## API Examples

### Python Example

```python
import requests
import json

# API base URL
base_url = "https://your-server:8081/api/v1"
auth = ("admin", "password")

# Get status
response = requests.get(f"{base_url}/status", auth=auth, verify=False)
print(response.json())

# List leases
response = requests.get(f"{base_url}/leases", auth=auth, verify=False)
leases = response.json()
print(json.dumps(leases, indent=2))

# Create static lease
lease_data = {
    "mac": "00:11:22:33:44:55",
    "ip": "192.168.1.50",
    "hostname": "server1"
}
response = requests.post(
    f"{base_url}/leases",
    json=lease_data,
    auth=auth,
    verify=False
)
print(response.json())
```

### Bash Example

```bash
#!/bin/bash

API_URL="https://your-server:8081/api/v1"
USERNAME="admin"
PASSWORD="password"

# Get status
curl -k -u "$USERNAME:$PASSWORD" "$API_URL/status"

# List leases
curl -k -u "$USERNAME:$PASSWORD" "$API_URL/leases"

# Create lease
curl -k -X POST -u "$USERNAME:$PASSWORD" \
  -H "Content-Type: application/json" \
  -d '{
    "mac": "00:11:22:33:44:55",
    "ip": "192.168.1.50",
    "hostname": "server1"
  }' \
  "$API_URL/leases"
```

## Monitoring

### Real-time Monitoring

The web UI provides real-time monitoring of:
- Request rate (RPS)
- Active leases
- Subnet utilization
- Performance metrics
- Security events

### Alerts

Configure alerts for:
- High request rate
- Lease pool exhaustion
- Server failures
- Security events

## Security

### SSL/TLS Configuration

```bash
# Generate SSL certificate
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes

# Update configuration
{
  "dhcp": {
    "enterprise": {
      "management": {
        "web_ui": {
          "ssl_cert": "/etc/ssl/certs/simple-dhcpd.crt",
          "ssl_key": "/etc/ssl/private/simple-dhcpd.key"
        }
      }
    }
  }
}
```

### Access Control

- User authentication
- Role-based access control (RBAC)
- API key management
- Rate limiting

## Related Documentation

- [Configuration Guide](configuration.md) - Management configuration
- [REST API Reference](api.md) - Complete API documentation
- [Security Guide](security.md) - Security configuration

---

**Last Updated:** December 2024  
**Version:** Foundation Complete
