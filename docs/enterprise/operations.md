# Enterprise Version Operations Guide

**Version:** Foundation Complete (v1.0.0 Alpha in development)
**License:** BSL 1.1

---

## Overview

This guide covers day-to-day operations for the Enterprise Version, including service management, monitoring, backup, and maintenance.

## Service Management

### Start/Stop Service

```bash
# Start
sudo systemctl start simple-dhcpd-enterprise

# Stop
sudo systemctl stop simple-dhcpd-enterprise

# Restart
sudo systemctl restart simple-dhcpd-enterprise

# Reload configuration
sudo systemctl reload simple-dhcpd-enterprise
```

### Service Status

```bash
# Check status
sudo systemctl status simple-dhcpd-enterprise

# Check HA status
simple-dhcpd-enterprise --ha-status

# Check all services
simple-dhcpd-enterprise --status-all
```

## Monitoring

### Health Checks

```bash
# API health check
curl -k https://your-server:8081/api/v1/health

# HA health check
curl -k https://your-server:8081/api/v1/ha/health

# Integration health checks
curl -k https://your-server:8081/api/v1/integrations/health
```

### Metrics Collection

```bash
# Get metrics
curl -k https://your-server:8081/api/v1/metrics

# Get performance metrics
curl -k https://your-server:8081/api/v1/metrics/performance

# Get lease metrics
curl -k https://your-server:8081/api/v1/metrics/leases
```

## Backup and Recovery

### Backup Configuration

```bash
# Backup configuration
sudo cp /etc/simple-dhcpd/simple-dhcpd.conf /backup/simple-dhcpd.conf.$(date +%Y%m%d)

# Backup lease database
sudo cp /var/lib/simple-dhcpd/leases.db /backup/leases.db.$(date +%Y%m%d)

# Backup state files
sudo cp /var/lib/simple-dhcpd/failover.state /backup/failover.state.$(date +%Y%m%d)
```

### Automated Backup

```bash
#!/bin/bash
# Backup script
BACKUP_DIR="/backup/simple-dhcpd"
DATE=$(date +%Y%m%d_%H%M%S)

mkdir -p "$BACKUP_DIR/$DATE"

# Backup files
cp /etc/simple-dhcpd/* "$BACKUP_DIR/$DATE/"
cp /var/lib/simple-dhcpd/* "$BACKUP_DIR/$DATE/"

# Compress
tar -czf "$BACKUP_DIR/simple-dhcpd-$DATE.tar.gz" "$BACKUP_DIR/$DATE"
rm -rf "$BACKUP_DIR/$DATE"

# Keep only last 30 days
find "$BACKUP_DIR" -name "*.tar.gz" -mtime +30 -delete
```

## Maintenance

### Regular Maintenance Tasks

1. **Daily:**
   - Check service status
   - Review logs
   - Monitor metrics

2. **Weekly:**
   - Review security events
   - Check disk space
   - Verify backups

3. **Monthly:**
   - Performance review
   - Configuration review
   - Update documentation

### Log Rotation

```bash
# Configure log rotation
sudo nano /etc/logrotate.d/simple-dhcpd-enterprise

# Content:
/var/log/simple-dhcpd/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0640 dhcpd dhcpd
    sharedscripts
    postrotate
        systemctl reload simple-dhcpd-enterprise > /dev/null 2>&1 || true
    endscript
}
```

## Troubleshooting

### Common Issues

**Service Won't Start:**
```bash
# Check logs
sudo journalctl -u simple-dhcpd-enterprise -n 50

# Check configuration
sudo simple-dhcpd-enterprise -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check permissions
ls -la /etc/simple-dhcpd/
ls -la /var/lib/simple-dhcpd/
```

**HA Not Working:**
```bash
# Check HA status
simple-dhcpd-enterprise --ha-status

# Check network
ping peer-server-ip

# Check firewall
sudo firewall-cmd --list-all
```

**Performance Issues:**
```bash
# Check metrics
curl -k https://your-server:8081/api/v1/metrics/performance

# Check system resources
top
iostat
netstat -i
```

## Related Documentation

- [High Availability](high-availability.md) - HA operations
- [Management Interface](management.md) - Management operations
- [Monitoring Guide](../shared/deployment/monitoring.md) - Monitoring setup

---

**Last Updated:** December 2024
**Version:** Foundation Complete
