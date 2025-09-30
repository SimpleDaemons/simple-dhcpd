#!/bin/bash

# Build PKG installer for Simple DHCP Daemon
# This script creates a proper PKG installer that users can actually run

set -e

PROJECT_NAME="simple-dhcpd"
VERSION="0.3.0"
BUILD_DIR="build"
DIST_DIR="dist"
PKG_DIR="${BUILD_DIR}/pkg-root"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "${SCRIPT_DIR}")"

echo "Building PKG installer for ${PROJECT_NAME} ${VERSION}..."

# Clean and create directories
rm -rf "${PROJECT_ROOT}/${PKG_DIR}"
mkdir -p "${PROJECT_ROOT}/${PKG_DIR}"

# Install files to package root
echo "Installing files to package root..."
cd "${PROJECT_ROOT}/${BUILD_DIR}"
cmake --install . --prefix "${PROJECT_ROOT}/${PKG_DIR}/usr/local"

# Create component package
echo "Creating component package..."
pkgbuild \
    --root "${PROJECT_ROOT}/${PKG_DIR}" \
    --identifier "com.simpledaemons.${PROJECT_NAME}" \
    --version "${VERSION}" \
    --install-location "/" \
    "${PROJECT_NAME}-component.pkg"

# Create distribution file
echo "Creating distribution file..."
cat > "${PROJECT_NAME}-distribution.xml" << EOF
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<installer-gui-script minSpecVersion="1">
    <title>Simple DHCP Daemon</title>
    <welcome file="Welcome.txt"/>
    <readme file="ReadMe.txt"/>
    <license file="License.txt"/>
    <options allow-external-scripts="no" customize="allow" rootVolumeOnly="false"/>
    
    <choices-outline>
        <line choice="default">
            <line choice="${PROJECT_NAME}"/>
        </line>
    </choices-outline>
    
    <choice id="default"/>
    <choice id="${PROJECT_NAME}" visible="false">
        <pkg-ref id="com.simpledaemons.${PROJECT_NAME}"/>
    </choice>
    
    <pkg-ref id="com.simpledaemons.${PROJECT_NAME}" version="${VERSION}" onConclusion="none">${PROJECT_NAME}-component.pkg</pkg-ref>
    
    <product id="com.simpledaemons.${PROJECT_NAME}" version="${VERSION}"/>
</installer-gui-script>
EOF

# Create resource files
echo "Creating resource files..."
cp "${PROJECT_ROOT}/LICENSE" "License.txt"
cp "${PROJECT_ROOT}/README.md" "ReadMe.txt"
cat > "Welcome.txt" << EOF
Welcome to Simple DHCP Daemon

Simple DHCP Daemon is a lightweight, high-performance DHCP server implementation designed for modern networking environments.

Features:
- High Performance: Handles 50,000+ requests per second
- Cross-Platform: Runs on Linux, macOS, and Windows
- Modern C++: Built with C++17 for reliability and performance
- JSON Configuration: Easy-to-use JSON-based configuration
- Comprehensive Logging: Detailed logging with multiple output formats
- Security Features: MAC filtering, rate limiting, and authentication support
- Lease Management: Dynamic and static lease allocation with database persistence

Installation:
After installation, the DHCP daemon will be available as 'simple-dhcpd' in your PATH.

Configuration:
Default configuration files are installed to /etc/simple-dhcpd/
Example configurations are available in /etc/simple-dhcpd/examples/

Documentation:
For detailed documentation, visit: https://github.com/SimpleDaemons/simple-dhcpd

Support:
- GitHub Issues: https://github.com/SimpleDaemons/simple-dhcpd/issues
- Email: support@simpledaemons.com

Thank you for using Simple DHCP Daemon!
EOF

# Create final PKG installer
echo "Creating final PKG installer..."
productbuild \
    --distribution "${PROJECT_NAME}-distribution.xml" \
    --package-path "." \
    --resources "." \
    "${PROJECT_ROOT}/${DIST_DIR}/${PROJECT_NAME}-${VERSION}-macOS.pkg"

echo "PKG installer created: ${PROJECT_ROOT}/${DIST_DIR}/${PROJECT_NAME}-${VERSION}-macOS.pkg"

# Clean up
rm -rf "${PROJECT_ROOT}/${PKG_DIR}"
rm -f "${PROJECT_NAME}-component.pkg"
rm -f "${PROJECT_NAME}-distribution.xml"
rm -f "License.txt"
rm -f "ReadMe.txt"
rm -f "Welcome.txt"

echo "PKG installer build completed successfully!"
