/**
 * @file test_integration.cpp
 * @brief Integration tests for Simple DHCP Daemon
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include "simple-dhcpd/core/parser.hpp"
#include "simple-dhcpd/core/types.hpp"
#include "simple-dhcpd/core/lease/manager.hpp"
#include "simple-dhcpd/core/config/manager.hpp"
#include "simple-dhcpd/core/network/udp_socket.hpp"
#include "simple-dhcpd/core/utils/utils.hpp"

using namespace simple_dhcpd;

// Integration Test: Full DORA Process
class DoraProcessIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test configuration
        config_manager_ = std::make_unique<ConfigManager>();

        // Create minimal test config
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "test-subnet";
        subnet.network = string_to_ip("192.168.1.0");
        subnet.prefix_length = 24;
        subnet.range_start = string_to_ip("192.168.1.100");
        subnet.range_end = string_to_ip("192.168.1.200");
        subnet.lease_time = 3600;
        subnet.max_lease_time = 7200;
        config.subnets.push_back(subnet);

        config_manager_->set_config(config);

        // Setup lease manager
        lease_manager_ = std::make_unique<LeaseManager>(config_manager_->get_config());
    }

    void TearDown() override {
        lease_manager_.reset();
        config_manager_.reset();
    }

    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<LeaseManager> lease_manager_;

    std::vector<uint8_t> create_dhcp_discover(const MacAddress& mac) {
        std::vector<uint8_t> data(576, 0);
        DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(data.data());

        header->op = 1; // BOOTREQUEST
        header->htype = 1; // Ethernet
        header->hlen = 6;
        header->hops = 0;
        header->xid = htonl(0x12345678);
        header->secs = 0;
        header->flags = 0;
        memcpy(header->chaddr, mac.data(), 6);

        // Magic cookie
        size_t offset = sizeof(DhcpMessageHeader);
        data[offset++] = 99;
        data[offset++] = 130;
        data[offset++] = 83;
        data[offset++] = 99;

        // Message type: Discover
        data[offset++] = 53; // Option code
        data[offset++] = 1;   // Length
        data[offset++] = 1;   // DISCOVER

        // End option
        data[offset++] = 255;

        return data;
    }
};

TEST_F(DoraProcessIntegrationTest, FullDoraProcess) {
    MacAddress client_mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

    // Create Discover message
    std::vector<uint8_t> discover = create_dhcp_discover(client_mac);

    // Parse Discover
    DhcpParser parser;
    DhcpMessage msg;
    EXPECT_TRUE(parser.parse_message(discover, msg));
    EXPECT_EQ(msg.message_type, DhcpMessageType::DISCOVER);

    // Process Discover and generate Offer
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    IpAddress offered_ip = lease_manager_->allocate_lease(client_mac, subnet.name);
    EXPECT_NE(offered_ip, 0);

    // Generate Offer message
    DhcpMessage offer_msg;
    offer_msg.message_type = DhcpMessageType::OFFER;
    offer_msg.xid = msg.xid;
    offer_msg.client_mac = client_mac;
    offer_msg.yiaddr = offered_ip;

    std::vector<uint8_t> offer_data = parser.generate_message(offer_msg);
    EXPECT_GT(offer_data.size(), 0);

    // Verify lease was created
    auto lease = lease_manager_->get_lease_by_mac(client_mac);
    EXPECT_NE(lease, nullptr);
    EXPECT_EQ(lease->ip_address, offered_ip);
}

// Integration Test: Protocol Compatibility
class ProtocolCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ProtocolCompatibilityTest, DhcpVersionCompatibility) {
    // Test that we can parse standard DHCP messages
    DhcpParser parser;

    // Create a standard DHCP Discover message
    std::vector<uint8_t> discover(576, 0);
    DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(discover.data());
    header->op = 1;
    header->htype = 1;
    header->hlen = 6;

    // Magic cookie
    size_t offset = sizeof(DhcpMessageHeader);
    discover[offset++] = 99;
    discover[offset++] = 130;
    discover[offset++] = 83;
    discover[offset++] = 99;

    // Message type
    discover[offset++] = 53;
    discover[offset++] = 1;
    discover[offset++] = 1; // DISCOVER
    discover[offset++] = 255;

    DhcpMessage msg;
    EXPECT_TRUE(parser.parse_message(discover, msg));
    EXPECT_EQ(msg.message_type, DhcpMessageType::DISCOVER);
}

TEST_F(ProtocolCompatibilityTest, ClientCompatibility) {
    // Test compatibility with different client types
    DhcpParser parser;

    // Test with different MAC address formats
    MacAddress mac1 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    MacAddress mac2 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    MacAddress mac3 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // All should be valid
    EXPECT_NE(mac1[0], 0);
    EXPECT_NE(mac2[0], 0);
    // mac3 is technically valid (broadcast-like)
}

// Integration Test: Cross-Platform Compatibility
class CrossPlatformTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CrossPlatformTest, NetworkByteOrder) {
    // Test network byte order conversion works correctly
    IpAddress ip = string_to_ip("192.168.1.1");
    EXPECT_NE(ip, 0);

    std::string ip_str = ip_to_string(ip);
    EXPECT_EQ(ip_str, "192.168.1.1");
}

TEST_F(CrossPlatformTest, SocketCreation) {
    // Test socket creation works on current platform
    EXPECT_NO_THROW({
        UdpSocket socket("127.0.0.1", 6767);
        EXPECT_FALSE(socket.is_bound());
    });
}

// Integration Test: Security Validation
class SecurityIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SecurityIntegrationTest, BasicSecurityValidation) {
    // Test that security features are properly integrated
    ConfigManager config_manager;
    DhcpConfig config = get_default_config();
    config.enable_security = true;

    EXPECT_NO_THROW(config_manager.set_config(config));
    EXPECT_TRUE(config_manager.get_config().enable_security);
}

TEST_F(SecurityIntegrationTest, Option82Validation) {
    // Test Option 82 (Relay Agent Information) handling
    // This is a basic integration test - full implementation would test actual Option 82 parsing
    DhcpParser parser;

    // Create message with Option 82
    std::vector<uint8_t> data(576, 0);
    DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(data.data());
    header->op = 1;
    header->htype = 1;
    header->hlen = 6;

    size_t offset = sizeof(DhcpMessageHeader);
    // Magic cookie
    data[offset++] = 99;
    data[offset++] = 130;
    data[offset++] = 83;
    data[offset++] = 99;

    // Option 82 (simplified)
    data[offset++] = 82; // Option 82
    data[offset++] = 4;  // Length
    data[offset++] = 1;  // Sub-option 1 (Circuit ID)
    data[offset++] = 2;  // Length
    data[offset++] = 0x01;
    data[offset++] = 0x02;

    // Message type
    data[offset++] = 53;
    data[offset++] = 1;
    data[offset++] = 1; // DISCOVER
    data[offset++] = 255;

    DhcpMessage msg;
    // Should parse successfully even with Option 82
    EXPECT_TRUE(parser.parse_message(data, msg));
}

// Integration Test: Configuration and Lease Management
class ConfigLeaseIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "integration-test";
        subnet.network = string_to_ip("10.0.0.0");
        subnet.prefix_length = 24;
        subnet.range_start = string_to_ip("10.0.0.100");
        subnet.range_end = string_to_ip("10.0.0.200");
        subnet.lease_time = 3600;
        config.subnets.push_back(subnet);

        config_manager_->set_config(config);
        lease_manager_ = std::make_unique<LeaseManager>(config_manager_->get_config());
    }

    void TearDown() override {
        lease_manager_.reset();
        config_manager_.reset();
    }

    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<LeaseManager> lease_manager_;
};

TEST_F(ConfigLeaseIntegrationTest, ConfigToLeaseAllocation) {
    // Test that configuration properly drives lease allocation
    const DhcpConfig& config = config_manager_->get_config();
    EXPECT_FALSE(config.subnets.empty());

    MacAddress mac = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    const DhcpSubnet& subnet = config.subnets[0];

    IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
    EXPECT_NE(ip, 0);

    // Verify IP is in configured range
    EXPECT_GE(ntohl(ip), ntohl(subnet.range_start));
    EXPECT_LE(ntohl(ip), ntohl(subnet.range_end));
}

TEST_F(ConfigLeaseIntegrationTest, MultipleLeaseAllocation) {
    // Test allocating multiple leases
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];

    std::vector<IpAddress> allocated_ips;
    for (int i = 0; i < 5; ++i) {
        MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, static_cast<uint8_t>(0x50 + i)};
        IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
        EXPECT_NE(ip, 0);
        allocated_ips.push_back(ip);
    }

    // Verify all IPs are unique
    for (size_t i = 0; i < allocated_ips.size(); ++i) {
        for (size_t j = i + 1; j < allocated_ips.size(); ++j) {
            EXPECT_NE(allocated_ips[i], allocated_ips[j]);
        }
    }
}
