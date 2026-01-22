# Datacenter Version Installation Guide

**Version:** Planned
**License:** BSL 1.1

---

## Overview

This guide covers installing the Datacenter Version of Simple DHCP Daemon for large-scale datacenter deployments.

## System Requirements

### Minimum Requirements (Per Node)
- **CPU:** 4+ cores, 3+ GHz
- **RAM:** 16+ GB
- **Disk:** 100+ GB free space (SSD recommended)
- **Network:** 10 Gigabit Ethernet
- **OS:** Linux (kernel 5.0+), Kubernetes 1.20+

### Recommended Requirements (Per Node)
- **CPU:** 8+ cores, 3.5+ GHz
- **RAM:** 32+ GB
- **Disk:** 500+ GB free space (NVMe SSD)
- **Network:** 25/100 Gigabit Ethernet
- **OS:** Linux (kernel 5.10+), Kubernetes 1.24+

### Cluster Requirements
- **Minimum Nodes:** 3 (for HA)
- **Recommended Nodes:** 5+ (for production)
- **Network:** Low-latency interconnect (<1ms)
- **Storage:** Shared or replicated storage

## Installation Methods

### Kubernetes Installation (Recommended)

```yaml
apiVersion: v1
kind: Namespace
metadata:
  name: simple-dhcpd-datacenter
---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: simple-dhcpd-datacenter
  namespace: simple-dhcpd-datacenter
spec:
  serviceName: simple-dhcpd-datacenter
  replicas: 3
  selector:
    matchLabels:
      app: simple-dhcpd-datacenter
  template:
    metadata:
      labels:
        app: simple-dhcpd-datacenter
    spec:
      containers:
      - name: simple-dhcpd
        image: simpledaemons/simple-dhcpd:datacenter
        ports:
        - containerPort: 67
          protocol: UDP
        - containerPort: 8080
          protocol: TCP
        - containerPort: 8081
          protocol: TCP
        env:
        - name: CLUSTER_MODE
          value: "enabled"
        - name: NODE_NAME
          valueFrom:
            fieldRef:
              fieldPath: metadata.name
        volumeMounts:
        - name: config
          mountPath: /etc/simple-dhcpd
        - name: data
          mountPath: /var/lib/simple-dhcpd
  volumeClaimTemplates:
  - metadata:
      name: data
    spec:
      accessModes: [ "ReadWriteOnce" ]
      storageClassName: fast-ssd
      resources:
        requests:
          storage: 100Gi
```

### Docker Swarm Installation

```yaml
version: '3.8'
services:
  simple-dhcpd-datacenter:
    image: simpledaemons/simple-dhcpd:datacenter
    deploy:
      replicas: 3
      placement:
        constraints:
          - node.role == worker
      update_config:
        parallelism: 1
        delay: 10s
    ports:
      - "67:67/udp"
      - "8080:8080"
      - "8081:8081"
    volumes:
      - config:/etc/simple-dhcpd
      - data:/var/lib/simple-dhcpd
    networks:
      - dhcp-network
volumes:
  config:
  data:
networks:
  dhcp-network:
    driver: overlay
```

### Package Installation

**Ubuntu/Debian:**
```bash
# Add Datacenter repository
wget -qO - https://packages.simpledaemons.org/gpg.key | sudo apt-key add -
echo "deb https://packages.simpledaemons.org/ubuntu datacenter main" | sudo tee /etc/apt/sources.list.d/simpledaemons-datacenter.list

# Install
sudo apt update
sudo apt install simple-dhcpd-datacenter
```

## Post-Installation

### Cluster Configuration

```json
{
  "dhcp": {
    "datacenter": {
      "cluster": {
        "enabled": true,
        "mode": "distributed",
        "nodes": [
          "node1.example.com",
          "node2.example.com",
          "node3.example.com"
        ],
        "discovery": {
          "method": "kubernetes",
          "namespace": "simple-dhcpd-datacenter"
        }
      }
    }
  }
}
```

### Multi-Site Configuration

```json
{
  "dhcp": {
    "datacenter": {
      "multi_site": {
        "enabled": true,
        "sites": [
          {
            "name": "us-east",
            "nodes": ["node1-us-east", "node2-us-east"],
            "priority": 1
          },
          {
            "name": "us-west",
            "nodes": ["node1-us-west", "node2-us-west"],
            "priority": 2
          }
        ]
      }
    }
  }
}
```

## Verification

### Check Cluster Status

```bash
# Check cluster status
simple-dhcpd-datacenter --cluster-status

# Check node status
simple-dhcpd-datacenter --node-status

# Check site status
simple-dhcpd-datacenter --site-status
```

### Access Management Interface

```bash
# Access web UI
https://your-cluster:8080

# Access API
curl -k https://your-cluster:8081/api/v1/status
```

## Next Steps

1. **[Multi-Site Setup](multi-site.md)** - Configure multi-site deployment
2. **[Scaling Guide](scaling.md)** - Configure horizontal scaling
3. **[Cloud Deployment](cloud.md)** - Deploy to cloud platforms
4. **[Monitoring](monitoring.md)** - Set up monitoring

## Troubleshooting

### Cluster Issues

```bash
# Check cluster connectivity
simple-dhcpd-datacenter --cluster-ping

# Check node health
simple-dhcpd-datacenter --node-health

# Check network
ping cluster-nodes
```

### Performance Issues

```bash
# Check cluster metrics
curl -k https://your-cluster:8081/api/v1/cluster/metrics

# Check node performance
simple-dhcpd-datacenter --node-performance
```

## License Activation

The Datacenter Version requires license activation for production use. See [Licensing](../LICENSING.md) for details.

---

**Last Updated:** December 2024
**Version:** Planned
