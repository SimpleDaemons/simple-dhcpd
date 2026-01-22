# Deployment Guide

This guide covers deployment procedures for Simple DHCP Daemon in production environments.

## Pre-Deployment Checklist

### System Requirements

- [ ] Verify system meets minimum requirements
- [ ] Check network connectivity
- [ ] Verify firewall configuration
- [ ] Prepare configuration files
- [ ] Set up logging directories
- [ ] Configure service account

### Prerequisites

- [ ] Operating system installed and updated
- [ ] Network interfaces configured
- [ ] DNS resolution working
- [ ] Time synchronization configured
- [ ] Backup procedures in place

## Installation

### From Package

```bash
# Ubuntu/Debian
sudo dpkg -i simple-dhcpd_*.deb
sudo apt-get install -f

# CentOS/RHEL
sudo rpm -ivh simple-dhcpd-*.rpm

# macOS
sudo installer -pkg simple-dhcpd-*.pkg -target /

# Windows
# Run MSI installer
```

### From Source

```bash
# Build and install
./scripts/build-linux.sh
sudo make install
```

## Configuration

### Initial Configuration

1. **Create Configuration Directory**

```bash
sudo mkdir -p /etc/simple-dhcpd
sudo chown dhcpd:dhcpd /etc/simple-dhcpd
```

2. **Create Configuration File**

```bash
sudo cp config/examples/simple.json /etc/simple-dhcpd/simple-dhcpd.conf
sudo chown dhcpd:dhcpd /etc/simple-dhcpd/simple-dhcpd.conf
sudo chmod 640 /etc/simple-dhcpd/simple-dhcpd.conf
```

3. **Edit Configuration**

```bash
sudo nano /etc/simple-dhcpd/simple-dhcpd.conf
```

4. **Validate Configuration**

```bash
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
```

### Directory Setup

```bash
# Create data directory
sudo mkdir -p /var/lib/simple-dhcpd
sudo chown dhcpd:dhcpd /var/lib/simple-dhcpd
sudo chmod 750 /var/lib/simple-dhcpd

# Create log directory
sudo mkdir -p /var/log/simple-dhcpd
sudo chown dhcpd:dhcpd /var/log/simple-dhcpd
sudo chmod 750 /var/log/simple-dhcpd
```

## Service Setup

### systemd (Linux)

1. **Install Service File**

```bash
sudo cp deployment/systemd/simple-dhcpd.service /etc/systemd/system/
sudo systemctl daemon-reload
```

2. **Enable Service**

```bash
sudo systemctl enable simple-dhcpd
```

3. **Start Service**

```bash
sudo systemctl start simple-dhcpd
```

4. **Check Status**

```bash
sudo systemctl status simple-dhcpd
```

### launchd (macOS)

1. **Install Service File**

```bash
sudo cp deployment/launchd/com.simpledaemons.simple-dhcpd.plist /Library/LaunchDaemons/
sudo chown root:wheel /Library/LaunchDaemons/com.simpledaemons.simple-dhcpd.plist
```

2. **Load Service**

```bash
sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-dhcpd.plist
```

3. **Start Service**

```bash
sudo launchctl start com.simpledaemons.simple-dhcpd
```

### Windows Service

1. **Install Service**

```bash
simple-dhcpd --install-service
```

2. **Start Service**

```bash
net start simple-dhcpd
```

## Network Configuration

### Firewall Rules

```bash
# Allow DHCP traffic
sudo ufw allow 67/udp
sudo ufw allow 68/udp

# Or with iptables
sudo iptables -A INPUT -p udp --dport 67 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 68 -j ACCEPT
```

### Network Interface

```bash
# Verify interface is up
ip link show eth0

# Check IP configuration
ip addr show eth0

# Verify routing
ip route show
```

## Validation

### Service Validation

```bash
# Check service status
sudo systemctl status simple-dhcpd

# Check process
ps aux | grep simple-dhcpd

# Check listening ports
sudo netstat -tulpn | grep :67
```

### Configuration Validation

```bash
# Test configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --validate

# Show configuration
simple-dhcpd --show-config
```

### Functional Testing

```bash
# Test DHCP server
dhclient -v eth0

# Check leases
simple-dhcpd --show-leases

# Monitor logs
tail -f /var/log/simple-dhcpd.log
```

## Post-Deployment

### Monitoring Setup

1. **Configure Monitoring**

See [Monitoring Setup](monitoring.md) for details.

2. **Set Up Alerts**

Configure alerting for:
- Service status
- Error rates
- Performance metrics
- Security events

### Backup Configuration

1. **Configure Backups**

See [Backup Procedures](backup-procedures.md) for details.

2. **Test Restores**

Regularly test backup restoration procedures.

### Documentation

- Document deployment configuration
- Record network topology
- Maintain change log
- Update runbooks

## Troubleshooting

### Common Issues

1. **Service Won't Start**
   - Check configuration syntax
   - Verify file permissions
   - Check port availability

2. **Clients Not Getting IPs**
   - Verify network connectivity
   - Check subnet configuration
   - Review firewall rules

3. **Performance Issues**
   - Review performance tuning guide
   - Check system resources
   - Monitor network traffic

See [Troubleshooting Guide](../troubleshooting/README.md) for more details.

## Rollback Procedures

### Service Rollback

```bash
# Stop service
sudo systemctl stop simple-dhcpd

# Restore previous configuration
sudo cp /etc/simple-dhcpd/simple-dhcpd.conf.backup /etc/simple-dhcpd/simple-dhcpd.conf

# Restart service
sudo systemctl start simple-dhcpd
```

### Configuration Rollback

```bash
# Restore configuration
sudo cp /etc/simple-dhcpd/simple-dhcpd.conf.backup /etc/simple-dhcpd/simple-dhcpd.conf

# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Reload service
sudo systemctl reload simple-dhcpd
```

## Additional Resources

- [Configuration Reference](../configuration/README.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
- [Monitoring Setup](monitoring.md)
- [Backup Procedures](backup-procedures.md)
