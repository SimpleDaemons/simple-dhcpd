# Monitoring Setup

This guide covers setting up monitoring for Simple DHCP Daemon.

## Overview

Monitoring is essential for maintaining reliable DHCP service. This guide covers monitoring setup, metrics, and alerting.

## Monitoring Components

### Service Health Monitoring

```bash
# Check service status
sudo systemctl status simple-dhcpd

# Health check script
#!/bin/bash
if systemctl is-active --quiet simple-dhcpd; then
    echo "Service is running"
    exit 0
else
    echo "Service is not running"
    exit 1
fi
```

### Process Monitoring

```bash
# Monitor process
ps aux | grep simple-dhcpd

# Monitor resource usage
top -p $(pidof simple-dhcpd)

# Monitor file descriptors
lsof -p $(pidof simple-dhcpd) | wc -l
```

## Metrics Collection

### Built-in Metrics

Simple DHCP Daemon provides built-in metrics:

```bash
# Get statistics
simple-dhcpd --stats

# Get metrics in JSON format
simple-dhcpd --stats --format json
```

### Key Metrics

- **Requests Per Second (RPS)**: DHCP request rate
- **Active Leases**: Number of active leases
- **Memory Usage**: RAM consumption
- **CPU Usage**: Processor utilization
- **Response Time**: Average response time
- **Error Rate**: Error percentage

## Log Monitoring

### Log File Monitoring

```bash
# Monitor main log
tail -f /var/log/simple-dhcpd.log

# Monitor error log
tail -f /var/log/simple-dhcpd/error.log

# Monitor security log
tail -f /var/log/simple-dhcpd/security.log
```

### Log Analysis

```bash
# Count errors
grep -c "ERROR" /var/log/simple-dhcpd.log

# Find recent errors
grep "ERROR" /var/log/simple-dhcpd.log | tail -20

# Analyze error patterns
grep "ERROR" /var/log/simple-dhcpd.log | awk '{print $5}' | sort | uniq -c
```

## Network Monitoring

### Network Traffic Monitoring

```bash
# Monitor DHCP traffic
sudo tcpdump -i eth0 port 67 -v

# Count DHCP packets
sudo tcpdump -i eth0 port 67 -c 100

# Save packets for analysis
sudo tcpdump -i eth0 port 67 -w dhcp.pcap
```

### Network Interface Monitoring

```bash
# Monitor interface statistics
watch -n 1 'cat /proc/net/dev'

# Monitor interface errors
ethtool -S eth0
```

## Performance Monitoring

### Resource Monitoring

```bash
# Monitor CPU and memory
top -p $(pidof simple-dhcpd)

# Monitor disk I/O
iostat -x 1

# Monitor network I/O
iftop -i eth0
```

### Performance Metrics

```bash
# Get performance statistics
simple-dhcpd --stats --performance

# Monitor response times
watch -n 1 'simple-dhcpd --stats | grep response_time'
```

## Alerting Setup

### Basic Alerting Script

```bash
#!/bin/bash
# health_check.sh

SERVICE="simple-dhcpd"
LOG_FILE="/var/log/simple-dhcpd.log"
ALERT_EMAIL="admin@example.com"

# Check service status
if ! systemctl is-active --quiet $SERVICE; then
    echo "ALERT: $SERVICE is not running" | mail -s "DHCP Service Alert" $ALERT_EMAIL
    exit 1
fi

# Check for errors in log
ERROR_COUNT=$(grep -c "ERROR" $LOG_FILE | tail -100)
if [ $ERROR_COUNT -gt 10 ]; then
    echo "ALERT: High error rate detected: $ERROR_COUNT errors" | mail -s "DHCP Error Alert" $ALERT_EMAIL
fi

exit 0
```

### Cron-based Monitoring

```bash
# Add to crontab
*/5 * * * * /usr/local/bin/health_check.sh
```

## Monitoring Tools Integration

### Prometheus Integration

```yaml
# prometheus.yml
scrape_configs:
  - job_name: 'simple-dhcpd'
    static_configs:
      - targets: ['localhost:8080']
    metrics_path: '/metrics'
```

### Grafana Dashboard

Create Grafana dashboard with:
- Service status
- Request rate
- Active leases
- Error rate
- Response time
- Resource usage

### SNMP Monitoring

```json
{
  "dhcp": {
    "monitoring": {
      "snmp": {
        "enabled": true,
        "community": "public",
        "port": 161
      }
    }
  }
}
```

## Health Checks

### HTTP Health Endpoint

```bash
# Check health endpoint
curl http://localhost:8080/health

# Health check script
#!/bin/bash
if curl -f http://localhost:8080/health > /dev/null 2>&1; then
    exit 0
else
    exit 1
fi
```

### Custom Health Checks

```bash
# Check lease database
simple-dhcpd --check-database

# Check configuration
simple-dhcpd --test-config

# Check network connectivity
ping -c 1 192.168.1.1
```

## Monitoring Best Practices

### Recommendations

1. **Monitor Key Metrics**: Focus on critical metrics
2. **Set Appropriate Thresholds**: Configure realistic alert thresholds
3. **Regular Reviews**: Review monitoring setup regularly
4. **Document Alerts**: Document alert procedures
5. **Test Alerts**: Regularly test alerting mechanisms

### Monitoring Checklist

- [ ] Service health monitoring configured
- [ ] Metrics collection enabled
- [ ] Log monitoring set up
- [ ] Network monitoring configured
- [ ] Performance monitoring active
- [ ] Alerting configured
- [ ] Dashboard created
- [ ] Alert procedures documented

## Troubleshooting Monitoring

### Common Issues

1. **Metrics Not Available**: Check service status and configuration
2. **High Alert Volume**: Adjust thresholds
3. **Missing Metrics**: Verify monitoring configuration
4. **Performance Impact**: Optimize monitoring overhead

## Additional Resources

- [Deployment Guide](deployment-guide.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
- [Performance Tuning](../user-guide/performance-tuning.md)
