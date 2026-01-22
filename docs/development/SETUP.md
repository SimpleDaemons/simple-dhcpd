# Developer Setup Guide

Step-by-step guide to set up your development environment for Simple DHCP Daemon.

## Prerequisites Checklist

- [ ] C++ compiler (GCC 7+ or Clang 8+)
- [ ] CMake 3.16+
- [ ] Git
- [ ] OpenSSL development libraries
- [ ] jsoncpp development libraries
- [ ] pkg-config

---

## Step 1: Install System Dependencies

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libjsoncpp-dev \
    pkg-config \
    valgrind \
    doxygen
```

### Linux (CentOS/RHEL/Fedora)

```bash
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake \
    git \
    openssl-devel \
    jsoncpp-devel \
    pkgconfig \
    valgrind \
    doxygen
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install \
    cmake \
    openssl \
    jsoncpp \
    pkg-config \
    doxygen
```

### Windows

1. Install **Visual Studio 2019 or later** with:
   - Desktop development with C++
   - Windows 10/11 SDK

2. Install **CMake** from [cmake.org](https://cmake.org/download/)

3. Install **vcpkg**:
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

4. Install dependencies:
   ```powershell
   .\vcpkg install openssl:x64-windows jsoncpp:x64-windows
   ```

---

## Step 2: Clone Repository

```bash
# Clone the repository
git clone https://github.com/SimpleDaemons/simple-dhcpd.git
cd simple-dhcpd

# Or if you have SSH access
git clone git@github.com:SimpleDaemons/simple-dhcpd.git
cd simple-dhcpd
```

---

## Step 3: Verify Dependencies

### Check Compiler

```bash
# Check GCC version
g++ --version
# Should show: g++ (GCC) 7.0.0 or higher

# Check Clang version
clang++ --version
# Should show: clang version 8.0.0 or higher
```

### Check CMake

```bash
cmake --version
# Should show: cmake version 3.16.0 or higher
```

### Check Libraries

```bash
# Check OpenSSL
pkg-config --modversion openssl
# Should show: 1.1.0 or higher

# Check jsoncpp
pkg-config --modversion jsoncpp
# Should show: 1.7.0 or higher
```

### Check Git

```bash
git --version
# Should show: git version 2.0.0 or higher
```

---

## Step 4: Initial Build

### Create Build Directory

```bash
mkdir build
cd build
```

### Configure Build

```bash
# Production version (recommended for development)
cmake -DBUILD_VERSION=production \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_TESTS=ON \
      ..
```

### Build

```bash
# Build all targets
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS

# Or use CMake
cmake --build . -j$(nproc)
```

### Verify Build

```bash
# Check if binary was created
ls -lh simple-dhcpd

# Test the binary
./simple-dhcpd --version
./simple-dhcpd --help
```

---

## Step 5: Run Tests

```bash
# Run all tests
make test

# Or use CTest
ctest

# Run with verbose output
ctest --verbose

# Run specific test suite
ctest -R UnitTest
```

---

## Step 6: Configure Development Tools

### Editor Configuration

#### VS Code

Create `.vscode/settings.json`:
```json
{
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/include"
    ],
    "files.associations": {
        "*.hpp": "cpp"
    }
}
```

#### Vim/Neovim

Add to `.vimrc`:
```vim
set path+=include
set suffixesadd=.hpp,.cpp
```

### Code Formatting

#### clang-format

Create `.clang-format`:
```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
Cpp11BracedListStyle: true
```

Format code:
```bash
find src include -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

---

## Step 7: Development Workflow

### Daily Development

1. **Update your branch:**
   ```bash
   git pull origin main
   ```

2. **Create feature branch:**
   ```bash
   git checkout -b feature/your-feature
   ```

3. **Make changes and test:**
   ```bash
   # Build
   cd build
   make -j$(nproc)

   # Test
   make test
   ```

4. **Commit changes:**
   ```bash
   git add .
   git commit -m "feat: Add your feature"
   ```

5. **Push and create PR:**
   ```bash
   git push origin feature/your-feature
   ```

### Debugging

#### GDB (Linux/macOS)

```bash
# Build debug version
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with GDB
gdb ./simple-dhcpd
(gdb) run -c /path/to/config.conf
```

#### LLDB (macOS)

```bash
# Run with LLDB
lldb ./simple-dhcpd
(lldb) run -c /path/to/config.conf
```

#### Visual Studio Debugger (Windows)

1. Open project in Visual Studio
2. Set breakpoints
3. Press F5 to debug

### Memory Debugging

#### Valgrind (Linux)

```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./simple-dhcpd --test-config
```

#### AddressSanitizer

```bash
# Build with sanitizer
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_SANITIZER=ON \
      ..
make
./simple-dhcpd --test-config
```

---

## Step 8: Optional Tools

### Documentation Generation

```bash
# Install Doxygen
sudo apt install doxygen graphviz  # Linux
brew install doxygen graphviz      # macOS

# Generate documentation
doxygen Doxyfile
# Output in docs/html/
```

### Static Analysis

#### cppcheck

```bash
# Install
sudo apt install cppcheck  # Linux
brew install cppcheck      # macOS

# Run
cppcheck --enable=all --std=c++17 src/ include/
```

#### clang-tidy

```bash
# Install
sudo apt install clang-tidy  # Linux
brew install llvm             # macOS (includes clang-tidy)

# Run
clang-tidy src/**/*.cpp -- -std=c++17
```

---

## Troubleshooting Setup

### Issue: CMake can't find OpenSSL

**Solution:**
```bash
# Linux
sudo apt install libssl-dev

# macOS
brew install openssl
export PKG_CONFIG_PATH=/usr/local/opt/openssl/lib/pkgconfig:$PKG_CONFIG_PATH

# Or specify path
cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl ..
```

### Issue: CMake can't find jsoncpp

**Solution:**
```bash
# Linux
sudo apt install libjsoncpp-dev

# macOS
brew install jsoncpp

# Or specify path
cmake -DJSONCPP_ROOT_DIR=/usr/local ..
```

### Issue: Permission denied errors

**Solution:**
```bash
# Make scripts executable
chmod +x scripts/*.sh

# For running DHCP server, use sudo
sudo ./simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf
```

### Issue: Build fails with "undefined reference"

**Solution:**
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
make clean
make
```

---

## Next Steps

- Read [Developer README](README.md) for development guidelines
- Check [Build Guide](BUILD_GUIDE.md) for build commands
- Review [Code Structure](../../REORGANIZATION_STATUS.md)
- Join development discussions

---

**Last Updated:** December 2024
