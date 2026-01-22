# Datacenter Version Multi-Site Guide

**Version:** Planned
**License:** BSL 1.1

---

## Overview

The Datacenter Version supports multi-site deployments, allowing DHCP services to span multiple datacenters with automatic synchronization and failover.

## Multi-Site Architecture

```
┌─────────────────────────────────┐
│      Site 1 (US East)           │
│  ┌──────────┐  ┌──────────┐    │
│  │  Node 1  │  │  Node 2  │    │
│  └────┬─────┘  └────┬─────┘    │
│       └──────┬───────┘          │
└──────────────┼──────────────────┘
               │
        ┌──────▼──────┐
        │  Sync Layer │
        └──────┬──────┘
               │
┌──────────────┼──────────────────┐
│      Site 2 (US West)            │
│  ┌──────────┐  ┌──────────┐    │
│  │  Node 1  │  │  Node 2  │    │
│  └────┬─────┘  └────┬─────┘    │
│       └──────┬───────┘          │
└──────────────┼──────────────────┘
```

## Configuration

### Site Configuration

```json
{
  "dhcp": {
    "datacenter": {
      "multi_site": {
        "enabled": true,
        "local_site": "us-east",
        "sites": [
          {
            "name": "us-east",
            "nodes": [
              "node1-us-east.example.com",
              "node2-us-east.example.com"
            ],
            "priority": 1,
            "latency": 0
          },
          {
            "name": "us-west",
            "nodes": [
              "node1-us-west.example.com",
              "node2-us-west.example.com"
            ],
            "priority": 2,
            "latency": 50
          },
          {
            "name": "eu-central",
            "nodes": [
              "node1-eu.example.com",
              "node2-eu.example.com"
            ],
            "priority": 3,
            "latency": 100
          }
        ]
      }
    }
  }
}
```

### Synchronization Configuration

```json
{
  "dhcp": {
    "datacenter": {
      "multi_site": {
        "synchronization": {
          "mode": "async",
          "interval": 60,
          "batch_size": 1000,
          "compression": true,
          "encryption": true
        }
      }
    }
  }
}
```

## Site-Aware Allocation

### Geographic Load Balancing

```json
{
  "dhcp": {
    "datacenter": {
      "multi_site": {
        "allocation": {
          "strategy": "geographic",
          "prefer_local": true,
          "fallback_sites": ["us-west", "eu-central"]
        }
      }
    }
  }
}
```

## Cross-Site Failover

### Automatic Failover

```json
{
  "dhcp": {
    "datacenter": {
      "multi_site": {
        "failover": {
          "enabled": true,
          "detection_interval": 10,
          "failover_timeout": 30,
          "preferred_sites": ["us-east", "us-west"]
        }
      }
    }
  }
}
```

## Bandwidth Optimization

### Bandwidth Management

```json
{
  "dhcp": {
    "datacenter": {
      "multi_site": {
        "bandwidth": {
          "optimization": true,
          "max_bandwidth": 1000,
          "compression": true,
          "deduplication": true
        }
      }
    }
  }
}
```

## Monitoring

### Site Status

```bash
# Check site status
simple-dhcpd-datacenter --site-status

# Check site health
curl -k https://your-cluster:8081/api/v1/sites/status

# Response:
{
  "sites": [
    {
      "name": "us-east",
      "status": "active",
      "nodes": 2,
      "latency": 0,
      "last_sync": "2024-12-01T10:00:00Z"
    },
    {
      "name": "us-west",
      "status": "active",
      "nodes": 2,
      "latency": 50,
      "last_sync": "2024-12-01T10:00:00Z"
    }
  ]
}
```

## Best Practices

1. **Network:** Use dedicated links for site synchronization
2. **Latency:** Minimize latency between sites
3. **Monitoring:** Monitor site health and synchronization
4. **Testing:** Regularly test cross-site failover
5. **Documentation:** Document site configurations

## Related Documentation

- [Configuration Guide](configuration.md) - Multi-site configuration
- [Scaling Guide](scaling.md) - Multi-site scaling
- [Cloud Deployment](cloud.md) - Cloud multi-site setup

---

**Last Updated:** December 2024
**Version:** Planned
