# simple-dhcpd Docker Deployment

This directory contains Docker deployment examples for simple-dhcpd.

## Quick Start

1. **Build the Docker image:**
   ```bash
   docker build -t simple-dhcpd:latest .
   ```

2. **Run with Docker Compose:**
   ```bash
   docker-compose up -d
   ```

3. **Check status:**
   ```bash
   docker-compose ps
   docker-compose logs simple-dhcpd
   ```

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `SIMPLE-DHCPD_CONFIG` | `/etc/simple-dhcpd/simple-dhcpd.conf` | Configuration file path |
| `SIMPLE-DHCPD_LOG_LEVEL` | `info` | Log level (debug, info, warn, error) |
| `SIMPLE-DHCPD_BIND_ADDRESS` | `0.0.0.0` | Bind address |
| `SIMPLE-DHCPD_BIND_PORT` | `67` | Bind port |

### Volumes

| Volume | Description |
|--------|-------------|
| `./config:/etc/simple-dhcpd:ro` | Configuration files (read-only) |
| `./data:/var/lib/simple-dhcpd` | Data directory |
| `./logs:/var/log/simple-dhcpd` | Log files |

## Docker Compose Commands

```bash
# Start services
docker-compose up -d

# Stop services
docker-compose down

# View logs
docker-compose logs -f simple-dhcpd

# Restart service
docker-compose restart simple-dhcpd

# Update and restart
docker-compose pull
docker-compose up -d

# Remove everything
docker-compose down -v
```

## Health Checks

The container includes health checks that verify the service is responding:

- **Check command:** `nc -z localhost 67`
- **Interval:** 30 seconds
- **Timeout:** 10 seconds
- **Retries:** 3
- **Start period:** 40 seconds

## Networking

The service uses a custom bridge network (`simple-dhcpd-network`) with subnet `172.22.0.0/16`.

## Security Considerations

1. **Run as non-root user** (configured in Dockerfile)
2. **Read-only configuration** volume
3. **Network isolation** with custom bridge
4. **Resource limits** (configure as needed)

## Troubleshooting

### Container won't start
```bash
# Check logs
docker-compose logs simple-dhcpd

# Check configuration
docker-compose exec simple-dhcpd cat /etc/simple-dhcpd/simple-dhcpd.conf
```

### Port conflicts
```bash
# Check what's using the port
netstat -tlnp | grep 67

# Change port in docker-compose.yml
ports:
  - "8080:67/tcp"
```

### Permission issues
```bash
# Fix volume permissions
sudo chown -R 1000:1000 ./data ./logs
```

## Production Deployment

For production deployments, consider:

1. **Use specific image tags** instead of `latest`
2. **Set resource limits** in docker-compose.yml
3. **Configure log rotation** for log volumes
4. **Use secrets management** for sensitive configuration
5. **Set up monitoring** and alerting
6. **Configure backup** for data volumes

## Examples

### Development
```bash
# Override configuration for development
docker-compose -f docker-compose.yml -f docker-compose.dev.yml up -d
```

### Production
```bash
# Use production configuration
docker-compose -f docker-compose.yml -f docker-compose.prod.yml up -d
```
