# Production Version Operations Guide

**Version:** 1.0.0  
**License:** Apache 2.0

---

## Overview

This guide covers day-to-day operations for the Production Version of Simple DHCP Daemon.

## Service Management

### Start/Stop Service

```bash
# Start
sudo systemctl start simple-dhcpd

# Stop
sudo systemctl stop simple-dhcpd

# Restart
sudo systemctl restart simple-dhcpd

# Reload configuration
sudo systemctl reload simple-dhcpd
```

### Check Status

```bash
# Service status
sudo systemctl status simple-dhcpd

# Check logs
sudo journalctl -u simple-dhcpd -f
```

## Configuration Management

### Reload Configuration

```bash
# Reload without restart
sudo systemctl reload simple-dhcpd

# Or send SIGHUP
sudo killall -HUP simple-dhcpd
```

### Validate Configuration

```bash
sudo simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
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
