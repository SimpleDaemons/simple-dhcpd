# Code Reorganization Plan - Modular Structure

## Current Structure
```
include/
  - advanced_lease_manager.hpp
  - config.hpp
  - dhcp_options_manager.hpp
  - dhcp_parser.hpp
  - dhcp_security_manager.hpp
  - dhcp_server.hpp
  - dhcp_types.hpp
  - dhcp_utils.hpp
  - lease_manager.hpp
  - logger.hpp
  - udp_socket.hpp

src/
  - advanced_lease_manager.cpp
  - config.cpp
  - dhcp_options_manager.cpp
  - dhcp_parser.cpp
  - dhcp_security_manager.cpp
  - dhcp_server.cpp
  - lease_manager.cpp
  - logger.cpp
  - main.cpp
  - udp_socket.cpp
```

## Proposed Modular Structure

### C++ Layered Architecture (Similar to Java MVC but organized by concerns)

```
include/simple-dhcpd/
  core/              # Core DHCP protocol layer
    - server.hpp      # Main DHCP server orchestrator
    - parser.hpp      # DHCP message parsing/generation
    - types.hpp       # DHCP types and structures
    - exceptions.hpp  # Core exceptions
  
  network/           # Network communication layer
    - udp_socket.hpp # UDP socket abstraction
  
  lease/             # Lease management layer
    - manager.hpp     # Basic lease manager
    - advanced_manager.hpp  # Advanced lease management
    - types.hpp       # Lease-related types
  
  options/           # DHCP options layer
    - manager.hpp     # Options manager
    - types.hpp       # Option types
  
  security/          # Security layer
    - manager.hpp     # Security manager
    - types.hpp       # Security types
  
  config/            # Configuration layer
    - manager.hpp     # Configuration manager
    - types.hpp       # Configuration types
  
  utils/             # Utility layer
    - logger.hpp      # Logging utilities
    - utils.hpp       # General utilities

src/simple-dhcpd/
  core/
    - server.cpp
    - parser.cpp
  
  network/
    - udp_socket.cpp
  
  lease/
    - manager.cpp
    - advanced_manager.cpp
  
  options/
    - manager.cpp
  
  security/
    - manager.cpp
  
  config/
    - manager.cpp
  
  utils/
    - logger.cpp
    - utils.cpp

src/
  - main.cpp         # Application entry point (stays at root)
```

## Benefits

1. **Clear Separation of Concerns**: Each layer has a specific responsibility
2. **Better Namespace Organization**: Matches directory structure
3. **Easier Navigation**: Developers know where to find code
4. **Scalability**: Easy to add new features in appropriate layers
5. **Testability**: Each layer can be tested independently
6. **Maintainability**: Related code is grouped together

## Migration Steps

1. Create new directory structure
2. Move files to new locations
3. Update include paths in all files
4. Update namespace declarations
5. Update CMakeLists.txt
6. Update any documentation
7. Test build

## Namespace Structure

```cpp
namespace simple_dhcpd {
  namespace core { ... }      // Core DHCP protocol
  namespace network { ... }   // Network layer
  namespace lease { ... }    // Lease management
  namespace options { ... }  // DHCP options
  namespace security { ... }  // Security features
  namespace config { ... }   // Configuration
  namespace utils { ... }    // Utilities
}
```

