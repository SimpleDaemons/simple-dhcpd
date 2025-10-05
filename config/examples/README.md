# Simple DHCP Daemon Configuration Examples

This directory contains example configuration files for the Simple DHCP Daemon in multiple formats.

## Configuration Formats

The Simple DHCP Daemon supports three configuration formats:

- **JSON** (`.json`) - JavaScript Object Notation
- **YAML** (`.yaml` or `.yml`) - YAML Ain't Markup Language  
- **INI** (`.ini`) - Windows-style configuration files

## Example Files

### Comprehensive Examples

- `simple-dhcpd.json` - Full-featured JSON configuration
- `simple-dhcpd.yaml` - Full-featured YAML configuration  
- `simple-dhcpd.ini` - Full-featured INI configuration

### Minimal Examples

- `minimal.json` - Minimal JSON configuration
- `minimal.yaml` - Minimal YAML configuration
- `minimal.ini` - Minimal INI configuration

## Configuration Structure

### Server Configuration

```yaml
server:
  listen_addresses: ["0.0.0.0:67", "::67"]  # Server listening addresses
  config_file: "/etc/simple-dhcpd/config.yaml"  # Config file path
  lease_file: "/var/lib/simple-dhcpd/dhcpd.leases"  # Lease database file
  log_file: "/var/log/simple-dhcpd/simple-dhcpd.log"  # Log file path
  enable_logging: true  # Enable logging
  enable_security: true  # Enable security features
  max_leases: 10000  # Maximum number of leases
```

### Subnet Configuration

```yaml
subnets:
  - name: "main-lan"  # Subnet name
    network: "192.168.1.0"  # Network address
    prefix_length: 24  # Subnet mask (CIDR notation)
    range_start: "192.168.1.100"  # Start of IP range
    range_end: "192.168.1.200"  # End of IP range
    gateway: "192.168.1.1"  # Default gateway
    dns_servers:  # DNS servers
      - "192.168.1.1"
      - "8.8.8.8"
    domain_name: "home.local"  # Domain name
    lease_time: 86400  # Default lease time (seconds)
    max_lease_time: 172800  # Maximum lease time (seconds)
```

### DHCP Options

```yaml
options:
  - code: 6  # DHCP option code
    name: "DNS_SERVERS"  # Option name
    data: "192.168.1.1,8.8.8.8"  # Option data
```

### Static Reservations

```yaml
reservations:
  - mac_address: "00:11:22:33:44:55"  # Client MAC address
    ip_address: "192.168.1.10"  # Reserved IP address
    hostname: "server-01"  # Hostname
    is_static: true  # Static reservation
```

### IP Exclusions

```yaml
exclusions:
  - start: "192.168.1.1"  # Start of excluded range
    end: "192.168.1.10"  # End of excluded range
```

## Usage

### Command Line

```bash
# Use JSON configuration
./simple-dhcpd -c /path/to/simple-dhcpd.json

# Use YAML configuration  
./simple-dhcpd -c /path/to/simple-dhcpd.yaml

# Use INI configuration
./simple-dhcpd -c /path/to/simple-dhcpd.ini
```

### Environment Variables

```bash
# Set configuration file via environment
export SIMPLE_DHCPD_CONFIG=/etc/simple-dhcpd/simple-dhcpd.yaml
./simple-dhcpd
```

## Configuration Validation

The daemon will validate your configuration on startup and report any errors:

```bash
./simple-dhcpd -c /path/to/config.yaml --validate
```

## Common DHCP Options

| Code | Name | Description |
|------|------|-------------|
| 1 | SUBNET_MASK | Subnet mask |
| 3 | ROUTER | Default gateway |
| 6 | DNS_SERVERS | DNS servers |
| 15 | DOMAIN_NAME | Domain name |
| 28 | BROADCAST_ADDRESS | Broadcast address |
| 42 | NTP_SERVERS | NTP servers |
| 51 | IP_ADDRESS_LEASE_TIME | Lease time |
| 54 | SERVER_IDENTIFIER | DHCP server identifier |

## Security Features

### MAC Address Filtering

```yaml
security:
  mac_filtering: true
  allowed_mac_addresses:
    - "00:11:22:33:44:55"
    - "aa:bb:cc:dd:ee:*"  # Wildcard pattern
  blocked_mac_addresses:
    - "ff:ff:ff:ff:ff:ff"
```

### Rate Limiting

```yaml
security:
  rate_limiting:
    enabled: true
    max_requests_per_minute: 60
    max_requests_per_hour: 1000
```

### DHCP Snooping

```yaml
security:
  dhcp_snooping: true
```

## Logging Configuration

```yaml
logging:
  level: "INFO"  # DEBUG, INFO, WARN, ERROR
  file_rotation:
    enabled: true
    max_size_mb: 100
    max_files: 5
  console_output: true
  syslog:
    enabled: true
    facility: "daemon"
```

## Troubleshooting

### Common Issues

1. **Invalid IP addresses**: Ensure all IP addresses are valid and properly formatted
2. **Invalid MAC addresses**: Use format `XX:XX:XX:XX:XX:XX` (uppercase)
3. **Invalid lease times**: Lease times must be in seconds (positive integers)
4. **Invalid subnet ranges**: Ensure `range_start` < `range_end` and both are within the subnet

### Debug Mode

Run with debug logging to troubleshoot issues:

```bash
./simple-dhcpd -c /path/to/config.yaml --verbose --log-level DEBUG
```

### Configuration Test

Test your configuration without starting the daemon:

```bash
./simple-dhcpd -c /path/to/config.yaml --test-config
```

## Examples by Use Case

### Home Network

Use `minimal.yaml` as a starting point for a simple home network.

### Small Office

Use `simple-dhcpd.yaml` and modify the subnets for your office network.

### Enterprise

Use `simple-dhcpd.yaml` and add additional subnets, security features, and logging configuration.

### IoT Network

Create a separate subnet for IoT devices with shorter lease times and restricted access.

## Support

For more information, see the main project documentation or create an issue on GitHub.
