# Enterprise Version Installation Guide

**Version:** Foundation Complete (v1.0.0 Alpha in development)  
**License:** BSL 1.1

---

## Overview

This guide covers installing the Enterprise Version of Simple DHCP Daemon. The Enterprise Version includes all Production Version features plus advanced enterprise capabilities.

## System Requirements

### Minimum Requirements
- **CPU:** 2+ cores, 2+ GHz
- **RAM:** 4+ GB
- **Disk:** 10+ GB free space (for database and logs)
- **Network:** Gigabit Ethernet
- **OS:** Linux (kernel 4.0+), macOS 11+, Windows Server 2019+

### Recommended Requirements
- **CPU:** 4+ cores, 3+ GHz
- **RAM:** 8+ GB
- **Disk:** 50+ GB free space (SSD recommended)
- **Network:** 10 Gigabit Ethernet
- **OS:** Linux (kernel 5.0+), macOS 12+, Windows Server 2022+

## Installation Methods

### Package Installation

**Ubuntu/Debian:**
```bash
# Add Enterprise repository
wget -qO - https://packages.simpledaemons.org/gpg.key | sudo apt-key add -
echo "deb https://packages.simpledaemons.org/ubuntu enterprise main" | sudo tee /etc/apt/sources.list.d/simpledaemons-enterprise.list

# Install
sudo apt update
sudo apt install simple-dhcpd-enterprise
```

**CentOS/RHEL:**
```bash
# Add Enterprise repository
sudo yum-config-manager --add-repo https://packages.simpledaemons.org/rhel/enterprise.repo

# Install
sudo yum install simple-dhcpd-enterprise
```

**macOS:**
```bash
# Add Enterprise tap
brew tap simpledaemons/enterprise

# Install
brew install simple-dhcpd-enterprise
```

### Docker Installation

```bash
# Pull Enterprise image
docker pull simpledaemons/simple-dhcpd:enterprise

# Run container
docker run -d \
  --name simple-dhcpd-enterprise \
  --net host \
  -v /etc/simple-dhcpd:/etc/simple-dhcpd \
  -v /var/lib/simple-dhcpd:/var/lib/simple-dhcpd \
  -v /var/log/simple-dhcpd:/var/log/simple-dhcpd \
  simpledaemons/simple-dhcpd:enterprise
```

### Kubernetes Installation

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: simple-dhcpd-enterprise
spec:
  replicas: 2
  selector:
    matchLabels:
      app: simple-dhcpd-enterprise
  template:
    metadata:
      labels:
        app: simple-dhcpd-enterprise
    spec:
      containers:
      - name: simple-dhcpd
        image: simpledaemons/simple-dhcpd:enterprise
        ports:
        - containerPort: 67
          protocol: UDP
        - containerPort: 8080
          protocol: TCP
        volumeMounts:
        - name: config
          mountPath: /etc/simple-dhcpd
```

## Post-Installation

### Create Directories

```bash
# Configuration directory
sudo mkdir -p /etc/simple-dhcpd
sudo chown dhcpd:dhcpd /etc/simple-dhcpd

# Lease database directory
sudo mkdir -p /var/lib/simple-dhcpd
sudo chown dhcpd:dhcpd /var/lib/simple-dhcpd

# Log directory
sudo mkdir -p /var/log/simple-dhcpd
sudo chown dhcpd:dhcpd /var/log/simple-dhcpd

# Web UI directory
sudo mkdir -p /var/www/simple-dhcpd
sudo chown dhcpd:dhcpd /var/www/simple-dhcpd
```

### Create System User

```bash
# Create dhcpd user
sudo useradd -r -s /bin/false -d /var/lib/simple-dhcpd dhcpd
```

## Initial Configuration

### Basic Enterprise Configuration

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
    ],
    "enterprise": {
      "high_availability": {
        "enabled": false
      },
      "management": {
        "web_ui": {
          "enabled": true,
          "port": 8080,
          "ssl": true
        },
        "rest_api": {
          "enabled": true,
          "port": 8081,
          "authentication": true
        }
      }
    }
  }
}
```

## Verification

### Check Installation

```bash
# Check version
simple-dhcpd-enterprise --version

# Check Enterprise features
simple-dhcpd-enterprise --features

# Test configuration
simple-dhcpd-enterprise -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
```

### Start Service

```bash
# Start service
sudo systemctl start simple-dhcpd-enterprise

# Enable on boot
sudo systemctl enable simple-dhcpd-enterprise

# Check status
sudo systemctl status simple-dhcpd-enterprise
```

### Access Web UI

```bash
# Access web UI (default port 8080)
https://your-server:8080

# Default credentials (change on first login)
# Username: admin
# Password: admin
```

### Test REST API

```bash
# Test API endpoint
curl -k https://your-server:8081/api/v1/status

# With authentication
curl -k -u admin:admin https://your-server:8081/api/v1/status
```

## Next Steps

1. **[Configuration Guide](configuration.md)** - Configure Enterprise features
2. **[High Availability Setup](high-availability.md)** - Configure HA if needed
3. **[Management Interface](management.md)** - Set up web UI and REST API
4. **[Integrations](integrations.md)** - Configure DNS, LDAP, SNMP integrations

## Troubleshooting

### Common Issues

**Port Already in Use:**
```bash
# Check ports
sudo netstat -tulpn | grep -E ':(67|8080|8081)'
```

**Permission Denied:**
```bash
# Fix permissions
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd
sudo chown -R dhcpd:dhcpd /var/lib/simple-dhcpd
```

**Web UI Not Accessible:**
```bash
# Check firewall
sudo firewall-cmd --list-all

# Allow ports
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --permanent --add-port=8081/tcp
sudo firewall-cmd --reload
```

## License Activation

The Enterprise Version requires license activation for production use. See [Licensing](../LICENSING.md) for details.

---

**Last Updated:** December 2024  
**Version:** Foundation Complete
