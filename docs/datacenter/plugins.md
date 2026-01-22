# Datacenter Version Plugin Architecture

**Version:** Planned
**License:** BSL 1.1

---

## Overview

The Datacenter Version includes a plugin architecture that allows extending functionality with custom plugins.

## Plugin Types

### Option Handler Plugins

Handle custom DHCP options:

```cpp
class CustomOptionHandler : public OptionHandlerPlugin {
public:
    std::string getOptionName() override { return "custom-option"; }
    uint8_t getOptionCode() override { return 200; }
    bool handleOption(const OptionData& data, LeaseContext& context) override;
};
```

### Lease Allocation Plugins

Custom lease allocation strategies:

```cpp
class GeographicAllocationPlugin : public LeaseAllocationPlugin {
public:
    IpAddress allocateLease(const MacAddress& mac, const Subnet& subnet) override;
    std::string getName() override { return "geographic"; }
};
```

### Authentication Plugins

Custom authentication methods:

```cpp
class LDAPAuthPlugin : public AuthenticationPlugin {
public:
    bool authenticate(const AuthRequest& request) override;
    std::string getName() override { return "ldap"; }
};
```

## Plugin Development

### Plugin Structure

```cpp
#include "simple-dhcpd/plugin/plugin.hpp"

class MyPlugin : public Plugin {
public:
    std::string getName() override { return "my-plugin"; }
    std::string getVersion() override { return "1.0.0"; }
    void initialize(PluginContext& context) override;
    void shutdown() override;
};
```

### Plugin Configuration

```json
{
  "dhcp": {
    "datacenter": {
      "plugins": {
        "enabled": true,
        "plugin_directory": "/usr/lib/simple-dhcpd/plugins",
        "plugins": [
          {
            "name": "my-plugin",
            "enabled": true,
            "config": {
              "option1": "value1"
            }
          }
        ]
      }
    }
  }
}
```

## Plugin Examples

### Custom Option Handler

```cpp
class VendorOptionHandler : public OptionHandlerPlugin {
public:
    bool handleOption(const OptionData& data, LeaseContext& context) override {
        // Process vendor-specific option
        return true;
    }
};
```

### Custom Allocation Strategy

```cpp
class RoundRobinAllocation : public LeaseAllocationPlugin {
public:
    IpAddress allocateLease(const MacAddress& mac, const Subnet& subnet) override {
        // Round-robin allocation logic
        return nextIp;
    }
};
```

## Plugin Management

### Load Plugins

```bash
# Load plugin
simple-dhcpd-datacenter --load-plugin my-plugin.so

# List loaded plugins
simple-dhcpd-datacenter --list-plugins

# Unload plugin
simple-dhcpd-datacenter --unload-plugin my-plugin
```

### Plugin API

```bash
# Get plugin info
curl -k https://your-cluster:8081/api/v1/plugins/my-plugin

# Enable plugin
curl -X POST -k https://your-cluster:8081/api/v1/plugins/my-plugin/enable

# Disable plugin
curl -X POST -k https://your-cluster:8081/api/v1/plugins/my-plugin/disable
```

## Related Documentation

- [Plugin API Reference](plugin-api.md) - Complete API documentation
- [Plugin Development Guide](plugin-development.md) - Development guide
- [Plugin Examples](plugin-examples.md) - Example plugins

---

**Last Updated:** December 2024
**Version:** Planned
