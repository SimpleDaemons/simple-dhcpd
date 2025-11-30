#!/bin/bash
# Post-installation script for simple-dhcpd RPM

set -e

PROJECT_NAME="simple-dhcpd"
SERVICE_USER="dhcpddev"

# Create service user if it doesn't exist
if ! id "$SERVICE_USER" &>/dev/null; then
    useradd -r -s /sbin/nologin -d /var/lib/$simple-dhcpd -c "$simple-dhcpd service user" "$SERVICE_USER"
fi

# Set ownership
chown -R "$SERVICE_USER:$SERVICE_USER" /etc/$simple-dhcpd 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/log/$simple-dhcpd 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/lib/$simple-dhcpd 2>/dev/null || true

# Enable and start service
systemctl daemon-reload
systemctl enable "$simple-dhcpd" 2>/dev/null || true
systemctl start "$simple-dhcpd" 2>/dev/null || true

exit 0

