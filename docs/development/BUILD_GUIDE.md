# Build Guide

Complete guide to building Simple DHCP Daemon for different versions and platforms.

## Quick Start

```bash
# Clone repository
git clone https://github.com/SimpleDaemons/simple-dhcpd.git
cd simple-dhcpd

# Build Production version
mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make

# Run
./simple-dhcpd --help
```

---

## Build Commands Reference

### Basic Build

```bash
# Configure
cmake ..

# Build
make

# Install (requires root)
sudo make install
```

### Version-Specific Builds

#### Production Version
```bash
cmake -DBUILD_VERSION=production ..
make
```

#### Enterprise Version
```bash
cmake -DBUILD_VERSION=enterprise ..
make
```

#### Datacenter Version
```bash
cmake -DBUILD_VERSION=datacenter ..
make
```

### Build Types

#### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

#### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

#### RelWithDebInfo Build
```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```

### Build Options

#### Disable Tests
```bash
cmake -DENABLE_TESTS=OFF ..
```

#### Disable Packaging
```bash
cmake -DENABLE_PACKAGING=OFF ..
```

#### Static Linking
```bash
cmake -DENABLE_STATIC_LINKING=ON ..
```

#### Disable SSL
```bash
cmake -DENABLE_SSL=OFF ..
```

#### Disable JSON
```bash
cmake -DENABLE_JSON=OFF ..
```

### Parallel Builds

```bash
# Linux
make -j$(nproc)

# macOS
make -j$(sysctl -n hw.ncpu)

# Windows (Visual Studio)
cmake --build . --parallel
```

---

## Platform-Specific Builds

### Linux

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git libssl-dev libjsoncpp-dev pkg-config

# Build
mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make -j$(nproc)
```

#### CentOS/RHEL
```bash
# Install dependencies
sudo yum groupinstall "Development Tools"
sudo yum install cmake git openssl-devel jsoncpp-devel pkgconfig

# Build
mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make -j$(nproc)
```

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S base-devel cmake git openssl jsoncpp pkgconf

# Build
mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make -j$(nproc)
```

### macOS

```bash
# Install dependencies
brew install cmake openssl jsoncpp pkg-config

# Build
mkdir build && cd build
cmake -DBUILD_VERSION=production \
      -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
      ..
make -j$(sysctl -n hw.ncpu)
```

### Windows

#### Visual Studio
```powershell
# Install dependencies via vcpkg
vcpkg install openssl:x64-windows jsoncpp:x64-windows

# Configure
mkdir build
cd build
cmake -DBUILD_VERSION=production `
      -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
      ..

# Build
cmake --build . --config Release
```

#### MinGW
```bash
# Install dependencies
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-openssl mingw-w64-x86_64-jsoncpp

# Build
mkdir build && cd build
cmake -DBUILD_VERSION=production -G "MinGW Makefiles" ..
mingw32-make
```

---

## Advanced Build Scenarios

### Cross-Compilation

#### Linux to Windows (using MinGW)
```bash
cmake -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
      -DBUILD_VERSION=production ..
```

#### Linux to macOS (using osxcross)
```bash
cmake -DCMAKE_SYSTEM_NAME=Darwin \
      -DCMAKE_C_COMPILER=o64-clang \
      -DCMAKE_CXX_COMPILER=o64-clang++ \
      -DBUILD_VERSION=production ..
```

### Docker Build

```bash
# Build in Docker container
docker build -t simple-dhcpd:build -f automation/docker/Dockerfile.build .

# Run build
docker run --rm -v $(pwd):/workspace simple-dhcpd:build \
  cmake -DBUILD_VERSION=production /workspace && \
  make -j$(nproc)
```

### CI/CD Build

See [CI/CD Configuration](../../automation/ci/) for automated build scripts.

---

## Package Generation

### Generate Packages

```bash
# Build with packaging enabled
cmake -DENABLE_PACKAGING=ON ..
make package

# Packages will be in build/ directory
```

### Package Types

#### Linux
- **DEB:** `simple-dhcpd-production-1.0.0-linux-amd64.deb`
- **RPM:** `simple-dhcpd-production-1.0.0-linux-amd64.rpm`
- **TGZ:** `simple-dhcpd-production-1.0.0-linux-amd64.tar.gz`

#### macOS
- **DMG:** `simple-dhcpd-production-1.0.0-macos.dmg`
- **PKG:** `simple-dhcpd-production-1.0.0-macos.pkg`

#### Windows
- **NSIS:** `simple-dhcpd-production-1.0.0-windows-amd64.exe`

### Source Package

```bash
make package_source
# Generates: simple-dhcpd-production-1.0.0-src.tar.gz
```

---

## Testing Builds

### Run Tests

```bash
# Build with tests
cmake -DENABLE_TESTS=ON ..
make

# Run all tests
make test

# Run specific test
ctest -R test_name

# Run with verbose output
ctest --verbose
```

### Test Coverage

```bash
# Build with coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make
make test
make coverage
```

### Memory Checking

```bash
# Build debug version
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with Valgrind (Linux)
valgrind --leak-check=full ./simple-dhcpd --test-config

# Run with AddressSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON ..
make
./simple-dhcpd --test-config
```

---

## Troubleshooting

### Common Build Errors

#### CMake Not Finding Dependencies

**Problem:** CMake can't find OpenSSL or jsoncpp

**Solution:**
```bash
# Set PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

# Or specify paths
cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
      -DJSONCPP_ROOT_DIR=/usr/local \
      ..
```

#### Compilation Errors

**Problem:** C++17 features not recognized

**Solution:**
```bash
# Check compiler version
g++ --version  # Should be 7+ or clang 8+

# Force C++17
cmake -DCMAKE_CXX_STANDARD=17 ..
```

#### Link Errors

**Problem:** Libraries not found at link time

**Solution:**
```bash
# Check library paths
ldconfig -p | grep ssl
pkg-config --libs jsoncpp

# Set library paths
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Clean Build

```bash
# Remove build directory
rm -rf build

# Rebuild
mkdir build && cd build
cmake ..
make
```

### Verbose Build

```bash
# Show compiler commands
make VERBOSE=1

# Or with CMake
cmake --build . --verbose
```

---

## Build Scripts

### Quick Build Script

Create `build.sh`:
```bash
#!/bin/bash
set -e

VERSION=${1:-production}
BUILD_TYPE=${2:-Release}

mkdir -p build
cd build

cmake -DBUILD_VERSION=$VERSION \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      ..

make -j$(nproc)
echo "Build complete: ./build/simple-dhcpd"
```

Usage:
```bash
chmod +x build.sh
./build.sh production Release
./build.sh enterprise Debug
```

---

## Performance Tips

### Faster Builds

1. **Use parallel builds:**
   ```bash
   make -j$(nproc)
   ```

2. **Use ccache:**
   ```bash
   sudo apt install ccache
   cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
   ```

3. **Incremental builds:**
   ```bash
   # Only rebuild changed files
   make
   ```

4. **Separate build directories:**
   ```bash
   # Debug build
   mkdir build-debug && cd build-debug
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   
   # Release build
   mkdir build-release && cd build-release
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

---

**Last Updated:** December 2024
