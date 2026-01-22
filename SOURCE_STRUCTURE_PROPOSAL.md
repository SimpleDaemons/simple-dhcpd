# Source Code Structure Proposal

## Current Structure

```
simple-dhcpd/
├── include/simple-dhcpd/
│   ├── core/          # Core DHCP functionality
│   ├── lease/         # Lease management
│   ├── security/      # Security features
│   ├── options/       # DHCP options
│   ├── config/        # Configuration
│   └── network/       # Network layer
├── src/simple-dhcpd/  # Implementation (mirrors include)
└── CMakeLists.txt     # Single build configuration
```

**Issues:**
- All features compiled into single binary
- No clear separation between Production/Enterprise/Datacenter
- Difficult to build version-specific binaries
- All code included regardless of version

## Proposed Structure

### Option 1: Modular Directory Structure (Recommended)

```
simple-dhcpd/
├── src/
│   ├── core/                    # Shared core (Production base)
│   │   ├── dhcp/
│   │   ├── lease/
│   │   ├── network/
│   │   ├── config/
│   │   └── utils/
│   ├── production/              # Production-specific features
│   │   ├── security/           # Basic security (Option 82, etc.)
│   │   └── features/            # Production-only features
│   ├── enterprise/              # Enterprise-specific features
│   │   ├── ha/                  # High availability
│   │   ├── management/          # Web UI, REST API
│   │   ├── integrations/       # DNS, LDAP, SNMP
│   │   └── security/            # Advanced security
│   └── datacenter/              # Datacenter-specific features
│       ├── cluster/             # Clustering
│       ├── multi-site/          # Multi-site sync
│       ├── plugins/             # Plugin system
│       └── cloud/                # Cloud integrations
├── include/
│   ├── simple-dhcpd/
│   │   ├── core/                # Core headers
│   │   ├── production/          # Production headers
│   │   ├── enterprise/          # Enterprise headers
│   │   └── datacenter/          # Datacenter headers
│   └── simple-dhcpd-common/     # Shared headers
├── main/
│   ├── production.cpp           # Production main
│   ├── enterprise.cpp           # Enterprise main
│   └── datacenter.cpp           # Datacenter main
└── CMakeLists.txt               # Version-aware build
```

**Benefits:**
- Clear separation of concerns
- Easy to build version-specific binaries
- Code reuse through shared core
- Maintainable and scalable

### Option 2: Feature Flags (Alternative)

Keep current structure but use compile-time flags:

```cmake
option(BUILD_PRODUCTION "Build Production version" ON)
option(BUILD_ENTERPRISE "Build Enterprise version" OFF)
option(BUILD_DATACENTER "Build Datacenter version" OFF)
```

**Benefits:**
- Minimal restructuring
- Single codebase
- Conditional compilation

**Drawbacks:**
- Code becomes cluttered with #ifdef
- Harder to maintain
- All code always present

## Recommended Approach: Option 1 (Modular)

### Implementation Plan

1. **Phase 1: Reorganize Core**
   - Move shared code to `src/core/`
   - Update includes
   - Ensure Production builds

2. **Phase 2: Extract Production Features**
   - Move Production-specific code to `src/production/`
   - Update build system
   - Test Production build

3. **Phase 3: Add Enterprise Structure**
   - Create `src/enterprise/` structure
   - Add Enterprise build option
   - Implement Enterprise features incrementally

4. **Phase 4: Add Datacenter Structure**
   - Create `src/datacenter/` structure
   - Add Datacenter build option
   - Implement Datacenter features incrementally

### Build System Changes

