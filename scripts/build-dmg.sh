#!/bin/bash

# Build enhanced DMG installer for Simple DHCP Daemon
# This script creates a professional DMG with installer script and proper layout

set -e

PROJECT_NAME="simple-dhcpd"
VERSION="0.1.0"
BUILD_DIR="build"
DIST_DIR="dist"
DMG_NAME="${PROJECT_NAME}-${VERSION}-macOS"
DMG_TEMP_DIR="${BUILD_DIR}/dmg-temp"
DMG_MOUNT_POINT="/Volumes/${PROJECT_NAME}"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "${SCRIPT_DIR}")"

echo "Building enhanced DMG installer for ${PROJECT_NAME} ${VERSION}..."

# Clean and create directories
rm -rf "${PROJECT_ROOT}/${DMG_TEMP_DIR}"
mkdir -p "${PROJECT_ROOT}/${DMG_TEMP_DIR}"

# Install files to DMG temp directory
echo "Installing files to DMG temp directory..."
cd "${PROJECT_ROOT}/${BUILD_DIR}"
cmake --install . --prefix "../${DMG_TEMP_DIR}/usr/local"

# Create DMG layout
echo "Creating DMG layout..."
mkdir -p "${PROJECT_ROOT}/${DMG_TEMP_DIR}/Applications"
mkdir -p "${PROJECT_ROOT}/${DMG_TEMP_DIR}/.background"

# Create installer script
echo "Creating installer script..."
cat > "${PROJECT_ROOT}/${DMG_TEMP_DIR}/Install Simple DHCP Daemon.command" << 'EOF'
#!/bin/bash

# Simple DHCP Daemon Installer
# This script installs Simple DHCP Daemon to your system

set -e

echo "Simple DHCP Daemon Installer"
echo "============================"
echo ""

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo "This installer should not be run as root."
   echo "Please run it as a regular user. It will ask for admin password when needed."
   exit 1
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DMG_ROOT="$(dirname "${SCRIPT_DIR}")"

echo "Installing Simple DHCP Daemon..."
echo ""

# Create directories
echo "Creating system directories..."
sudo mkdir -p /usr/local/bin
sudo mkdir -p /usr/local/include
sudo mkdir -p /usr/local/etc/simple-dhcpd
sudo mkdir -p /Library/LaunchDaemons

# Install binary
echo "Installing binary..."
sudo cp "${DMG_ROOT}/usr/local/bin/simple-dhcpd" /usr/local/bin/
sudo chmod +x /usr/local/bin/simple-dhcpd

# Install headers
echo "Installing header files..."
sudo cp -r "${DMG_ROOT}/usr/local/include/"* /usr/local/include/

# Install configuration files
echo "Installing configuration files..."
sudo cp -r "${DMG_ROOT}/usr/local/etc/simple-dhcpd/"* /usr/local/etc/simple-dhcpd/

# Install LaunchDaemon
echo "Installing LaunchDaemon..."
sudo cp "${DMG_ROOT}/usr/local/Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist" /Library/LaunchDaemons/
sudo chmod 644 /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist

# Load LaunchDaemon
echo "Loading LaunchDaemon..."
sudo launchctl load /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist 2>/dev/null || true

echo ""
echo "Installation completed successfully!"
echo ""
echo "Simple DHCP Daemon has been installed to:"
echo "  Binary: /usr/local/bin/simple-dhcpd"
echo "  Config: /usr/local/etc/simple-dhcpd/"
echo "  Headers: /usr/local/include/"
echo ""
echo "Configuration examples are available in:"
echo "  /usr/local/etc/simple-dhcpd/examples/"
echo ""
echo "To start the service:"
echo "  sudo launchctl start com.simple-dhcpd.simple-dhcpd"
echo ""
echo "To stop the service:"
echo "  sudo launchctl stop com.simple-dhcpd.simple-dhcpd"
echo ""
echo "To uninstall, run:"
echo "  sudo launchctl unload /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist"
echo "  sudo rm -rf /usr/local/bin/simple-dhcpd"
echo "  sudo rm -rf /usr/local/include/simple-dhcpd*"
echo "  sudo rm -rf /usr/local/etc/simple-dhcpd"
echo "  sudo rm /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist"
echo ""
echo "Press any key to continue..."
read -n 1 -s
EOF

chmod +x "${PROJECT_ROOT}/${DMG_TEMP_DIR}/Install Simple DHCP Daemon.command"

# Create uninstaller script
echo "Creating uninstaller script..."
cat > "${PROJECT_ROOT}/${DMG_TEMP_DIR}/Uninstall Simple DHCP Daemon.command" << 'EOF'
#!/bin/bash

# Simple DHCP Daemon Uninstaller
# This script removes Simple DHCP Daemon from your system

set -e

echo "Simple DHCP Daemon Uninstaller"
echo "==============================="
echo ""

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo "This uninstaller should not be run as root."
   echo "Please run it as a regular user. It will ask for admin password when needed."
   exit 1
fi

echo "This will remove Simple DHCP Daemon from your system."
echo "Are you sure you want to continue? (y/N)"
read -r response
if [[ ! "$response" =~ ^[Yy]$ ]]; then
    echo "Uninstall cancelled."
    exit 0
