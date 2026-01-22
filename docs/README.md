# Simple DHCP Daemon Documentation

Welcome to the Simple DHCP Daemon documentation. This guide will help you understand, configure, and deploy the Simple DHCP Daemon effectively.

## Product Versions

Simple DHCP Daemon is available in three versions, each designed for different deployment scenarios:

### 🏭 Production Version
**License:** Apache 2.0
**Status:** ✅ 100% Complete - Ready for v1.0.0 Release
**Target:** Small to medium deployments, single-server installations

- Complete DHCP protocol implementation (DORA process)
- Advanced lease management with conflict resolution
- Basic security features (Option 82, security logging, trusted relay agents)
- Multi-format configuration (JSON, YAML, INI)
- Comprehensive documentation
- Full test coverage (unit, integration, performance, load)

**Documentation:** [Production Version Documentation](production/README.md)

### 🏢 Enterprise Version
**License:** BSL 1.1 (Business Source License 1.1)
**Status:** 🚧 ~15% Complete (Foundation Ready)
**Target:** Large deployments, multi-server environments, enterprise integrations

- All Production Version features included
- Advanced security features (enhanced snooping, authentication, access control)
- High availability (DHCP failover protocol)
- Web management interface
- REST API for management
- SNMP integration
- DNS integration
- LDAP/AD integration

**Documentation:** [Enterprise Version Documentation](enterprise/README.md)

### 🏛️ Datacenter Version
**License:** BSL 1.1 (Business Source License 1.1)
**Status:** 📋 Planned
**Target:** Large-scale datacenter deployments, cloud environments, multi-site operations

- All Enterprise Version features included
- Horizontal scaling support
- Multi-site synchronization
- Advanced monitoring and analytics
- Cloud service integrations
- Plugin architecture
- Advanced performance optimizations
- Multi-tenant support

**Documentation:** [Datacenter Version Documentation](datacenter/README.md)

---

## Documentation Structure

### 📚 Shared Documentation
Common documentation applicable to all versions:

- **[Getting Started](shared/getting-started/)** - Installation and quick start guides
- **[Configuration](shared/configuration/)** - Configuration reference and examples
- **[Diagrams](shared/diagrams/)** - Architecture and flow diagrams
- **[Troubleshooting](shared/troubleshooting/)** - Common issues and debugging
- **[Support](shared/support/)** - FAQ and support resources

### 🏭 Production Version Documentation
- **[Production Guide](production/README.md)** - Complete Production Version documentation
- **[Installation](production/installation.md)** - Production installation guide
- **[Configuration](production/configuration.md)** - Production configuration reference
- **[Deployment](production/deployment.md)** - Production deployment guide
- **[Security](production/security.md)** - Production security best practices
- **[Performance](production/performance.md)** - Production performance tuning
- **[Operations](production/operations.md)** - Production operations guide

### 🏢 Enterprise Version Documentation
- **[Enterprise Guide](enterprise/README.md)** - Complete Enterprise Version documentation
- **[Installation](enterprise/installation.md)** - Enterprise installation guide
- **[High Availability](enterprise/high-availability.md)** - HA setup and configuration
- **[Management Interface](enterprise/management.md)** - Web UI and REST API
- **[Integrations](enterprise/integrations.md)** - DNS, LDAP, SNMP integrations
- **[Security](enterprise/security.md)** - Advanced security features
- **[Scalability](enterprise/scalability.md)** - Scaling and performance optimization

### 🏛️ Datacenter Version Documentation
- **[Datacenter Guide](datacenter/README.md)** - Complete Datacenter Version documentation
- **[Installation](datacenter/installation.md)** - Datacenter installation guide
- **[Multi-Site Setup](datacenter/multi-site.md)** - Multi-site configuration
- **[Cloud Deployment](datacenter/cloud.md)** - Cloud deployment guides
- **[Scaling](datacenter/scaling.md)** - Horizontal scaling and load balancing
- **[Monitoring](datacenter/monitoring.md)** - Advanced monitoring and analytics
- **[Plugins](datacenter/plugins.md)** - Plugin architecture and development

---

## Quick Start

### Production Version
1. [Install Simple DHCP Daemon](shared/getting-started/installation.md)
2. [Quick Start Guide](shared/getting-started/quick-start.md)
3. [Production Configuration](production/configuration.md)
4. [Production Deployment](production/deployment.md)

