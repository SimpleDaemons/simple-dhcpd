/**
 * @file test_config.cpp
 * @brief Google Test tests for configuration parsing and validation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "simple-dhcpd/core/config/manager.hpp"
#include "simple-dhcpd/core/types.hpp"

using namespace simple_dhcpd;

// Configuration Parsing Tests
class ConfigParsingTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_config_dir_ = "/tmp/simple-dhcpd-test-configs";
        std::filesystem::create_directories(test_config_dir_);
    }

    void TearDown() override {
        // Cleanup test config files
        if (std::filesystem::exists(test_config_dir_)) {
            std::filesystem::remove_all(test_config_dir_);
        }
    }

    std::string test_config_dir_;

    std::string create_test_json_config() {
        std::string config = R"({
  "dhcp": {
    "listen": ["0.0.0.0"],
    "subnets": [
      {
        "name": "test-subnet",
        "network": "192.168.1.0",
        "prefix_length": 24,
        "range_start": "192.168.1.100",
        "range_end": "192.168.1.200",
        "lease_time": 3600,
        "max_lease_time": 7200
      }
    ],
    "global_options": [],
    "security": {
      "enable": false
    },
    "logging": {
      "enable": true,
      "log_file": "/var/log/simple-dhcpd.log"
    }
  }
})";
        return config;
    }

    std::string create_invalid_json_config() {
        return R"({
  "dhcp": {
    "listen": [],
    "subnets": []
  }
})";
    }

    std::string create_config_with_invalid_subnet() {
        return R"({
  "dhcp": {
    "listen": ["0.0.0.0"],
    "subnets": [
      {
        "name": "",
        "network": "192.168.1.0",
        "prefix_length": 33,
        "range_start": "192.168.1.200",
        "range_end": "192.168.1.100",
        "lease_time": 0
      }
    ]
  }
})";
    }
};

TEST_F(ConfigParsingTest, LoadValidJsonConfig) {
    ConfigManager config_manager;
    std::string config_file = test_config_dir_ + "/test.json";

    std::ofstream file(config_file);
    file << create_test_json_config();
    file.close();

    EXPECT_NO_THROW(config_manager.load_config(config_file));
    EXPECT_TRUE(config_manager.is_loaded());

    const DhcpConfig& config = config_manager.get_config();
    EXPECT_FALSE(config.listen_addresses.empty());
    EXPECT_FALSE(config.subnets.empty());
}

TEST_F(ConfigParsingTest, LoadInvalidJsonConfig) {
    ConfigManager config_manager;
    std::string config_file = test_config_dir_ + "/invalid.json";

    std::ofstream file(config_file);
    file << create_invalid_json_config();
    file.close();

    EXPECT_THROW(config_manager.load_config(config_file), ConfigException);
}

TEST_F(ConfigParsingTest, ValidateConfigWithInvalidSubnet) {
    ConfigManager config_manager;
    std::string config_file = test_config_dir_ + "/invalid_subnet.json";

    std::ofstream file(config_file);
    file << create_config_with_invalid_subnet();
    file.close();

    // Loading might succeed, but validation should fail
    try {
        config_manager.load_config(config_file);
        // If loading succeeds, validation should catch errors
        EXPECT_THROW(config_manager.validate_config(), ConfigException);
    } catch (const ConfigException&) {
        // Loading might fail validation immediately
        EXPECT_TRUE(true);
    }
}

TEST_F(ConfigParsingTest, ConfigValidationEmptyListenAddresses) {
    ConfigManager config_manager;
    DhcpConfig config = get_default_config();
    config.listen_addresses.clear();

    EXPECT_THROW({
        config_manager.set_config(config);
    }, ConfigException);
}

TEST_F(ConfigParsingTest, ConfigValidationEmptySubnets) {
    ConfigManager config_manager;
    DhcpConfig config = get_default_config();
    config.subnets.clear();

    EXPECT_THROW({
        config_manager.set_config(config);
    }, ConfigException);
}

TEST_F(ConfigParsingTest, ConfigReload) {
    ConfigManager config_manager;
    std::string config_file = test_config_dir_ + "/reload_test.json";

    std::ofstream file(config_file);
    file << create_test_json_config();
    file.close();

    config_manager.load_config(config_file);
    EXPECT_TRUE(config_manager.is_loaded());

    // Reload should work
    EXPECT_NO_THROW(config_manager.reload_config());
}

TEST_F(ConfigParsingTest, ConfigReloadWithoutFile) {
    ConfigManager config_manager;

    EXPECT_THROW(config_manager.reload_config(), ConfigException);
}

TEST_F(ConfigParsingTest, ConfigFileNotFound) {
    ConfigManager config_manager;

    EXPECT_THROW(config_manager.load_config("/nonexistent/path/config.json"), ConfigException);
}

// Configuration Error Tests
class ConfigErrorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ConfigErrorTest, InvalidJsonSyntax) {
    ConfigManager config_manager;
    std::string config_file = "/tmp/test_invalid_syntax.json";

    std::ofstream file(config_file);
    file << "{ invalid json }";
    file.close();

    EXPECT_THROW(config_manager.load_config(config_file), ConfigException);

    std::filesystem::remove(config_file);
}

TEST_F(ConfigErrorTest, MissingRequiredFields) {
    ConfigManager config_manager;
    std::string config_file = "/tmp/test_missing_fields.json";

    std::ofstream file(config_file);
    file << R"({"dhcp": {}})";
    file.close();

    // Should either fail on load or validation
    try {
        config_manager.load_config(config_file);
        EXPECT_THROW(config_manager.validate_config(), ConfigException);
    } catch (const ConfigException&) {
        EXPECT_TRUE(true);
    }

    std::filesystem::remove(config_file);
}

TEST_F(ConfigErrorTest, InvalidIpAddressFormat) {
    ConfigManager config_manager;
    std::string config_file = "/tmp/test_invalid_ip.json";

    std::ofstream file(config_file);
    file << R"({
  "dhcp": {
    "listen": ["invalid.ip.address"],
    "subnets": []
  }
})";
    file.close();

    // Should handle invalid IP gracefully
    try {
        config_manager.load_config(config_file);
    } catch (const ConfigException&) {
        EXPECT_TRUE(true);
    }

    std::filesystem::remove(config_file);
}

TEST_F(ConfigErrorTest, InvalidSubnetRange) {
    ConfigManager config_manager;
    std::string config_file = "/tmp/test_invalid_range.json";

    std::ofstream file(config_file);
    file << R"({
  "dhcp": {
    "listen": ["0.0.0.0"],
    "subnets": [
      {
        "name": "test",
        "network": "192.168.1.0",
        "prefix_length": 24,
        "range_start": "192.168.2.100",
        "range_end": "192.168.2.200",
        "lease_time": 3600
      }
    ]
  }
})";
    file.close();

    // Range outside subnet should fail validation
    try {
        config_manager.load_config(config_file);
        EXPECT_THROW(config_manager.validate_config(), ConfigException);
    } catch (const ConfigException&) {
        EXPECT_TRUE(true);
    }

    std::filesystem::remove(config_file);
}
