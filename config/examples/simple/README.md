# Simple Configurations

This directory contains simple, easy-to-understand configuration examples for basic networking scenarios.

## Files

### home-network.yaml
Perfect for basic home networks with minimal complexity:
- Single subnet (192.168.1.0/24)
- Basic DHCP options
- No security features
- Simple logging

**Use case**: Home users, small apartments, basic setups

### small-office.yaml
Suitable for small businesses with basic networking needs:
- Two subnets (office + guest)
- Static IP reservations
- Basic security enabled
- IP exclusions

**Use case**: Small offices, home offices, basic business networks

## Quick Start

1. Choose the configuration that matches your needs
2. Copy it to your preferred location:
   ```bash
   cp home-network.yaml /etc/simple-dhcpd/config.yaml
   ```
3. Modify the IP addresses to match your network
4. Start the daemon:
   ```bash
   ./simple-dhcpd -c /etc/simple-dhcpd/config.yaml
   ```

## Customization

### Changing IP Ranges
Edit the `network`, `range_start`, and `range_end` values to match your network:

```yaml
subnets:
  - name: "home"
    network: "10.0.0.0"        # Your network
    range_start: "10.0.0.100"  # Start of DHCP range
    range_end: "10.0.0.200"    # End of DHCP range
    gateway: "10.0.0.1"        # Your router/gateway
```

### Adding Static Reservations
Add devices that should always get the same IP:

```yaml
reservations:
  - mac_address: "AA:BB:CC:DD:EE:FF"
    ip_address: "192.168.1.50"
    hostname: "my-device"
    is_static: true
```

### Excluding IP Ranges
Reserve IP ranges that shouldn't be assigned by DHCP:

```yaml
exclusions:
  - start: "192.168.1.1"
    end: "192.168.1.10"
```

## Next Steps

If you need more advanced features, check out:
- `../advanced/` - Advanced networking features
- `../production/` - Enterprise-grade configurations
- `../security/` - Security-focused setups