### Enterprise Version
1. [Install Simple DHCP Daemon](shared/getting-started/installation.md)
2. [Enterprise Setup](enterprise/README.md)
3. [High Availability Configuration](enterprise/high-availability.md)
4. [Management Interface](enterprise/management.md)

### Datacenter Version
1. [Install Simple DHCP Daemon](shared/getting-started/installation.md)
2. [Datacenter Setup](datacenter/README.md)
3. [Multi-Site Configuration](datacenter/multi-site.md)
4. [Scaling Guide](datacenter/scaling.md)

---

## Version Comparison

| Feature | Production | Enterprise | Datacenter |
|---------|-----------|------------|------------|
| **License** | Apache 2.0 | BSL 1.1 | BSL 1.1 |
| **DHCP Protocol** | ✅ Complete | ✅ Complete | ✅ Complete |
| **Lease Management** | ✅ Advanced | ✅ Advanced | ✅ Advanced |
| **Basic Security** | ✅ | ✅ | ✅ |
| **Advanced Security** | ❌ | ✅ | ✅ |
| **High Availability** | ❌ | ✅ | ✅ |
| **Web Management** | ❌ | ✅ | ✅ |
| **REST API** | ❌ | ✅ | ✅ |
| **SNMP Integration** | ❌ | ✅ | ✅ |
| **DNS Integration** | ❌ | ✅ | ✅ |
| **LDAP/AD Integration** | ❌ | ✅ | ✅ |
| **Horizontal Scaling** | ❌ | ❌ | ✅ |
| **Multi-Site Sync** | ❌ | ❌ | ✅ |
| **Cloud Integration** | ❌ | ❌ | ✅ |
| **Plugin System** | ❌ | ❌ | ✅ |
| **Multi-Tenant** | ❌ | ❌ | ✅ |

---

## Documentation by Topic

### Getting Started
- [Installation Guide](shared/getting-started/installation.md) - Install on Linux, macOS, Windows
- [Quick Start](shared/getting-started/quick-start.md) - Get running in minutes
- [First Steps](shared/getting-started/first-steps.md) - Basic configuration

### Configuration
- [Configuration Reference](shared/configuration/README.md) - Complete configuration guide
- [Production Configuration](production/configuration.md) - Production-specific configuration
- [Enterprise Configuration](enterprise/configuration.md) - Enterprise-specific configuration
- [Datacenter Configuration](datacenter/configuration.md) - Datacenter-specific configuration

### Deployment
- [Production Deployment](production/deployment.md) - Production deployment guide
- [Enterprise Deployment](enterprise/deployment.md) - Enterprise deployment guide
- [Datacenter Deployment](datacenter/deployment.md) - Datacenter deployment guide
- [Docker Deployment](shared/deployment/docker.md) - Containerized deployment
- [High Availability](enterprise/high-availability.md) - HA setup (Enterprise+)

### Operations
- [Production Operations](production/operations.md) - Production operations guide
- [Monitoring](shared/deployment/monitoring.md) - Monitoring setup
- [Backup Procedures](shared/deployment/backup-procedures.md) - Backup and restore
- [Maintenance](shared/deployment/maintenance-procedures.md) - Maintenance procedures

### Security
- [Production Security](production/security.md) - Production security best practices
- [Enterprise Security](enterprise/security.md) - Advanced security features
- [Security Best Practices](shared/user-guide/security-best-practices.md) - General security guide

### Performance
- [Production Performance](production/performance.md) - Production performance tuning
- [Enterprise Performance](enterprise/performance.md) - Enterprise performance optimization
- [Datacenter Performance](datacenter/performance.md) - Datacenter scaling and optimization

### Troubleshooting
- [Common Issues](shared/troubleshooting/README.md) - Troubleshooting guide
- [Debugging](shared/troubleshooting/debugging.md) - Debugging techniques
- [Performance Issues](shared/troubleshooting/performance.md) - Performance troubleshooting

---

## Contributing to Documentation

We welcome contributions to improve this documentation. Please see our [Contributing Guide](../CONTRIBUTING.md) for details on how to contribute.

## Feedback

If you find any issues with the documentation or have suggestions for improvement, please:

1. Open an issue on [GitHub](https://github.com/SimpleDaemons/simple-dhcpd/issues)
2. Submit a pull request with your improvements
3. Contact us at docs@simpledaemons.com

---

**Last Updated**: December 2024
**Production Version**: 1.0.0 (Ready for Release)
**Enterprise Version**: Foundation Complete
**Datacenter Version**: Planned
