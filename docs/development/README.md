# Developer Documentation

Welcome to the Simple DHCP Daemon development guide. This document will help you get started with development, building, and contributing to the project.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Build System](#build-system)
- [Building Different Versions](#building-different-versions)
- [Testing](#testing)
- [Development Workflow](#development-workflow)
- [Code Structure](#code-structure)
- [Contributing](#contributing)

---

## Prerequisites

### Required Software

- **C++ Compiler:** GCC 7+ or Clang 8+ (C++17 support required)
- **CMake:** 3.16 or higher
- **Make:** GNU Make or equivalent
- **Git:** For version control

### Platform-Specific Requirements

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git libssl-dev libjsoncpp-dev pkg-config
```

#### Linux (CentOS/RHEL)
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git openssl-devel jsoncpp-devel pkgconfig
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake openssl jsoncpp pkg-config
```

#### Windows
- Install Visual Studio 2019 or later (with C++ workload)
- Install CMake
- Install vcpkg for dependencies:
  ```powershell
  vcpkg install openssl jsoncpp
  ```

### Optional Tools

- **Google Test:** For running tests (automatically downloaded if not found)
- **Doxygen:** For generating API documentation
- **Valgrind/AddressSanitizer:** For memory debugging
- **clang-format:** For code formatting

---

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/SimpleDaemons/simple-dhcpd.git
cd simple-dhcpd
```

### 2. Create Build Directory

```bash
mkdir build
cd build
```

### 3. Configure Build

```bash
# Production version (default)
cmake ..

# Or specify version explicitly
cmake -DBUILD_VERSION=production ..
```

### 4. Build

```bash
# Build all targets
make

# Or use CMake
cmake --build .

# Build with multiple jobs (faster)
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
```

### 5. Run

```bash
# Run the daemon
./simple-dhcpd --help

# Run with configuration
./simple-dhcpd -c /path/to/config.conf
```

---

## Build System

### CMake Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_VERSION` | `production` | Version to build: `production`, `enterprise`, `datacenter` |
| `CMAKE_BUILD_TYPE` | `Release` | Build type: `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel` |
| `ENABLE_TESTS` | `ON` | Enable test suite |
| `ENABLE_PACKAGING` | `ON` | Enable package generation |
| `ENABLE_SSL` | `ON` | Enable SSL/TLS support |
| `ENABLE_JSON` | `ON` | Enable JSON configuration support |
| `ENABLE_STATIC_LINKING` | `OFF` | Enable static linking |

### Build Types

#### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
- Includes debug symbols
- No optimizations
- Assertions enabled
- Best for development

#### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
- Full optimizations
- No debug symbols
- Best for production

#### RelWithDebInfo Build
```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```
- Optimizations enabled
- Debug symbols included
- Good for production debugging

---

## Building Different Versions

### Production Version

The Production version includes core DHCP functionality and basic security features.

```bash
# Configure for Production
cmake -DBUILD_VERSION=production ..

# Build
make

# Output: simple-dhcpd (Production binary)
```

**Features:**
- Complete DHCP protocol (DORA process)
- Basic lease management
- Basic security (Option 82, rate limiting)
- Multi-format configuration (JSON, YAML, INI)

### Enterprise Version

The Enterprise version includes all Production features plus high availability, management interfaces, and integrations.

```bash
# Configure for Enterprise
cmake -DBUILD_VERSION=enterprise ..

# Build
make

# Output: simple-dhcpd (Enterprise binary)
```

**Additional Features:**
- High availability (DHCP failover)
- Web management interface
- REST API
- DNS/LDAP/SNMP integrations
- Advanced security

**Note:** Enterprise features are in development. The build will succeed but some features may not be fully implemented.

### Datacenter Version

The Datacenter version includes all Enterprise features plus horizontal scaling, multi-site support, and cloud integrations.

```bash
# Configure for Datacenter
cmake -DBUILD_VERSION=datacenter ..

# Build
make

# Output: simple-dhcpd (Datacenter binary)
```

**Additional Features:**
- Horizontal scaling
- Multi-site synchronization
- Cloud integrations (AWS, Azure, GCP)
- Plugin architecture
- Multi-tenant support

**Note:** Datacenter features are planned. The build will succeed but features are not yet implemented.

---

## Testing

### Running Tests

```bash
# Build with tests enabled (default)
cmake -DENABLE_TESTS=ON ..
make

# Run all tests
make test

# Or use CTest
ctest

# Run with verbose output
ctest --verbose

# Run specific test
ctest -R test_name
```

### Test Coverage

```bash
# Build with coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make

# Run tests
make test

# Generate coverage report
make coverage
```

### Unit Tests

```bash
# Run unit tests only
./tests/simple_dhcpd_tests --gtest_filter="UnitTest*"
```

### Integration Tests

```bash
# Run integration tests
./tests/simple_dhcpd_tests --gtest_filter="IntegrationTest*"
```

### Performance Tests

```bash
# Run performance benchmarks
./tests/simple_dhcpd_tests --gtest_filter="PerformanceTest*"
```

---

## Development Workflow

### 1. Create Feature Branch

```bash
git checkout -b feature/your-feature-name
```

### 2. Make Changes

- Follow coding standards (see [Code Style](#code-style))
- Write tests for new features
- Update documentation

### 3. Build and Test

```bash
# Clean build
rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run tests
make test

# Check for memory leaks (Linux/macOS)
valgrind --leak-check=full ./simple-dhcpd --test-config
```

### 4. Commit Changes

```bash
git add .
git commit -m "feat: Add your feature description"
```

### 5. Push and Create Pull Request

```bash
git push origin feature/your-feature-name
```

---

## Code Structure

### Directory Layout

```
simple-dhcpd/
├── src/
│   ├── core/              # Shared core (all versions)
│   │   ├── dhcp/         # DHCP protocol
│   │   ├── lease/        # Lease management
│   │   ├── network/      # Network layer
│   │   ├── config/       # Configuration
│   │   ├── options/      # DHCP options
│   │   └── utils/        # Utilities
│   ├── production/        # Production-specific
│   │   ├── security/     # Basic security
│   │   └── features/      # Production features
│   ├── enterprise/        # Enterprise-specific
│   └── datacenter/        # Datacenter-specific
├── include/
│   └── simple-dhcpd/      # Header files (mirrors src/)
├── main/                  # Version-specific main files
├── tests/                 # Test suite
├── config/                # Configuration examples
├── docs/                  # Documentation
└── CMakeLists.txt         # Build configuration
```

### Adding New Code

#### Core Code (Shared)
- Add to `src/core/` and `include/simple-dhcpd/core/`
- Available to all versions
- Must not depend on version-specific code

#### Production Code
- Add to `src/production/` and `include/simple-dhcpd/production/`
- Production version only
- Can depend on core code

#### Enterprise Code
- Add to `src/enterprise/` and `include/simple-dhcpd/enterprise/`
- Enterprise and Datacenter versions
- Can depend on core and production code

#### Datacenter Code
- Add to `src/datacenter/` and `include/simple-dhcpd/datacenter/`
- Datacenter version only
- Can depend on core, production, and enterprise code

---

## Code Style

### C++ Standards

- **C++17** required
- Use modern C++ features (smart pointers, auto, range-based for, etc.)
- Avoid raw pointers where possible

### Naming Conventions

- **Classes:** `PascalCase` (e.g., `DhcpServer`)
- **Functions:** `snake_case` (e.g., `parse_message`)
- **Variables:** `snake_case` (e.g., `config_file`)
- **Constants:** `UPPER_SNAKE_CASE` (e.g., `MAX_LEASE_TIME`)
- **Namespaces:** `snake_case` (e.g., `simple_dhcpd`)

### File Organization

- One class per header file
- Header guards: `#ifndef SIMPLE_DHCPD_MODULE_NAME_HPP`
- Include order:
  1. Corresponding header
  2. System headers
  3. Third-party headers
  4. Project headers

### Example

```cpp
#ifndef SIMPLE_DHCPD_CORE_PARSER_HPP
#define SIMPLE_DHCPD_CORE_PARSER_HPP

#include <vector>
#include <cstdint>
#include "simple-dhcpd/core/types.hpp"

namespace simple_dhcpd {

class DhcpParser {
public:
    static DhcpMessage parse_message(const std::vector<uint8_t>& data);
    // ...
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_CORE_PARSER_HPP
```

---

## Contributing

### Before Contributing

1. Read [CONTRIBUTING.md](../CONTRIBUTING.md)
2. Check existing issues and pull requests
3. Discuss major changes in an issue first

### Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Update documentation
6. Ensure all tests pass
7. Submit pull request

### Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: Add new feature
fix: Fix bug
docs: Update documentation
test: Add tests
refactor: Code refactoring
style: Code style changes
chore: Maintenance tasks
```

### Code Review

- All code must be reviewed before merging
- Address review comments promptly
- Keep pull requests focused and small

---

## Troubleshooting

### Build Issues

**CMake not finding dependencies:**
```bash
# Set PKG_CONFIG_PATH (Linux/macOS)
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

# Or specify paths explicitly
cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl ..
```

**Compilation errors:**
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
make clean
make
```

**Link errors:**
```bash
# Check library paths
ldconfig -p | grep ssl
pkg-config --libs jsoncpp
```

### Runtime Issues

**Permission denied:**
```bash
# DHCP requires root/administrator privileges
sudo ./simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf
```

**Port already in use:**
```bash
# Check what's using port 67
sudo netstat -tulpn | grep :67
sudo lsof -i :67
```

---

## Additional Resources

- [Configuration Guide](../shared/configuration/README.md)
- [API Documentation](../api/)
- [Architecture Diagrams](../shared/diagrams/architecture.md)
- [Project Roadmap](../../project/ROADMAP_CHECKLIST.md)

---

**Last Updated:** December 2024
**Version:** 1.0.0
