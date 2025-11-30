/**
 * @file test_simple_dhcpd.cpp
 * @brief Google Test tests for Simple DHCP Daemon
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include "simple-dhcpd/core/parser.hpp"
#include "simple-dhcpd/core/types.hpp"
#include "simple-dhcpd/utils/utils.hpp"
#include "simple-dhcpd/lease/manager.hpp"
#include "simple-dhcpd/config/manager.hpp"

using namespace simple_dhcpd;

// Helper function for creating mock DHCP messages
std::vector<uint8_t> create_mock_dhcp_discover() {
    std::vector<uint8_t> data(576, 0); // Standard DHCP message size
    
    // Set up DHCP header
    DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(data.data());
    header->op = 1; // BOOTREQUEST
    header->htype = 1; // Ethernet
    header->hlen = 6; // MAC address length
    header->hops = 0;
    header->xid = htonl(0x12345678);
    header->secs = 0;
    header->flags = 0;
    header->ciaddr = 0;
    header->yiaddr = 0;
    header->siaddr = 0;
    header->giaddr = 0;
    
    // Set MAC address
    uint8_t mac[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    memcpy(header->chaddr, mac, 6);
    
    // Add DHCP options
    size_t offset = sizeof(DhcpMessageHeader);
    
    // Magic cookie
    data[offset++] = 99;
    data[offset++] = 130;
    data[offset++] = 83;
    data[offset++] = 99;
    
    // Message type option (DISCOVER)
    data[offset++] = static_cast<uint8_t>(DhcpOptionCode::DHCP_MESSAGE_TYPE);
    data[offset++] = 1;
    data[offset++] = static_cast<uint8_t>(DhcpMessageType::DISCOVER);
    
    // Client identifier option
    data[offset++] = static_cast<uint8_t>(DhcpOptionCode::CLIENT_IDENTIFIER);
    data[offset++] = 7;
    data[offset++] = 1; // Hardware type
    memcpy(data.data() + offset, mac, 6);
    offset += 6;
    
    // END option
    data[offset++] = static_cast<uint8_t>(DhcpOptionCode::END);
    
    // Debug: Print the constructed message
    std::cout << "Constructed DHCP message size: " << offset << std::endl;
    std::cout << "Options start at offset: " << sizeof(DhcpMessageHeader) << std::endl;
    
    return data;
}

// Test DHCP Parser
class DhcpParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
    }
    
    void TearDown() override {
        // Clean up
    }
};

TEST_F(DhcpParserTest, BasicMessageParsing) {
    // Create a mock DHCP Discover message
    std::vector<uint8_t> data = create_mock_dhcp_discover();
    
    // Debug: Print the raw data
    std::cout << "Raw DHCP data size: " << data.size() << std::endl;
    std::cout << "First 64 bytes: ";
    for (size_t i = 0; i < std::min(data.size(), size_t(64)); ++i) {
        printf("%02x ", data[i]);
    }
    std::cout << std::endl;
    
    // Parse the message
    DhcpMessage message = DhcpParser::parse_message(data);
    
    // Verify it's a DISCOVER message
    EXPECT_EQ(message.message_type, DhcpMessageType::DISCOVER);
    
    // Verify header fields
    EXPECT_EQ(message.header.op, 1); // BOOTREQUEST
    EXPECT_EQ(message.header.htype, 1); // Ethernet
    EXPECT_EQ(message.header.hlen, 6); // MAC address length
}

TEST_F(DhcpParserTest, MessageGeneration) {
    // Create a DHCP Offer message using the builder
    DhcpMessageBuilder builder;
    builder.set_message_type(DhcpMessageType::OFFER)
           .set_transaction_id(0x12345678)
           .set_client_mac({0x00, 0x11, 0x22, 0x33, 0x44, 0x55})
           .set_your_ip(string_to_ip("192.168.1.100"))
           .set_server_ip(string_to_ip("192.168.1.1"));
    
    DhcpMessage offer = builder.build();
    
    // Generate the message
    std::vector<uint8_t> data = DhcpParser::generate_message(offer);
    
    // Verify the generated message can be parsed back
    DhcpMessage parsed = DhcpParser::parse_message(data);
    
    EXPECT_EQ(parsed.message_type, DhcpMessageType::OFFER);
    EXPECT_EQ(parsed.header.xid, 0x12345678);
}

TEST_F(DhcpParserTest, MessageValidation) {
    // Test valid message
    DhcpMessageBuilder builder;
    builder.set_message_type(DhcpMessageType::OFFER)
           .set_transaction_id(0x12345678)
           .set_client_mac({0x00, 0x11, 0x22, 0x33, 0x44, 0x55})
           .set_your_ip(string_to_ip("192.168.1.100"))
           .set_server_ip(string_to_ip("192.168.1.1"));
    
    DhcpMessage message = builder.build();
    
    EXPECT_TRUE(DhcpParser::validate_message(message));
    
    // Test invalid message (wrong op code)
    message.header.op = 0; // Invalid op code
    EXPECT_FALSE(DhcpParser::validate_message(message));
}

TEST_F(DhcpParserTest, OptionHandling) {
    // Test adding and finding options
    DhcpMessageBuilder builder;
    builder.set_message_type(DhcpMessageType::OFFER)
           .set_transaction_id(0x12345678)
           .set_client_mac({0x00, 0x11, 0x22, 0x33, 0x44, 0x55})
           .set_your_ip(string_to_ip("192.168.1.100"))
           .set_server_ip(string_to_ip("192.168.1.1"))
           .add_option_ip(DhcpOptionCode::SUBNET_MASK, string_to_ip("255.255.255.0"))
           .add_option_ip(DhcpOptionCode::ROUTER, string_to_ip("192.168.1.1"));
    
    DhcpMessage message = builder.build();
    
    // Find the subnet mask option
    const DhcpOption* subnet_mask = DhcpParser::find_option(message.options, DhcpOptionCode::SUBNET_MASK);
    ASSERT_NE(subnet_mask, nullptr);
    EXPECT_EQ(subnet_mask->length, 4);
    
    // Find the router option
    const DhcpOption* router = DhcpParser::find_option(message.options, DhcpOptionCode::ROUTER);
    ASSERT_NE(router, nullptr);
    EXPECT_EQ(router->length, 4);
}

// Test Lease Manager
class LeaseManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = create_test_config();
        manager = std::make_unique<LeaseManager>(config);
        manager->start();
    }
    
    void TearDown() override {
        if (manager) {
            manager->stop();
        }
    }
    
    DhcpConfig create_test_config() {
        DhcpConfig config;
        config.enable_logging = false; // Disable logging for tests
        
        // Create a test subnet
        DhcpSubnet subnet;
        subnet.name = "test-subnet";
        subnet.network = string_to_ip("192.168.1.0");
        subnet.range_start = string_to_ip("192.168.1.100");
        subnet.range_end = string_to_ip("192.168.1.200");
        subnet.gateway = string_to_ip("192.168.1.1");
        subnet.lease_time = 3600; // 1 hour
        subnet.max_lease_time = 7200; // 2 hours
        
        config.subnets.push_back(subnet);
        
        return config;
    }
    
    DhcpConfig config;
    std::unique_ptr<LeaseManager> manager;
};

TEST_F(LeaseManagerTest, LeaseAllocation) {
    // Test MAC address
    MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    
    // Allocate a lease
    DhcpLease lease = manager->allocate_lease(mac, 0, "test-subnet");
    
    // Verify lease properties
    EXPECT_EQ(lease.mac_address, mac);
    EXPECT_NE(lease.ip_address, 0);
    EXPECT_TRUE(lease.is_active);
    EXPECT_FALSE(lease.is_static); // Should be dynamic
    
    // Verify the lease can be retrieved
    auto retrieved_lease = manager->get_lease_by_mac(mac);
    ASSERT_NE(retrieved_lease, nullptr);
    EXPECT_EQ(retrieved_lease->ip_address, lease.ip_address);
}

TEST_F(LeaseManagerTest, LeaseRenewal) {
    // Test MAC address
    MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    
    // Allocate a lease
    DhcpLease lease = manager->allocate_lease(mac, 0, "test-subnet");
    IpAddress original_ip = lease.ip_address;
    
    // Renew the lease
    DhcpLease renewed_lease = manager->renew_lease(mac, original_ip);
    
    // Verify renewal properties
    EXPECT_EQ(renewed_lease.mac_address, mac);
    EXPECT_EQ(renewed_lease.ip_address, original_ip);
    EXPECT_TRUE(renewed_lease.is_active);
    
    // Verify the lease time was updated
    EXPECT_GT(renewed_lease.lease_start, lease.lease_start);
}

TEST_F(LeaseManagerTest, LeaseRelease) {
    // Test MAC address
    MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    
    // Allocate a lease
    DhcpLease lease = manager->allocate_lease(mac, 0, "test-subnet");
    IpAddress allocated_ip = lease.ip_address;
    
    // Verify lease exists
    auto existing_lease = manager->get_lease_by_mac(mac);
    ASSERT_NE(existing_lease, nullptr);
    
    // Release the lease
    bool released = manager->release_lease(mac, allocated_ip);
    EXPECT_TRUE(released);
    
    // Verify lease is no longer active
    auto released_lease = manager->get_lease_by_mac(mac);
    if (released_lease) {
        EXPECT_FALSE(released_lease->is_active);
    }
}