fi

echo ""
echo "Uninstalling Simple DHCP Daemon..."

# Stop and unload LaunchDaemon
echo "Stopping service..."
sudo launchctl stop com.simple-dhcpd.simple-dhcpd 2>/dev/null || true
sudo launchctl unload /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist 2>/dev/null || true

# Remove files
echo "Removing files..."
sudo rm -f /usr/local/bin/simple-dhcpd
sudo rm -rf /usr/local/include/simple-dhcpd*
sudo rm -rf /usr/local/etc/simple-dhcpd
sudo rm -f /Library/LaunchDaemons/com.simple-dhcpd.simple-dhcpd.plist

echo ""
echo "Simple DHCP Daemon has been uninstalled successfully!"
echo ""
echo "Press any key to continue..."
read -n 1 -s
EOF

chmod +x "${PROJECT_ROOT}/${DMG_TEMP_DIR}/Uninstall Simple DHCP Daemon.command"

# Create README
echo "Creating README..."
cat > "${PROJECT_ROOT}/${DMG_TEMP_DIR}/README.txt" << 'EOF'
Simple DHCP Daemon v0.1.0
=========================

A lightweight and secure DHCP server daemon for macOS.

INSTALLATION:
1. Double-click "Install Simple DHCP Daemon.command"
2. Follow the on-screen instructions
3. The installer will ask for your admin password

UNINSTALLATION:
1. Double-click "Uninstall Simple DHCP Daemon.command"
2. Follow the on-screen instructions

MANUAL INSTALLATION:
If you prefer to install manually, copy the files from the usr/local/ directory
to your system's /usr/local/ directory.

CONFIGURATION:
After installation, edit the configuration file:
  /usr/local/etc/simple-dhcpd/simple-dhcpd.conf

Example configurations are available in:
  /usr/local/etc/simple-dhcpd/examples/

SERVICE MANAGEMENT:
Start service:    sudo launchctl start com.simple-dhcpd.simple-dhcpd
Stop service:     sudo launchctl stop com.simple-dhcpd.simple-dhcpd
Check status:     sudo launchctl list | grep simple-dhcpd

DOCUMENTATION:
For detailed documentation, visit:
https://github.com/simpledaemons/simple-dhcpd

SUPPORT:
- GitHub Issues: https://github.com/simpledaemons/simple-dhcpd/issues
- Email: support@simpledaemons.com

LICENSE:
Apache 2.0 License - see LICENSE file for details.
EOF

# Create Applications folder shortcut
echo "Creating Applications folder shortcut..."
ln -sf /Applications "${PROJECT_ROOT}/${DMG_TEMP_DIR}/Applications"

# Create DMG
echo "Creating DMG..."
cd "${PROJECT_ROOT}"

# Remove existing DMG if it exists
rm -f "${DIST_DIR}/${DMG_NAME}.dmg"

# Create the DMG
hdiutil create -srcfolder "${DMG_TEMP_DIR}" -volname "${PROJECT_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDZO "${DIST_DIR}/${DMG_NAME}.dmg"

# Try to customize DMG layout (optional - may fail due to permissions)
echo "Customizing DMG layout..."
if hdiutil attach "${DIST_DIR}/${DMG_NAME}.dmg" -readwrite -noverify -noautoopen 2>/dev/null; then
    echo "Setting DMG properties..."
    osascript << EOF
tell application "Finder"
    tell disk "${PROJECT_NAME}"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {100, 100, 600, 400}
        set theViewOptions to the icon view options of container window
        set arrangement of theViewOptions to not arranged
        set icon size of theViewOptions to 128
        set position of item "Install Simple DHCP Daemon.command" of container window to {150, 150}
        set position of item "Uninstall Simple DHCP Daemon.command" of container window to {300, 150}
        set position of item "README.txt" of container window to {450, 150}
        set position of item "Applications" of container window to {150, 300}
        set position of item "usr" of container window to {300, 300}
        close
        open
        update without registering applications
        delay 2
    end tell
end tell
EOF
    hdiutil detach "${DMG_MOUNT_POINT}" 2>/dev/null || true
    echo "DMG layout customized successfully"
else
    echo "Could not customize DMG layout (permissions issue), but DMG is still functional"
fi

# Convert to final format
echo "Converting to final DMG format..."
hdiutil convert "${DIST_DIR}/${DMG_NAME}.dmg" -format UDZO -o "${DIST_DIR}/${DMG_NAME}-final.dmg"
mv "${DIST_DIR}/${DMG_NAME}-final.dmg" "${DIST_DIR}/${DMG_NAME}.dmg"

# Clean up
rm -rf "${PROJECT_ROOT}/${DMG_TEMP_DIR}"

echo "Enhanced DMG installer created: ${DIST_DIR}/${DMG_NAME}.dmg"
echo "DMG installer features:"
echo "  - Double-click installer script"
echo "  - Double-click uninstaller script"
echo "  - README with instructions"
echo "  - Applications folder shortcut"
echo "  - Professional layout"
echo "  - Manual installation option"
