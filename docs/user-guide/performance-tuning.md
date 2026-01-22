# Performance Tuning Guide

This guide covers performance optimization techniques for Simple DHCP Daemon.

## Overview

Simple DHCP Daemon is designed for high performance, but proper tuning is essential for optimal operation in production environments.

## Performance Metrics

### Key Metrics to Monitor

- **Requests Per Second (RPS)**: Number of DHCP requests processed per second
- **Response Time**: Time to process and respond to DHCP requests
- **Concurrent Leases**: Number of active leases
- **Memory Usage**: RAM consumption
- **CPU Usage**: Processor utilization
- **Network Throughput**: Network bandwidth utilization

### Target Performance

- **RPS**: >10,000 requests per second
- **Response Time**: <5ms average
- **Concurrent Leases**: >10,000 active leases
- **Memory**: <50MB base + 1KB per lease
- **CPU**: <50% utilization under normal load

## Configuration Tuning

### Lease Database Optimization

```json
{
  "dhcp": {
    "performance": {
      "lease_database": {
        "type": "file",
        "path": "/var/lib/simple-dhcpd/leases.db",
        "backup": true,
        "backup_interval": 3600,
        "optimization": true,
        "compression": false,
        "sync_interval": 60
      }
    }
  }
}
```

**Recommendations:**
- Use file-based database for small deployments (<1000 leases)
- Enable optimization for better query performance
- Disable compression if performance is critical
- Adjust sync interval based on durability requirements

### Connection Pooling

```json
{
  "dhcp": {
    "performance": {
      "connection_pooling": {
        "enabled": true,
        "max_connections": 1000,
        "idle_timeout": 300,
        "connection_timeout": 30,
        "keep_alive": true
      }
    }
  }
}
```

**Recommendations:**
- Enable for high-traffic environments
- Set max_connections based on expected load
- Adjust idle_timeout based on traffic patterns

### Caching Configuration

```json
{
  "dhcp": {
    "performance": {
      "caching": {
        "enabled": true,
        "cache_size": "200MB",
        "ttl": 3600,
        "cache_leases": true,
        "cache_options": true,
        "cache_reservations": true
      }
    }
  }
}
```

**Recommendations:**
- Enable caching for read-heavy workloads
- Set cache_size based on available memory
- Adjust TTL based on data freshness requirements

## System-Level Tuning

### Network Tuning

```bash
# Increase UDP buffer sizes
sudo sysctl -w net.core.rmem_max=16777216
sudo sysctl -w net.core.wmem_max=16777216
sudo sysctl -w net.core.rmem_default=65536
sudo sysctl -w net.core.wmem_default=65536

# Make changes persistent
echo "net.core.rmem_max=16777216" | sudo tee -a /etc/sysctl.conf
echo "net.core.wmem_max=16777216" | sudo tee -a /etc/sysctl.conf
```

### File Descriptor Limits

```bash
# Increase file descriptor limits
sudo ulimit -n 65536

# Make persistent
echo "* soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65536" | sudo tee -a /etc/security/limits.conf
```

### CPU Affinity

```bash
# Pin process to specific CPUs
sudo taskset -cp 0,1 $(pidof simple-dhcpd)
```

## Lease Management Tuning

### Lease Time Configuration

```json
{
  "dhcp": {
    "subnets": [
      {
        "name": "main-subnet",
        "lease_time": 3600,
        "max_lease_time": 7200,
        "renewal_time": 1800,
        "rebinding_time": 3150
      }
    ]
  }
}
```

**Recommendations:**
- Shorter lease times for dynamic environments
- Longer lease times for stable networks
- Balance between network flexibility and performance

### Subnet Configuration

```json
{
  "dhcp": {
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0/24",
        "range_start": "192.168.1.100",
        "range_end": "192.168.1.200",
        "exclusions": [
          {
            "start": "192.168.1.1",
            "end": "192.168.1.10"
          }
        ]
      }
    ]
  }
}
```

**Recommendations:**
- Use appropriate IP ranges for your network size
- Exclude infrastructure IPs to avoid conflicts
- Monitor lease utilization

## Logging Performance

### Log Level Configuration

```json
{
  "dhcp": {
    "logging": {
      "enable": true,
      "level": "info",
      "format": "json",
      "rotation": true,
      "max_size": "100MB",
      "max_files": 10,
      "compression": true
    }
  }
}
```

**Recommendations:**
- Use "info" level for production
- Use "debug" only for troubleshooting
- Enable log rotation to manage disk space
- Use JSON format for better parsing

### Log File Optimization

```bash
# Use separate log files for different components
{
  "dhcp": {
    "logging": {
      "access_log": "/var/log/simple-dhcpd/access.log",
      "error_log": "/var/log/simple-dhcpd/error.log",
      "lease_log": "/var/log/simple-dhcpd/leases.log"
    }
  }
}
```

## Monitoring and Profiling

### Performance Monitoring

```bash
# Monitor RPS
watch -n 1 'sudo simple-dhcpd --stats | grep rps'

# Monitor memory usage
watch -n 1 'ps aux | grep simple-dhcpd'

# Monitor network traffic
sudo tcpdump -i eth0 port 67 -c 100
```

### Profiling

```bash
# CPU profiling
sudo perf record -p $(pidof simple-dhcpd)
sudo perf report

# Memory profiling
valgrind --tool=massif simple-dhcpd -f
```

## Best Practices

### General Recommendations

1. **Monitor Performance**: Regularly monitor key metrics
2. **Tune Incrementally**: Make one change at a time and measure impact
3. **Test Changes**: Test configuration changes in staging first
4. **Document Changes**: Keep track of tuning changes and their effects
5. **Review Regularly**: Periodically review and optimize configuration

### Scaling Recommendations

- **Small Deployments** (<100 clients): Default configuration is sufficient
- **Medium Deployments** (100-1000 clients): Enable caching and connection pooling
- **Large Deployments** (>1000 clients): Full optimization with system-level tuning

### Troubleshooting Performance Issues

1. **High CPU Usage**: Check for high request rates, enable rate limiting
2. **High Memory Usage**: Review cache size, check for memory leaks
3. **Slow Response Times**: Check network configuration, review database performance
4. **Network Issues**: Verify network tuning, check for congestion

## Performance Testing

### Benchmarking

```bash
# Run performance tests
./tests/performance/benchmark.sh

# Load testing
./tests/performance/load_test.sh
```

### Stress Testing

```bash
# Stress test with high load
./tests/performance/stress_test.sh --clients 1000 --duration 300
```

## Additional Resources

- [Configuration Reference](../configuration/README.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
- [Monitoring Setup](../deployment/monitoring.md)
