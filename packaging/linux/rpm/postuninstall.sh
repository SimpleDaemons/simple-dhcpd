#!/bin/bash
# Post-uninstallation script for simple-dhcpd RPM

set -e

# Reload systemd
systemctl daemon-reload

exit 0

