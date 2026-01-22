# Troubleshooting Guide

This guide helps you diagnose and resolve common issues with Simple DHCP Daemon.

## Quick Diagnostics

### Check Service Status

```bash
# Check if service is running
sudo systemctl status simple-dhcpd

# Check process
ps aux | grep simple-dhcpd

# Check if port is listening
sudo netstat -tulpn | grep :67
sudo lsof -i :67
```

### Check Logs

```bash
# System logs
sudo journalctl -u simple-dhcpd -f

# Log file
tail -f /var/log/simple-dhcpd.log

# All log files
tail -f /var/log/simple-dhcpd/*.log
```

### Test Configuration

```bash
# Test configuration syntax
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --validate

# Show parsed configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --show-config
```

## Common Issues

### Service Won't Start

#### Issue: Permission Denied

**Symptoms:**
- Service fails to start
- Error: "Permission denied"
- Cannot access configuration files

**Solutions:**
```bash
# Check file permissions
ls -la /etc/simple-dhcpd/
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd/

# Check if running as root
sudo simple-dhcpd -f

# Fix directory permissions
sudo chmod 755 /etc/simple-dhcpd
sudo chmod 644 /etc/simple-dhcpd/simple-dhcpd.conf
```

#### Issue: Port Already in Use

**Symptoms:**
- Error: "Address already in use"
- Port 67 is occupied
- Service fails to bind

**Solutions:**
```bash
# Find what's using port 67
sudo netstat -tulpn | grep :67
sudo lsof -i :67

# Stop conflicting services
sudo systemctl stop isc-dhcp-server
sudo systemctl stop dnsmasq
sudo systemctl stop dhcpd

# Disable conflicting services
sudo systemctl disable isc-dhcp-server
sudo systemctl disable dnsmasq
```

#### Issue: Configuration Errors

**Symptoms:**
- Service fails to start
- Configuration validation errors
- JSON syntax errors

**Solutions:**
```bash
# Validate JSON syntax
python -m json.tool /etc/simple-dhcpd/simple-dhcpd.conf

# Test configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Check for common JSON errors
# - Missing commas
# - Unmatched brackets
# - Invalid quotes
```

### Clients Not Getting IP Addresses

#### Issue: Network Connectivity

**Symptoms:**
- Clients don't receive IP addresses
- No DHCP responses
- Network unreachable

**Solutions:**
```bash
# Check network interface
ip addr show
ip link show

# Check routing
ip route show

# Test connectivity
ping 192.168.1.1

# Check firewall
sudo iptables -L
sudo ufw status
```

#### Issue: Subnet Configuration

**Symptoms:**
- Clients on wrong subnet
- IP addresses outside range
- Gateway issues

**Solutions:**
```bash
# Check subnet configuration
simple-dhcpd --show-config

# Verify network ranges
# Ensure client is on correct subnet
# Check gateway configuration

# Test with specific client
sudo dhclient -v eth0
```

#### Issue: Lease Database

**Symptoms:**
- Leases not being created
- Database errors
- Storage issues

**Solutions:**
```bash
# Check lease database
sudo simple-dhcpd --show-leases

# Check database permissions
ls -la /var/lib/simple-dhcpd/

# Fix permissions
sudo chown -R dhcpd:dhcpd /var/lib/simple-dhcpd/
sudo chmod 755 /var/lib/simple-dhcpd/

# Check disk space
df -h /var/lib/simple-dhcpd/
```

### Performance Issues

#### Issue: High CPU Usage

**Symptoms:**
- High CPU utilization
- Slow response times
- System load issues

**Solutions:**
```bash
# Check statistics
sudo simple-dhcpd --stats

# Monitor performance
top -p $(pidof simple-dhcpd)
htop -p $(pidof simple-dhcpd)

# Check for high request rates
# Review rate limiting configuration
# Consider connection pooling
```

#### Issue: Memory Issues

**Symptoms:**
- High memory usage
- Out of memory errors
- System instability

**Solutions:**
```bash
# Check memory usage
free -h
ps aux --sort=-%mem | head

# Monitor memory
watch -n 1 'free -h'

# Check for memory leaks
# Review lease database size
# Consider caching configuration
```

#### Issue: Slow Response Times

**Symptoms:**
- Slow DHCP responses
- Client timeouts
- Network delays

**Solutions:**
```bash
# Check network latency
ping 192.168.1.1

# Monitor network traffic
sudo tcpdump -i eth0 port 67

# Check for network congestion
# Review performance settings
# Consider optimization
```

### Security Issues

#### Issue: Unauthorized Clients

**Symptoms:**
- Unknown clients getting IPs
- Security violations
- MAC address issues

**Solutions:**
```bash
# Check MAC filtering
# Review security configuration
# Enable MAC filtering

# Check logs for security events
grep -i "security" /var/log/simple-dhcpd.log
grep -i "denied" /var/log/simple-dhcpd.log
```

#### Issue: DHCP Attacks

**Symptoms:**
- High request rates
- Malicious traffic
- System overload

**Solutions:**
```bash
# Enable rate limiting
# Review security configuration
# Monitor for attacks

# Check for suspicious activity
sudo tcpdump -i eth0 port 67
```

## Debugging Techniques

### Enable Debug Logging

