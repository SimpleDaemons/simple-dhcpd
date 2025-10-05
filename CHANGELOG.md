# Changelog

All notable changes to the Simple DHCP Daemon project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Transparent configuration loading for JSON, YAML, and INI
- YAML/INI coverage for server, subnets, global options, security, logging
- Advanced snooping validation and monitoring hooks
- MAC filtering wildcard/regex matching
- IP filtering with CIDR/mask support
- Rate limiting block windows and event reporting

### Planned
- Phase 4: Enterprise Features
- Phase 5: Performance Optimization
- Phase 6: Production Readiness

## [0.3.0] - 2024-12-29

### Added
- **Phase 3: Advanced DHCP Features - COMPLETE! 🎉**
- **Advanced Lease Management**
  - Dynamic lease allocation with conflict resolution
  - Static lease allocation with MAC-to-IP binding
  - Lease renewal and expiration handling
  - Lease database management with persistence
  - Lease history tracking and analytics
  - Subnet utilization monitoring
- **Comprehensive DHCP Options System**
  - Standard DHCP options (1-255) with full support
  - Vendor-specific options with validation
  - Custom option support with templates
  - Option validation with length, format, and range checking
  - Option inheritance rules (global, subnet, pool, host)
  - Option 82 (Relay Agent Information) support
- **Advanced Security Features**
  - DHCP Snooping with trusted interface validation and monitoring
  - MAC Address Filtering with wildcard/regex matching and management events
  - IP Address Filtering with CIDR/mask support and management events
  - Rate Limiting with sliding windows, block windows, and event reporting
  - Option 82: implementation, configuration, management, and monitoring
  - Client Authentication with HMAC-SHA256 and time-window validation
  - Security Event Logging with comprehensive audit trails
  - Trusted Relay Agent management
  - Client Credential management
  - Real-time Security Statistics tracking
- **Security Configuration Examples**
  - Security-focused configuration example
  - Enhanced main configuration with all security features
  - Updated advanced configuration with comprehensive security
  - Complete security configuration reference documentation
- **Comprehensive Documentation**
  - Security configuration reference guide
  - Security best practices documentation
  - Security troubleshooting guide
  - Security compliance features documentation

### Changed
- Enhanced main configuration with full security feature support
- Updated advanced configuration with comprehensive security settings
- Improved README with security features and configuration examples

### Technical Achievements
- All Phase 3 advanced features fully implemented and functional
- Complete security framework with production-ready features
- Comprehensive configuration system supporting all security features
- Full documentation coverage for all new features
- Build system updated to version 0.3.0

## [0.2.1] - 2024-12-29

### Added
- Enhanced DMG installer with professional layout
- Double-click installer script (`Install Simple DHCP Daemon.command`)
- Double-click uninstaller script (`Uninstall Simple DHCP Daemon.command`)
- Comprehensive README with installation instructions
- Applications folder shortcut in DMG
- Manual installation option with `usr/` directory structure
- Fixed PKG installer with proper distribution package
- Professional installer experience for both DMG and PKG formats
- Complete configuration examples (simple, advanced, production)
- Comprehensive documentation structure
- Apache 2.0 LICENSE file
- WELCOME.txt for PKG installer

### Changed
- DMG installer now provides automated installation experience
- PKG installer now shows packages and can be installed successfully
- Improved package generation process with custom build scripts
- Enhanced user experience for both installer formats

### Fixed
- PKG installer package selection issue (empty choices-outline)
- DMG installer permissions and layout customization
- Package generation errors and missing files
- CMakeLists.txt configuration for proper packaging

## [0.2.0] - 2024-12-29

### Added
- Complete DHCP protocol implementation
- DHCP message parsing and generation
- DHCP option handling system
- DHCP lease management with database persistence
- DHCP state machine implementation
- UDP socket handling for DHCP communication
- Broadcast and unicast message handling
- Network interface management
- IP address validation and subnet management
- DHCP Discover, Offer, Request, ACK handling
- DHCP Release, Decline, Inform handling
- Comprehensive error handling and logging
- JSON-based configuration management
- Cross-platform build system (Linux, macOS, Windows)
- Service integration (systemd, launchd, Windows)
- Docker containerization support
- Package generation (DEB, RPM, DMG, PKG, MSI)

### Technical Details
- C++17 implementation with modern features
- Static linking for self-contained binaries
- Thread-safe logging infrastructure
- Hot-reloadable configuration system
- Comprehensive test suite
- CI/CD pipeline integration

## [0.1.0] - 2024-12-29

### Added
- Project structure and build system
- CMake configuration with static linking support
- Cross-platform build scripts (Linux, macOS, Windows)
- CI/CD pipeline setup (Travis CI, Jenkins)
- Basic daemon framework with signal handling
- Configuration management system
- Logging infrastructure with multiple output formats
- Graceful shutdown and restart capabilities
- Standardized Makefile with comprehensive targets
- Deployment configurations for all platforms:
  - systemd service files
  - macOS LaunchDaemon plist
  - Windows service configuration
- Docker containerization with multi-stage builds
- Package generation support:
  - DEB packages for Debian/Ubuntu
  - RPM packages for Red Hat/CentOS
  - DMG packages for macOS
  - MSI packages for Windows
- Comprehensive documentation structure
- Development roadmap and checklist
- Risk assessment and success metrics
- Testing strategy and quality assurance

### Technical Foundation
- Modern C++ build system with CMake
- Cross-platform compatibility
- Professional packaging and deployment
- Comprehensive documentation
- Development workflow automation
- Quality assurance processes

---

## Version History Summary

- **v0.1.0**: Foundation - Complete build system, deployment configs, and infrastructure
- **v0.2.0**: Core DHCP Protocol - Full DHCP server implementation with all essential features
- **v0.2.1**: Enhanced Packaging - Professional installers and improved user experience

## Next Releases

- **v0.3.0**: Advanced DHCP Features - Lease management, DHCP options, security features
- **v0.4.0**: Enterprise Features - High availability, monitoring, management interface
- **v0.5.0**: Performance Optimization - High-performance optimizations and scalability
- **v1.0.0**: Production Ready - Complete feature set with production-grade stability