```cmake
# Version selection
set(BUILD_VERSION "production" CACHE STRING "Version to build: production, enterprise, datacenter")
set_property(CACHE BUILD_VERSION PROPERTY STRINGS production enterprise datacenter)

# Core sources (always included)
set(CORE_SOURCES
    src/core/dhcp/parser.cpp
    src/core/dhcp/server.cpp
    src/core/lease/manager.cpp
    src/core/network/udp_socket.cpp
    # ... all core sources
)

# Version-specific sources
if(BUILD_VERSION STREQUAL "production")
    set(VERSION_SOURCES
        src/production/security/basic_security.cpp
        # ... production sources
    )
    set(VERSION_MAIN src/main/production.cpp)
elseif(BUILD_VERSION STREQUAL "enterprise")
    set(VERSION_SOURCES
        ${PRODUCTION_SOURCES}  # Include production
        src/enterprise/ha/failover.cpp
        src/enterprise/management/web_ui.cpp
        # ... enterprise sources
    )
    set(VERSION_MAIN src/main/enterprise.cpp)
elseif(BUILD_VERSION STREQUAL "datacenter")
    set(VERSION_SOURCES
        ${ENTERPRISE_SOURCES}  # Include enterprise
        src/datacenter/cluster/cluster_manager.cpp
        src/datacenter/multi-site/sync.cpp
        # ... datacenter sources
    )
    set(VERSION_MAIN src/main/datacenter.cpp)
endif()

# Build executable
add_executable(${PROJECT_NAME} ${CORE_SOURCES} ${VERSION_SOURCES} ${VERSION_MAIN})
```

### Directory Structure Details

#### Core (`src/core/`)
- **Purpose:** Shared functionality used by all versions
- **Contents:**
  - DHCP protocol implementation
  - Basic lease management
  - Network layer
  - Configuration parsing
  - Utilities

#### Production (`src/production/`)
- **Purpose:** Production-specific features
- **Contents:**
  - Basic security (Option 82, rate limiting)
  - Production logging
  - Production configuration

#### Enterprise (`src/enterprise/`)
- **Purpose:** Enterprise features building on Production
- **Contents:**
  - High availability (failover)
  - Management interfaces (Web UI, REST API)
  - Integrations (DNS, LDAP, SNMP)
  - Advanced security

#### Datacenter (`src/datacenter/`)
- **Purpose:** Datacenter features building on Enterprise
- **Contents:**
  - Clustering and horizontal scaling
  - Multi-site synchronization
  - Plugin architecture
  - Cloud integrations

### Build Commands

```bash
# Build Production version
cmake -DBUILD_VERSION=production ..
make

# Build Enterprise version
cmake -DBUILD_VERSION=enterprise ..
make

# Build Datacenter version
cmake -DBUILD_VERSION=datacenter ..
make
```

### Package Naming

```bash
# Production
simple-dhcpd-production-1.0.0-linux-amd64.deb

# Enterprise
simple-dhcpd-enterprise-1.0.0-linux-amd64.deb

# Datacenter
simple-dhcpd-datacenter-1.0.0-linux-amd64.deb
```

## Migration Strategy

### Step 1: Create New Structure (Non-Breaking)
1. Create new directories
2. Copy files to new locations
3. Update includes
4. Test builds

### Step 2: Update Build System
1. Modify CMakeLists.txt
2. Add version selection
3. Test all versions build

### Step 3: Clean Up
1. Remove old structure
2. Update documentation
3. Update CI/CD

## Benefits

1. **Clear Separation:** Each version has its own code
2. **Code Reuse:** Shared core reduces duplication
3. **Maintainability:** Easier to understand and modify
4. **Build Flexibility:** Build only what you need
5. **Scalability:** Easy to add new features per version
6. **Licensing:** Clear code boundaries for licensing

## Considerations

1. **Initial Effort:** Requires restructuring
2. **Testing:** Need to test all versions
3. **Documentation:** Update build docs
4. **CI/CD:** Update build pipelines

## Recommendation

**Proceed with Option 1 (Modular Directory Structure)**

This provides the best long-term maintainability and clear separation between versions while maximizing code reuse.

---

**Status:** Proposal  
**Next Steps:** Review and approval before implementation
