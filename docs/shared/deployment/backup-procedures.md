# Backup Procedures

This guide covers backup and restore procedures for Simple DHCP Daemon.

## Overview

Regular backups are essential for disaster recovery and data protection. This guide covers backup strategies and procedures.

## What to Backup

### Configuration Files

- `/etc/simple-dhcpd/simple-dhcpd.conf` - Main configuration file
- `/etc/simple-dhcpd/` - Configuration directory
- Custom configuration files

### Data Files

- `/var/lib/simple-dhcpd/leases.db` - Lease database
- `/var/lib/simple-dhcpd/` - Data directory
- Static lease files
- Reservation files

### Log Files (Optional)

- `/var/log/simple-dhcpd/` - Log directory
- Important log files for audit

## Backup Strategies

### Full Backup

Complete backup of all data and configuration:

```bash
#!/bin/bash
# full_backup.sh

BACKUP_DIR="/backup/simple-dhcpd"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_PATH="$BACKUP_DIR/full_$DATE"

mkdir -p $BACKUP_PATH

# Backup configuration
cp -r /etc/simple-dhcpd $BACKUP_PATH/config

# Backup data
cp -r /var/lib/simple-dhcpd $BACKUP_PATH/data

# Backup logs (optional)
cp -r /var/log/simple-dhcpd $BACKUP_PATH/logs

# Create archive
tar -czf $BACKUP_PATH.tar.gz $BACKUP_PATH
rm -rf $BACKUP_PATH

echo "Full backup completed: $BACKUP_PATH.tar.gz"
```

### Incremental Backup

Backup only changed files:

```bash
#!/bin/bash
# incremental_backup.sh

BACKUP_DIR="/backup/simple-dhcpd"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_PATH="$BACKUP_DIR/incremental_$DATE"

mkdir -p $BACKUP_PATH

# Backup changed files
rsync -av --link-dest=$BACKUP_DIR/latest \
    /etc/simple-dhcpd $BACKUP_PATH/config
rsync -av --link-dest=$BACKUP_DIR/latest \
    /var/lib/simple-dhcpd $BACKUP_PATH/data

# Update latest symlink
rm -f $BACKUP_DIR/latest
ln -s $BACKUP_PATH $BACKUP_DIR/latest

echo "Incremental backup completed: $BACKUP_PATH"
```

### Database-Only Backup

Backup only lease database:

```bash
#!/bin/bash
# database_backup.sh

BACKUP_DIR="/backup/simple-dhcpd"
DATE=$(date +%Y%m%d_%H%M%S)
DB_FILE="/var/lib/simple-dhcpd/leases.db"
BACKUP_FILE="$BACKUP_DIR/leases_$DATE.db"

# Stop service for consistent backup
sudo systemctl stop simple-dhcpd

# Backup database
cp $DB_FILE $BACKUP_FILE

# Start service
sudo systemctl start simple-dhcpd

# Compress backup
gzip $BACKUP_FILE

echo "Database backup completed: $BACKUP_FILE.gz"
```

## Automated Backups

### Cron-based Backups

```bash
# Add to crontab
# Daily full backup at 2 AM
0 2 * * * /usr/local/bin/full_backup.sh

# Hourly incremental backup
0 * * * * /usr/local/bin/incremental_backup.sh

# Daily database backup at 3 AM
0 3 * * * /usr/local/bin/database_backup.sh
```

### systemd Timer

```ini
# /etc/systemd/system/simple-dhcpd-backup.timer
[Unit]
Description=Simple DHCP Daemon Backup Timer
Requires=simple-dhcpd-backup.service

[Timer]
OnCalendar=daily
Persistent=true

[Install]
WantedBy=timers.target
```

```ini
# /etc/systemd/system/simple-dhcpd-backup.service
[Unit]
Description=Simple DHCP Daemon Backup
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/full_backup.sh
```

## Backup Storage

### Local Storage

```bash
# Create backup directory
sudo mkdir -p /backup/simple-dhcpd
sudo chown dhcpd:dhcpd /backup/simple-dhcpd
sudo chmod 750 /backup/simple-dhcpd
```

### Remote Storage

```bash
# Backup to remote server
rsync -av /backup/simple-dhcpd/ user@backup-server:/backups/simple-dhcpd/

# Backup to cloud storage
aws s3 sync /backup/simple-dhcpd/ s3://my-backup-bucket/simple-dhcpd/
```

### Backup Retention

```bash
#!/bin/bash
# cleanup_old_backups.sh

BACKUP_DIR="/backup/simple-dhcpd"
RETENTION_DAYS=30

# Remove backups older than retention period
find $BACKUP_DIR -type f -mtime +$RETENTION_DAYS -delete

echo "Cleaned up backups older than $RETENTION_DAYS days"
```

