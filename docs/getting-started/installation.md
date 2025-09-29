# Installation Guide

This guide covers installing Simple DHCP Daemon on various platforms.

## System Requirements

### Minimum Requirements
- **CPU**: 1 core, 1 GHz
- **RAM**: 512 MB
- **Disk**: 100 MB free space
- **OS**: Linux (kernel 3.10+), macOS 10.15+, Windows 10/11

### Recommended Requirements
- **CPU**: 2+ cores, 2+ GHz
- **RAM**: 2+ GB
- **Disk**: 1+ GB free space
- **Network**: Gigabit Ethernet

## Installation Methods

### From Source

#### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake git libssl-dev libjsoncpp-dev
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git openssl-devel jsoncpp-devel
```

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake openssl jsoncpp
```

**Windows:**
- Install Visual Studio 2019 or later
- Install CMake
- Install vcpkg for dependencies

#### Build and Install

```bash
# Clone repository
git clone https://github.com/SimpleDaemons/simple-dhcpd.git
cd simple-dhcpd

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Run tests
make test

# Install
sudo make install
```

### Package Installation

#### Ubuntu/Debian (Coming Soon)

```bash
# Add repository
wget -qO - https://packages.simpledaemons.org/gpg.key | sudo apt-key add -
echo "deb https://packages.simpledaemons.org/ubuntu focal main" | sudo tee /etc/apt/sources.list.d/simpledaemons.list

# Install
sudo apt update
sudo apt install simple-dhcpd
```

#### CentOS/RHEL (Coming Soon)

```bash
# Add repository
sudo yum-config-manager --add-repo https://packages.simpledaemons.org/rhel/simpledaemons.repo

# Install
sudo yum install simple-dhcpd
```

#### macOS (Coming Soon)

```bash
# Add tap
brew tap simpledaemons/simple-dhcpd

# Install
brew install simple-dhcpd
```

### Docker Installation

#### Using Docker Hub

```bash
# Pull image
docker pull simpledaemons/simple-dhcpd:latest

# Run container
docker run -d \
  --name simple-dhcpd \
  --net host \
  -v /etc/simple-dhcpd:/etc/simple-dhcpd \
  simpledaemons/simple-dhcpd:latest
```

#### Using Docker Compose

```yaml
version: '3.8'
services:
  simple-dhcpd:
    image: simpledaemons/simple-dhcpd:latest
    network_mode: host
    volumes:
      - ./config:/etc/simple-dhcpd
      - ./logs:/var/log/simple-dhcpd
    restart: unless-stopped
    environment:
      - CONFIG_FILE=/etc/simple-dhcpd/simple-dhcpd.conf
```

## Post-Installation

### Create Configuration Directory

```bash
sudo mkdir -p /etc/simple-dhcpd
sudo chown dhcpd:dhcpd /etc/simple-dhcpd
```

### Create Log Directory

```bash
sudo mkdir -p /var/log/simple-dhcpd
sudo chown dhcpd:dhcpd /var/log/simple-dhcpd
```

### Create Lease Database Directory

```bash
sudo mkdir -p /var/lib/simple-dhcpd
sudo chown dhcpd:dhcpd /var/lib/simple-dhcpd
```

### Create System User

```bash
# Create dhcpd user
sudo useradd -r -s /bin/false -d /var/lib/simple-dhcpd dhcpd
```

## Verification

### Check Installation

```bash
# Check version
simple-dhcpd --version

# Check help
simple-dhcpd --help

# Test configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
```

### Basic Test

```bash
# Start server in foreground
sudo simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf -v

# In another terminal, test DHCP
sudo dhclient -v eth0
```

## Uninstallation

### Remove Package

**Ubuntu/Debian:**
```bash
sudo apt remove simple-dhcpd
sudo apt autoremove
```

**CentOS/RHEL:**
```bash
sudo yum remove simple-dhcpd
```

**macOS:**
```bash
brew uninstall simple-dhcpd
```

### Remove from Source

```bash
# From build directory
sudo make uninstall

# Remove configuration files
sudo rm -rf /etc/simple-dhcpd
sudo rm -rf /var/lib/simple-dhcpd
sudo rm -rf /var/log/simple-dhcpd

# Remove user
sudo userdel dhcpd
```

## Troubleshooting

### Common Issues

**Permission Denied:**
```bash
# Check file permissions
ls -la /etc/simple-dhcpd/
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd/
```

**Port Already in Use:**
```bash
# Check what's using port 67
sudo netstat -tulpn | grep :67
sudo lsof -i :67
```

**Configuration Errors:**
```bash
# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config
```

### Getting Help

- Check the [Troubleshooting Guide](../troubleshooting/README.md)
- View logs: `sudo journalctl -u simple-dhcpd -f`
- Open an issue on [GitHub](https://github.com/simpledaemons/simple-dhcpd/issues)

## Next Steps

After installation, proceed to:
- [Quick Start Guide](quick-start.md)
- [Configuration Guide](../configuration/README.md)
- [First Steps](first-steps.md)
