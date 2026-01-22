# Maintenance Procedures

This guide covers maintenance procedures for Simple DHCP Daemon.

## Overview

Regular maintenance is essential for reliable operation. This guide covers maintenance tasks, schedules, and procedures.

## Maintenance Tasks

### Daily Tasks

- [ ] Check service status
- [ ] Review error logs
- [ ] Monitor performance metrics
- [ ] Verify backup completion

### Weekly Tasks

- [ ] Review security logs
- [ ] Check disk space
- [ ] Review lease database
- [ ] Update documentation

### Monthly Tasks

- [ ] Review configuration
- [ ] Performance analysis
- [ ] Security audit
- [ ] Backup verification

### Quarterly Tasks

- [ ] Software updates
- [ ] Configuration review
- [ ] Disaster recovery test
- [ ] Capacity planning

## Service Maintenance

### Service Restart

```bash
# Restart service
sudo systemctl restart simple-dhcpd

# Graceful restart
sudo systemctl reload simple-dhcpd

# Check status
sudo systemctl status simple-dhcpd
```

### Configuration Reload

```bash
# Reload configuration
sudo systemctl reload simple-dhcpd

# Or send SIGHUP
sudo kill -HUP $(pidof simple-dhcpd)
```

### Service Stop/Start

```bash
# Stop service
sudo systemctl stop simple-dhcpd

# Start service
sudo systemctl start simple-dhcpd
```

## Database Maintenance

### Lease Database Cleanup

```bash
# Clean expired leases
simple-dhcpd --cleanup-leases

# Optimize database
simple-dhcpd --optimize-database

# Check database integrity
simple-dhcpd --check-database
```

### Database Backup

```bash
# Backup database
simple-dhcpd --backup-database /backup/leases.db

# Verify backup
simple-dhcpd --verify-backup /backup/leases.db
```

## Log Maintenance

### Log Rotation

```bash
# Manual log rotation
sudo logrotate -f /etc/logrotate.d/simple-dhcpd

# Check log rotation configuration
cat /etc/logrotate.d/simple-dhcpd
```

### Log Cleanup

```bash
# Remove old logs
find /var/log/simple-dhcpd -name "*.log.*" -mtime +30 -delete

# Compress old logs
find /var/log/simple-dhcpd -name "*.log" -mtime +7 -exec gzip {} \;
```

### Log Analysis

```bash
# Analyze error patterns
grep "ERROR" /var/log/simple-dhcpd.log | \
    awk '{print $5}' | sort | uniq -c | sort -rn

# Find most common errors
grep "ERROR" /var/log/simple-dhcpd.log | \
    cut -d' ' -f5- | sort | uniq -c | sort -rn | head -10
```

## Configuration Maintenance

### Configuration Review

```bash
# Show current configuration
simple-dhcpd --show-config

# Validate configuration
simple-dhcpd --test-config

# Compare configurations
diff /etc/simple-dhcpd/simple-dhcpd.conf /etc/simple-dhcpd/simple-dhcpd.conf.backup
```

### Configuration Updates

```bash
# Backup current configuration
sudo cp /etc/simple-dhcpd/simple-dhcpd.conf \
    /etc/simple-dhcpd/simple-dhcpd.conf.$(date +%Y%m%d)

# Edit configuration
sudo nano /etc/simple-dhcpd/simple-dhcpd.conf

# Validate changes
simple-dhcpd --test-config

# Reload configuration
sudo systemctl reload simple-dhcpd
```

## Performance Maintenance

### Performance Monitoring

```bash
# Get performance statistics
simple-dhcpd --stats

# Monitor resource usage
top -p $(pidof simple-dhcpd)

# Check network performance
sudo tcpdump -i eth0 port 67 -c 100
```

### Performance Tuning

```bash
# Review performance configuration
grep -i performance /etc/simple-dhcpd/simple-dhcpd.conf

# Apply performance tuning
# See performance-tuning.md for details
```

## Security Maintenance

### Security Audit

```bash
# Review security logs
grep -i "security\|denied\|violation" /var/log/simple-dhcpd/security.log

# Check for unauthorized access
grep -i "unauthorized" /var/log/simple-dhcpd/security.log

# Review authentication failures
grep -i "authentication failed" /var/log/simple-dhcpd/security.log
```

### Security Updates

```bash
# Check for updates
sudo apt update
sudo apt list --upgradable | grep simple-dhcpd

# Apply security updates
sudo apt upgrade simple-dhcpd
```

## Network Maintenance

### Network Interface Check

```bash
# Check interface status
ip link show

# Check IP configuration
ip addr show

# Check routing
ip route show
```

### Network Connectivity

```bash
# Test connectivity
ping -c 3 192.168.1.1

# Check DHCP traffic
sudo tcpdump -i eth0 port 67 -c 10
```

## Maintenance Schedule

### Recommended Schedule

**Daily:**
- Service status check
- Error log review
- Performance monitoring

**Weekly:**
- Security log review
- Disk space check
- Lease database review

**Monthly:**
- Configuration review
- Performance analysis
- Security audit
- Backup verification

**Quarterly:**
- Software updates
- Disaster recovery test
- Capacity planning
- Documentation update

## Maintenance Procedures

### Pre-Maintenance Checklist

- [ ] Notify users of maintenance window
- [ ] Backup configuration and data
- [ ] Document current state
- [ ] Prepare rollback plan

### During Maintenance

- [ ] Follow maintenance procedures
- [ ] Document changes
- [ ] Test changes
- [ ] Monitor for issues

### Post-Maintenance

- [ ] Verify service status
- [ ] Test functionality
- [ ] Review logs
- [ ] Update documentation
- [ ] Notify users of completion

## Troubleshooting Maintenance Issues

### Common Issues

1. **Service Won't Start**: Check configuration and logs
2. **Performance Degradation**: Review recent changes
3. **Database Errors**: Check database integrity
4. **Network Issues**: Verify network configuration

### Recovery Procedures

```bash
# Rollback configuration
sudo cp /etc/simple-dhcpd/simple-dhcpd.conf.backup \
    /etc/simple-dhcpd/simple-dhcpd.conf
sudo systemctl reload simple-dhcpd

# Restore database
sudo systemctl stop simple-dhcpd
sudo cp /backup/leases.db /var/lib/simple-dhcpd/leases.db
sudo systemctl start simple-dhcpd
```

## Maintenance Documentation

### Document Maintenance Activities

- Date and time
- Tasks performed
- Changes made
- Issues encountered
- Resolution steps
- Next steps

### Maintenance Log

```bash
# Create maintenance log entry
echo "$(date): Configuration updated" >> /var/log/simple-dhcpd/maintenance.log
```

## Best Practices

### Recommendations

1. **Schedule Regular Maintenance**: Plan maintenance windows
2. **Document Everything**: Keep detailed maintenance logs
3. **Test Changes**: Test in staging before production
4. **Backup First**: Always backup before changes
5. **Monitor After Changes**: Monitor after maintenance
6. **Review Regularly**: Review maintenance procedures

## Additional Resources

- [Deployment Guide](deployment-guide.md)
- [Backup Procedures](backup-procedures.md)
- [Monitoring Setup](monitoring.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