```bash
# Start with debug logging
sudo simple-dhcpd -f -v

# Or modify configuration
{
  "dhcp": {
    "logging": {
      "level": "debug"
    }
  }
}
```

### Network Packet Analysis

```bash
# Capture DHCP packets
sudo tcpdump -i eth0 port 67 -v

# Capture with detailed output
sudo tcpdump -i eth0 port 67 -X

# Save to file
sudo tcpdump -i eth0 port 67 -w dhcp.pcap
```

### Process Monitoring

```bash
# Monitor process
top -p $(pidof simple-dhcpd)

# Monitor system calls
sudo strace -p $(pidof simple-dhcpd)

# Monitor file access
sudo lsof -p $(pidof simple-dhcpd)
```

### Configuration Debugging

```bash
# Show parsed configuration
simple-dhcpd --show-config

# Test configuration
simple-dhcpd --test-config

# Validate JSON
python -m json.tool /etc/simple-dhcpd/simple-dhcpd.conf
```

## Log Analysis

### Common Log Messages

#### Info Messages
```
[INFO] DHCP server started
[INFO] Allocated lease: 00:11:22:33:44:55 -> 192.168.1.100
[INFO] Renewed lease: 00:11:22:33:44:55 -> 192.168.1.100
```

#### Warning Messages
```
[WARN] No available IP addresses in subnet: main-subnet
[WARN] Client declined IP 192.168.1.100
[WARN] Rate limit exceeded for client 00:11:22:33:44:55
```

#### Error Messages
```
[ERROR] Failed to bind socket: Address already in use
[ERROR] Configuration validation failed: Invalid subnet range
[ERROR] Database error: Permission denied
```

### Log Filtering

```bash
# Filter by level
grep "ERROR" /var/log/simple-dhcpd.log
grep "WARN" /var/log/simple-dhcpd.log

# Filter by component
grep "DHCP" /var/log/simple-dhcpd.log
grep "LEASE" /var/log/simple-dhcpd.log

# Filter by time
grep "2024-12-01" /var/log/simple-dhcpd.log
```

## Performance Tuning

### Database Optimization

```json
{
  "dhcp": {
    "performance": {
      "lease_database": {
        "type": "sqlite",
        "optimization": true,
        "compression": true
      }
    }
  }
}
```

### Caching Configuration

```json
{
  "dhcp": {
    "performance": {
      "caching": {
        "enabled": true,
        "cache_size": "500MB",
        "ttl": 3600
      }
    }
  }
}
```

### Connection Pooling

```json
{
  "dhcp": {
    "performance": {
      "connection_pooling": {
        "enabled": true,
        "max_connections": 2000,
        "idle_timeout": 300
      }
    }
  }
}
```

## Recovery Procedures

### Service Recovery

```bash
# Restart service
sudo systemctl restart simple-dhcpd

# Reload configuration
sudo systemctl reload simple-dhcpd

# Check status
sudo systemctl status simple-dhcpd
```

### Database Recovery

```bash
# Backup database
sudo cp /var/lib/simple-dhcpd/leases.db /var/lib/simple-dhcpd/leases.db.backup

# Recreate database
sudo rm /var/lib/simple-dhcpd/leases.db
sudo systemctl restart simple-dhcpd
```

### Configuration Recovery

```bash
# Restore from backup
sudo cp /etc/simple-dhcpd/simple-dhcpd.conf.backup /etc/simple-dhcpd/simple-dhcpd.conf

# Validate configuration
simple-dhcpd --test-config

# Restart service
sudo systemctl restart simple-dhcpd
```

## Getting Help

### Self-Help Resources

1. **Documentation**: Check the [full documentation](../README.md)
2. **Configuration Guide**: Review [configuration options](../configuration/README.md)
3. **CLI Reference**: Check [command line options](../user-guide/cli.md)
4. **Logs**: Analyze log files for error messages

### Community Support

1. **GitHub Issues**: [Report bugs and request features](https://github.com/SimpleDaemons/simple-dhcpd/issues)
2. **GitHub Discussions**: [Ask questions and share experiences](https://github.com/SimpleDaemons/simple-dhcpd/discussions)
3. **Community Forum**: [Join the community](https://community.simpledaemons.com)

### Professional Support

1. **Email Support**: support@simpledaemons.com
2. **Phone Support**: +1-800-SIMPLE-1
3. **Enterprise Support**: enterprise@simpledaemons.com

### Reporting Issues

When reporting issues, please include:

1. **Version**: `simple-dhcpd --version`
2. **Configuration**: `simple-dhcpd --show-config`
3. **Logs**: Relevant log entries
4. **Environment**: OS, hardware, network setup
5. **Steps to Reproduce**: Detailed steps to reproduce the issue

## Prevention

### Best Practices

1. **Regular Backups**: Backup configuration and database regularly
2. **Monitoring**: Set up monitoring and alerting
3. **Testing**: Test configurations before deployment
4. **Documentation**: Document your setup and changes
5. **Updates**: Keep the software updated

### Monitoring Setup

```bash
# Set up log monitoring
sudo tail -f /var/log/simple-dhcpd.log | grep -i error

# Set up performance monitoring
watch -n 1 'sudo simple-dhcpd --stats'

# Set up health checks
curl -f http://localhost:8080/health || echo "Service down"
```

---

**Need more help?** Check the [full documentation](../README.md) or [contact support](support/README.md).
