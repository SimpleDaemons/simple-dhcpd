#!/bin/bash
# Pre-installation script for simple-dhcpd RPM
# License acceptance check

set -e

PROJECT_NAME="simple-dhcpd"

# Display license and require acceptance
echo "=========================================="
echo "simple-dhcpd License Agreement"
echo "=========================================="
echo ""
cat /usr/share/doc/$simple-dhcpd/LICENSE 2>/dev/null || \
    cat /usr/share/$simple-dhcpd/LICENSE.txt 2>/dev/null || \
    echo "Please review the license at: https://github.com/simpledaemons/$simple-dhcpd/blob/main/LICENSE"
echo ""
echo "By continuing, you agree to the terms of the license."
echo ""
read -p "Do you accept the license? [y/N]: " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "License not accepted. Installation cancelled."
    exit 1
fi

exit 0

