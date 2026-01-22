# Production Version Deployment Guide

**Version:** 1.0.0  
**License:** Apache 2.0

---

## Overview

This guide covers deploying the Production Version of Simple DHCP Daemon in production environments.

## Pre-Deployment Checklist

- [ ] System requirements met
- [ ] Network configuration planned
- [ ] IP address ranges allocated
- [ ] Configuration file prepared
- [ ] Firewall rules configured
- [ ] Monitoring setup planned
- [ ] Backup strategy defined

## Deployment Methods

### Package Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install simple-dhcpd
```

**CentOS/RHEL:**
```bash
sudo yum install simple-dhcpd
```

**macOS:**
```bash
brew install simple-dhcpd
```

### Docker Deployment

```bash
docker run -d \
  --name simple-dhcpd \
  --net host \
  -v /etc/simple-dhcpd:/etc/simple-dhcpd \
  -v /var/lib/simple-dhcpd:/var/lib/simple-dhcpd \
  -v /var/log/simple-dhcpd:/var/log/simple-dhcpd \
  simpledaemons/simple-dhcpd:production
```

### Systemd Service

```bash
# Enable and start service
sudo systemctl enable simple-dhcpd
sudo systemctl start simple-dhcpd

# Check status
sudo systemctl status simple-dhcpd
```

## Configuration

1. **Create Configuration Directory**
```bash
sudo mkdir -p /etc/simple-dhcpd
sudo chown dhcpd:dhcpd /etc/simple-dhcpd
```

2. **Create Configuration File**
```bash
sudo nano /etc/simple-dhcpd/simple-dhcpd.conf
```

3. **Validate Configuration**
```bash
sudo simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
```

## Network Configuration

### Firewall Rules

**UFW (Ubuntu):**
```bash
sudo ufw allow 67/udp
sudo ufw allow 68/udp
```

**firewalld (CentOS/RHEL):**
```bash
sudo firewall-cmd --permanent --add-service=dhcp
sudo firewall-cmd --reload
```

### Network Interface

Ensure the DHCP server can bind to the correct interface:
```bash
# Check interface
ip addr show

# Configure listen address in config
"listen_addresses": ["192.168.1.10"]
```

## Verification

### Test DHCP Server

```bash
# Start server
sudo systemctl start simple-dhcpd

# Check logs
sudo journalctl -u simple-dhcpd -f

# Test from client
sudo dhclient -v eth0
```

### Verify Leases

```bash
# Check lease database
sudo cat /var/lib/simple-dhcpd/leases.db
```

## Monitoring

See [Monitoring Guide](../shared/deployment/monitoring.md) for monitoring setup.

## Backup

See [Backup Procedures](../shared/deployment/backup-procedures.md) for backup configuration.

## Maintenance

See [Maintenance Procedures](../shared/deployment/maintenance-procedures.md) for maintenance tasks.

## Troubleshooting

See [Troubleshooting Guide](../shared/troubleshooting/README.md) for common issues.

---

**Last Updated:** December 2024  
**Version:** 1.0.0
