# Enterprise Version High Availability Guide

**Version:** Foundation Complete (v1.0.0 Alpha in development)
**License:** BSL 1.1

---

## Overview

The Enterprise Version supports high availability (HA) through DHCP failover protocol (RFC 2131), providing redundancy and automatic failover for critical DHCP services.

## HA Modes

### Active-Passive Mode

In active-passive mode, one server is active and handles all requests, while the other is in standby:

- **Active Server:** Handles all DHCP requests
- **Passive Server:** Standby, ready to take over
- **Failover:** Automatic failover on active server failure
- **State Sync:** Lease state synchronized between servers

### Active-Active Mode

In active-active mode, both servers handle requests with load balancing:

- **Load Distribution:** Requests distributed between servers
- **Split Configuration:** Configurable load split (e.g., 50/50)
- **State Sync:** Full state synchronization
- **Failover:** Automatic failover on server failure

## HA Architecture

```
┌─────────────────┐         ┌─────────────────┐
│  Active Server  │◄───────►│ Passive Server  │
│  192.168.1.10   │         │  192.168.1.11   │
└────────┬────────┘         └────────┬────────┘
         │                            │
         │                            │
         └────────────┬───────────────┘
                      │
              ┌───────▼───────┐
              │ DHCP Clients  │
              └───────────────┘
```

## Configuration

### Active Server Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "high_availability": {
        "enabled": true,
        "mode": "active-passive",
        "role": "active",
        "peer": "192.168.1.11",
        "failover_port": 647,
        "mclt": 3600,
        "max_response_delay": 60,
        "state_file": "/var/lib/simple-dhcpd/failover.state"
      }
    }
  }
}
```

### Passive Server Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "high_availability": {
        "enabled": true,
        "mode": "active-passive",
        "role": "passive",
        "peer": "192.168.1.10",
        "failover_port": 647,
        "mclt": 3600,
        "max_response_delay": 60,
        "state_file": "/var/lib/simple-dhcpd/failover.state"
      }
    }
  }
}
```

### Active-Active Configuration

```json
{
  "dhcp": {
    "enterprise": {
      "high_availability": {
        "enabled": true,
        "mode": "active-active",
        "peer": "192.168.1.11",
        "failover_port": 647,
        "load_balance": true,
        "split": 50,
        "state_file": "/var/lib/simple-dhcpd/failover.state"
      }
    }
  }
}
```

## Failover Protocol

### Communication

- **Port:** Configurable (default 647)
- **Protocol:** TCP for reliable communication
- **Heartbeat:** Regular heartbeat messages
- **State Updates:** Continuous state synchronization

### State Synchronization

- **Lease State:** All lease states synchronized
- **Conflict Resolution:** Automatic conflict resolution
- **Split Scopes:** Automatic scope splitting in active-active mode
- **Recovery:** Automatic state recovery on reconnect

## Deployment

### Network Requirements

- **Low Latency:** <10ms between servers
- **Reliable Network:** Stable connection required
- **Firewall:** Allow failover port (default 647)
- **Bandwidth:** Sufficient for state synchronization

### Firewall Configuration

```bash
# Allow failover port
sudo firewall-cmd --permanent --add-port=647/tcp
sudo firewall-cmd --reload
```

### Service Setup

```bash
# Start both servers
sudo systemctl start simple-dhcpd-enterprise

# Check HA status
simple-dhcpd-enterprise --ha-status

# Monitor failover
simple-dhcpd-enterprise --monitor-failover
```

## Monitoring

### HA Status

```bash
# Check HA status
curl -k https://your-server:8081/api/v1/ha/status

# Response:
{
  "mode": "active-passive",
  "role": "active",
  "peer": "192.168.1.11",
  "peer_status": "connected",
  "state_sync": "synchronized",
  "last_sync": "2024-12-01T10:00:00Z"
}
```

### Health Checks

```bash
# Health check endpoint
curl -k https://your-server:8081/api/v1/ha/health

# Response:
{
  "status": "healthy",
  "peer_connected": true,
  "state_synced": true,
  "uptime": 3600
}
```

## Failover Scenarios

### Automatic Failover

1. **Active Server Failure:**
   - Passive server detects failure
   - Automatic transition to active
   - Clients continue to receive leases
   - State preserved

2. **Network Partition:**
   - Both servers continue operating
   - State synchronization resumes on reconnect
   - Conflict resolution on merge

3. **Manual Failover:**
   - Controlled failover via API
   - Graceful transition
   - No service interruption

### Manual Failover

```bash
# Initiate manual failover
curl -X POST -k https://your-server:8081/api/v1/ha/failover \
  -u admin:password

# Response:
{
  "status": "success",
  "new_role": "passive",
  "transition_time": "2024-12-01T10:05:00Z"
}
```

## Best Practices

1. **Network:** Use dedicated network for failover communication
2. **Monitoring:** Set up monitoring for both servers
3. **Testing:** Regularly test failover scenarios
4. **Backup:** Backup state files regularly
5. **Documentation:** Document failover procedures

## Troubleshooting

### Common Issues

**Peer Not Connecting:**
```bash
# Check network connectivity
ping 192.168.1.11

# Check firewall
sudo firewall-cmd --list-all

# Check failover port
sudo netstat -tulpn | grep 647
```

**State Not Synchronizing:**
```bash
# Check state file permissions
ls -la /var/lib/simple-dhcpd/failover.state

# Check logs
sudo journalctl -u simple-dhcpd-enterprise -f
```

**Split Brain:**
```bash
# Force role assignment
simple-dhcpd-enterprise --force-role active

# Reset failover state
simple-dhcpd-enterprise --reset-failover
```

## Related Documentation

- [Configuration Guide](configuration.md) - HA configuration
- [Management Interface](management.md) - HA monitoring
- [Operations Guide](operations.md) - HA operations

---

**Last Updated:** December 2024
**Version:** Foundation Complete