## Backup Encryption

### Encrypt Backups

```bash
#!/bin/bash
# encrypted_backup.sh

BACKUP_DIR="/backup/simple-dhcpd"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="$BACKUP_DIR/backup_$DATE.tar.gz"
ENCRYPTED_FILE="$BACKUP_FILE.enc"

# Create backup
tar -czf - /etc/simple-dhcpd /var/lib/simple-dhcpd | \
    openssl enc -aes-256-cbc -salt -out $ENCRYPTED_FILE

echo "Encrypted backup created: $ENCRYPTED_FILE"
```

## Restore Procedures

### Full Restore

```bash
#!/bin/bash
# restore_full.sh

BACKUP_FILE=$1

if [ -z "$BACKUP_FILE" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

# Stop service
sudo systemctl stop simple-dhcpd

# Extract backup
tar -xzf $BACKUP_FILE -C /tmp/restore

# Restore configuration
cp -r /tmp/restore/etc/simple-dhcpd/* /etc/simple-dhcpd/

# Restore data
cp -r /tmp/restore/var/lib/simple-dhcpd/* /var/lib/simple-dhcpd/

# Set permissions
sudo chown -R dhcpd:dhcpd /etc/simple-dhcpd
sudo chown -R dhcpd:dhcpd /var/lib/simple-dhcpd

# Start service
sudo systemctl start simple-dhcpd

# Cleanup
rm -rf /tmp/restore

echo "Restore completed"
```

### Database Restore

```bash
#!/bin/bash
# restore_database.sh

BACKUP_FILE=$1

if [ -z "$BACKUP_FILE" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

# Stop service
sudo systemctl stop simple-dhcpd

# Restore database
if [[ $BACKUP_FILE == *.gz ]]; then
    gunzip -c $BACKUP_FILE > /var/lib/simple-dhcpd/leases.db
else
    cp $BACKUP_FILE /var/lib/simple-dhcpd/leases.db
fi

# Set permissions
sudo chown dhcpd:dhcpd /var/lib/simple-dhcpd/leases.db
sudo chmod 640 /var/lib/simple-dhcpd/leases.db

# Start service
sudo systemctl start simple-dhcpd

echo "Database restore completed"
```

### Configuration Restore

```bash
#!/bin/bash
# restore_config.sh

BACKUP_FILE=$1

if [ -z "$BACKUP_FILE" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

# Backup current configuration
cp /etc/simple-dhcpd/simple-dhcpd.conf /etc/simple-dhcpd/simple-dhcpd.conf.backup

# Restore configuration
cp $BACKUP_FILE /etc/simple-dhcpd/simple-dhcpd.conf

# Validate configuration
simple-dhcpd -c /etc/simple-dhcpd/simple-dhcpd.conf --test-config

# Reload service
sudo systemctl reload simple-dhcpd

echo "Configuration restore completed"
```

## Backup Verification

### Verify Backup Integrity

```bash
#!/bin/bash
# verify_backup.sh

BACKUP_FILE=$1

if [ -z "$BACKUP_FILE" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

# Check if file exists
if [ ! -f "$BACKUP_FILE" ]; then
    echo "Error: Backup file not found"
    exit 1
fi

# Check file size
SIZE=$(stat -f%z "$BACKUP_FILE" 2>/dev/null || stat -c%s "$BACKUP_FILE" 2>/dev/null)
if [ $SIZE -eq 0 ]; then
    echo "Error: Backup file is empty"
    exit 1
fi

# Test archive integrity
if [[ $BACKUP_FILE == *.tar.gz ]]; then
    tar -tzf $BACKUP_FILE > /dev/null
    if [ $? -eq 0 ]; then
        echo "Backup integrity verified"
    else
        echo "Error: Backup file is corrupted"
        exit 1
    fi
fi

echo "Backup verification completed"
```

## Best Practices

### Recommendations

1. **Regular Backups**: Schedule regular backups
2. **Test Restores**: Regularly test restore procedures
3. **Multiple Copies**: Keep multiple backup copies
4. **Off-site Storage**: Store backups off-site
5. **Encryption**: Encrypt sensitive backups
6. **Documentation**: Document backup procedures
7. **Monitoring**: Monitor backup success/failure

### Backup Schedule

- **Full Backup**: Daily or weekly
- **Incremental Backup**: Hourly or daily
- **Database Backup**: Daily
- **Configuration Backup**: Before any changes

## Troubleshooting

### Common Issues

1. **Backup Fails**: Check permissions and disk space
2. **Restore Fails**: Verify backup file integrity
3. **Large Backups**: Use compression and incremental backups
4. **Backup Performance**: Schedule during low-traffic periods

## Additional Resources

- [Deployment Guide](deployment-guide.md)
- [Maintenance Procedures](maintenance-procedures.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
