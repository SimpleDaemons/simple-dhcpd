# Simple DHCP Daemon Configuration Examples

This directory contains comprehensive configuration examples for the Simple DHCP Daemon, organized by use case and complexity level.

## Directory Structure

```
config/examples/
├── README.md                    # This file
├── validate_config.sh           # Configuration validation script
├── convert_config.py            # Format conversion utility
├── Makefile                     # Management commands
├── simple-dhcpd.json           # Full-featured JSON configuration
├── simple-dhcpd.yaml           # Full-featured YAML configuration
├── simple-dhcpd.ini            # Full-featured INI configuration
├── minimal.json                # Minimal JSON configuration
├── minimal.yaml                # Minimal YAML configuration
├── minimal.ini                 # Minimal INI configuration
├── simple/                     # Simple configurations
│   ├── README.md
│   ├── home-network.yaml
│   └── small-office.yaml
├── advanced/                   # Advanced configurations
│   ├── README.md
│   ├── multi-vlan.yaml
│   └── load-balanced.yaml
├── production/                 # Production configurations
│   ├── README.md
│   ├── enterprise.yaml
│   └── datacenter.yaml
└── security/                   # Security configurations
    ├── README.md
    ├── high-security.yaml
    └── zero-trust.yaml
```

## Quick Start

### 1. Choose Your Configuration

**For beginners:**
- Start with `simple/` directory
- Use `minimal.yaml` for basic setups

**For advanced users:**
- Use `advanced/` for complex networks
- Use `production/` for enterprise environments
- Use `security/` for high-security requirements

### 2. Select Your Format

The daemon supports three configuration formats:
- **JSON** (`.json`) - JavaScript Object Notation
- **YAML** (`.yaml` or `.yml`) - YAML Ain't Markup Language
- **INI** (`.ini`) - Windows-style configuration files

### 3. Validate Your Configuration

```bash
# Validate a configuration file
./validate_config.sh your-config.yaml

# Validate all configurations
make validate
```

### 4. Convert Between Formats

```bash
# Convert JSON to YAML
python3 convert_config.py config.json config.yaml

# Convert using Makefile
make convert INPUT=config.json OUTPUT=config.yaml
```

## Configuration Categories

### Simple Configurations (`simple/`)
Perfect for basic networking scenarios:
- **home-network.yaml** - Basic home network setup
- **small-office.yaml** - Small business configuration

**Use cases**: Home users, small offices, basic setups

### Advanced Configurations (`advanced/`)
Complex networking scenarios with advanced features:
- **multi-vlan.yaml** - Multi-VLAN enterprise setup
- **load-balanced.yaml** - High-availability configuration

**Use cases**: Enterprise networks, data centers, complex office environments

### Production Configurations (`production/`)
Enterprise-grade configurations for mission-critical environments:
- **enterprise.yaml** - Comprehensive enterprise setup
- **datacenter.yaml** - High-performance data center configuration

**Use cases**: Large enterprises, mission-critical applications, high-availability environments

### Security Configurations (`security/`)
High-security configurations with comprehensive protection:
- **high-security.yaml** - Maximum security setup
- **zero-trust.yaml** - Zero trust security configuration

**Use cases**: High-security environments, government networks, financial institutions

## Configuration Features

### Supported Features
- **Multi-Format Support**: JSON, YAML, and INI formats
- **Multiple Subnets**: Support for complex network topologies
- **Static Reservations**: Guaranteed IP addresses for critical devices
- **IP Exclusions**: Reserved ranges for infrastructure
- **DHCP Options**: Global and subnet-specific options
- **Security Features**: MAC filtering, rate limiting, DHCP snooping
- **Advanced Logging**: Comprehensive logging and monitoring
- **High Availability**: Load balancing and failover support

### Security Features
- **DHCP Snooping**: Protection against rogue DHCP servers
- **MAC Filtering**: Whitelist/blacklist based on MAC addresses
- **Rate Limiting**: Protection against DHCP flooding attacks
- **Intrusion Detection**: Real-time threat detection
- **Authentication**: Device authentication and registration
- **Audit Logging**: Comprehensive security event logging

## Usage Examples

### Basic Usage
```bash
# Use a specific configuration file
./simple-dhcpd -c /path/to/config.yaml

# Validate configuration without starting
./simple-dhcpd -c /path/to/config.yaml --validate

# Run with debug logging
./simple-dhcpd -c /path/to/config.yaml --verbose --log-level DEBUG
```

### Configuration Management
```bash
# Validate all configurations
make validate

# Convert between formats
make convert INPUT=config.json OUTPUT=config.yaml

# Install examples to system
make install

# Show configuration information
make info
```

### Customization
1. **Copy an example**: `cp simple/home-network.yaml my-config.yaml`
2. **Edit the configuration**: Modify IP ranges, subnets, and options
3. **Validate**: `./validate_config.sh my-config.yaml`
4. **Deploy**: `./simple-dhcpd -c my-config.yaml`

## Best Practices

### Network Planning
- **Document Everything**: Keep detailed records of all configurations
- **Plan for Growth**: Design for future expansion
- **Security First**: Implement appropriate security controls
- **Regular Audits**: Review and update configurations regularly

### Configuration Management
- **Version Control**: Use version control for configurations
- **Backup**: Regular backup of configuration files
- **Testing**: Test configurations in non-production environments
- **Documentation**: Maintain detailed documentation

### Security
- **Principle of Least Privilege**: Minimum necessary access
- **Network Segmentation**: Isolate different types of traffic
- **Monitoring**: Continuous security monitoring
- **Incident Response**: Defined procedures for security incidents

## Troubleshooting

### Common Issues
- **Invalid IP addresses**: Ensure all IP addresses are valid and properly formatted
- **Invalid MAC addresses**: Use format `XX:XX:XX:XX:XX:XX` (uppercase)
- **Invalid lease times**: Lease times must be in seconds (positive integers)
- **Invalid subnet ranges**: Ensure `range_start` < `range_end` and both are within the subnet

### Debug Commands
```bash
# Check configuration syntax
./validate_config.sh config.yaml

# Run with debug logging
./simple-dhcpd -c config.yaml --verbose --log-level DEBUG

# Monitor DHCP activity
tail -f /var/log/simple-dhcpd/simple-dhcpd.log
```

### Getting Help
- Check the specific README files in each subdirectory
- Review the main project documentation
- Create an issue on GitHub for bugs or feature requests
- Check the troubleshooting guide in the main documentation

## Contributing

### Adding New Examples
1. Create a new configuration file in the appropriate directory
2. Add a description to the relevant README.md
3. Test the configuration thoroughly
4. Submit a pull request

### Improving Existing Examples
1. Test the existing configuration
2. Make improvements or fixes
3. Update documentation as needed
4. Submit a pull request

## License

These configuration examples are provided under the same license as the Simple DHCP Daemon project.

## Support

For more information, see:
- Main project documentation
- GitHub repository
- Issue tracker for bugs and feature requests
- Community forums for discussions