#!/bin/bash
# Pre-uninstallation script for simple-dhcpd RPM

set -e

PROJECT_NAME="simple-dhcpd"

# Stop service before removal
if [ "$1" -eq 0 ]; then
    systemctl stop "$simple-dhcpd" 2>/dev/null || true
    systemctl disable "$simple-dhcpd" 2>/dev/null || true
fi

exit 0

