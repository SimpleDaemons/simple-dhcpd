# Quick Start Guide

Get Simple DHCP Daemon up and running in minutes with this quick start guide.

## Prerequisites

- Simple DHCP Daemon installed (see [Installation Guide](installation.md))
- Root or sudo access
- Basic understanding of DHCP concepts

## Step 1: Create Basic Configuration

Create a minimal configuration file:

```bash
sudo mkdir -p /etc/simple-dhcpd
sudo tee /etc/simple-dhcpd/simple-dhcpd.conf > /dev/null << 'EOF'
{
  "dhcp": {
    "listen": ["0.0.0.0:67"],
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0/24",
        "range": "192.168.1.100-192.168.1.200",
        "gateway": "192.168.1.1",
        "dns_servers": ["8.8.8.8", "8.8.4.4"],
        "domain_name": "local",
        "lease_time": 86400
      }
    ],
    "logging": {
      "enable": true,
      "level": "info"
    }
  }
}
EOF
```

## Step 2: Start the Server

### Option A: Run in Foreground (for testing)

```bash
sudo simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf -v
```

### Option B: Run as Daemon

```bash
sudo simple-dhcpd -d -c /etc/simple-dhcpd/simple-dhcpd.conf
```

### Option C: Using systemd (if installed)

```bash
sudo systemctl start simple-dhcpd
sudo systemctl enable simple-dhcpd
```

## Step 3: Test DHCP Service

### Test from Another Machine

1. Connect a client machine to the same network
2. Configure the client to use DHCP
3. Check if the client receives an IP address in the range 192.168.1.100-192.168.1.200

### Test from Command Line

```bash
# Release current IP
sudo dhclient -r eth0

# Request new IP
sudo dhclient -v eth0

# Check assigned IP
ip addr show eth0
```

## Step 4: Verify Operation

### Check Server Logs

```bash
# If running in foreground, logs appear in terminal
# If running as daemon, check system logs
sudo journalctl -u simple-dhcpd -f

# Or check log file
tail -f /var/log/simple-dhcpd.log
```

### Check Lease Database

```bash
# View active leases
sudo simple-dhcpd --show-leases

# View statistics
sudo simple-dhcpd --stats
```

## Step 5: Configure Client

### Linux Client

```bash
# Configure interface for DHCP
sudo ip link set eth0 up
sudo dhclient eth0

# Check configuration
ip addr show eth0
ip route show
cat /etc/resolv.conf
```

### Windows Client

1. Open Network and Sharing Center
2. Click "Change adapter settings"
3. Right-click your network adapter
4. Select "Properties"
5. Select "Internet Protocol Version 4 (TCP/IPv4)"
6. Click "Properties"
7. Select "Obtain an IP address automatically"
8. Select "Obtain DNS server address automatically"
9. Click "OK"

### macOS Client

```bash
# Configure interface for DHCP
sudo ifconfig en0 up
sudo dhclient en0

# Check configuration
ifconfig en0
networksetup -getdnsservers Wi-Fi
```

## Common Issues and Solutions

### Issue: Client Not Getting IP Address

**Check server logs:**
```bash
sudo journalctl -u simple-dhcpd -f
```

**Verify configuration:**
```bash
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
```

**Check network connectivity:**
```bash
# Ping the server
ping 192.168.1.1

# Check if port 67 is listening
sudo netstat -tulpn | grep :67
```

### Issue: Permission Denied

**Fix file permissions:**
```bash
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd/
sudo chown -R dhcpd:dhcpd /var/lib/simple-dhcpd/
sudo chown -R dhcpd:dhcpd /var/log/simple-dhcpd/
```

### Issue: Port Already in Use

**Check what's using port 67:**
```bash
sudo lsof -i :67
sudo netstat -tulpn | grep :67
```

**Stop conflicting service:**
```bash
# Stop ISC DHCP server
sudo systemctl stop isc-dhcp-server
sudo systemctl disable isc-dhcp-server

# Stop dnsmasq
sudo systemctl stop dnsmasq
sudo systemctl disable dnsmasq
```

## Next Steps

Now that you have a basic DHCP server running:

1. **Configure Additional Subnets**: Add more subnets for different VLANs
2. **Set Up Static Leases**: Reserve IP addresses for specific devices
3. **Enable Security Features**: Configure MAC filtering and rate limiting
4. **Set Up Monitoring**: Configure logging and metrics collection
5. **Production Deployment**: Follow the [Production Deployment Guide](../deployment/production.md)

## Configuration Examples

### Multiple Subnets

```json
{
  "dhcp": {
    "listen": ["0.0.0.0:67"],
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0/24",
        "range": "192.168.1.100-192.168.1.200",
        "gateway": "192.168.1.1",
        "dns_servers": ["8.8.8.8", "8.8.4.4"],
        "domain_name": "local",
        "lease_time": 86400
      },
      {
        "name": "guest-subnet",
        "network": "192.168.2.0/24",
        "range": "192.168.2.100-192.168.2.200",
        "gateway": "192.168.2.1",
        "dns_servers": ["8.8.8.8", "8.8.4.4"],
        "domain_name": "guest.local",
        "lease_time": 3600
      }
    ]
  }
}
```

### Static Leases

```json
{
  "dhcp": {
    "subnets": [
      {
        "name": "main-subnet",
        "network": "192.168.1.0/24",
        "range": "192.168.1.100-192.168.1.200",
        "reservations": [
          {
            "mac_address": "00:11:22:33:44:55",
            "ip_address": "192.168.1.10",
            "hostname": "server1"
          }
        ]
      }
    ]
  }
}
```

## Getting Help

- **Documentation**: [Full Documentation](../README.md)
- **Configuration**: [Configuration Guide](../configuration/README.md)
- **Troubleshooting**: [Troubleshooting Guide](../troubleshooting/README.md)
- **Community**: [GitHub Discussions](https://github.com/simpledaemons/simple-dhcpd/discussions)
- **Issues**: [GitHub Issues](https://github.com/simpledaemons/simple-dhcpd/issues)
