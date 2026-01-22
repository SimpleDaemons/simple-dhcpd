/**
 * @file test_dhcp_parser.cpp
 * @brief Tests for DHCP message parsing and generation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <iostream>
#include <cassert>
#include <vector>
#include <cstring>
#include "simple-dhcpd/core/parser.hpp"
#include "simple-dhcpd/core/types.hpp"
#include "simple-dhcpd/core/utils/utils.hpp"

using namespace simple_dhcpd;

class DhcpParserTest {
public:
    static int run_tests() {
        int passed = 0;
        int failed = 0;
        
        std::cout << "Running DHCP Parser Tests..." << std::endl;
        std::cout << "=============================" << std::endl;
        
        // Test DHCP message parsing
        if (test_parse_dhcp_discover()) {
            std::cout << "✓ DHCP Discover parsing test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ DHCP Discover parsing test failed" << std::endl;
            failed++;
        }
        
        if (test_parse_dhcp_request()) {
            std::cout << "✓ DHCP Request parsing test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ DHCP Request parsing test failed" << std::endl;
            failed++;
        }
        
        // Test DHCP message generation
        if (test_generate_dhcp_offer()) {
            std::cout << "✓ DHCP Offer generation test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ DHCP Offer generation test failed" << std::endl;
            failed++;
        }
        
        if (test_generate_dhcp_ack()) {
            std::cout << "✓ DHCP ACK generation test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ DHCP ACK generation test failed" << std::endl;
            failed++;
        }
        
        // Test message validation
        if (test_message_validation()) {
            std::cout << "✓ Message validation test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Message validation test failed" << std::endl;
            failed++;
        }
        
        // Test option handling
        if (test_option_handling()) {
            std::cout << "✓ Option handling test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Option handling test failed" << std::endl;
            failed++;
        }
        
        std::cout << "=============================" << std::endl;
        std::cout << "DHCP Parser Tests completed: " << passed << " passed, " << failed << " failed" << std::endl;
        
        return failed == 0 ? 0 : 1;
    }
    
private:
    static bool test_parse_dhcp_discover() {
        try {
            // Create a mock DHCP Discover message
            std::vector<uint8_t> data = create_mock_dhcp_discover();
            
            // Parse the message
            DhcpMessage message = DhcpParser::parse_message(data);
            
            // Verify it's a DISCOVER message
            if (message.message_type != DhcpMessageType::DISCOVER) {
                return false;
            }
            
            // Verify header fields
            if (message.header.op != 1) { // BOOTREQUEST
                return false;
            }
            
            if (message.header.htype != 1) { // Ethernet
                return false;
            }
            
            if (message.header.hlen != 6) { // MAC address length
                return false;
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_parse_dhcp_request() {
        try {
            // Create a mock DHCP Request message
            std::vector<uint8_t> data = create_mock_dhcp_request();
            
            // Parse the message
            DhcpMessage message = DhcpParser::parse_message(data);
            
            // Verify it's a REQUEST message
            if (message.message_type != DhcpMessageType::REQUEST) {
                return false;
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_generate_dhcp_offer() {
        try {
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
            
            if (parsed.message_type != DhcpMessageType::OFFER) {
                return false;
            }
            
            if (parsed.header.xid != 0x12345678) {
                return false;
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_generate_dhcp_ack() {
        try {
            // Create a DHCP ACK message using the builder
            DhcpMessageBuilder builder;
            builder.set_message_type(DhcpMessageType::ACK)
                   .set_transaction_id(0x87654321)
                   .set_client_mac({0x00, 0x11, 0x22, 0x33, 0x44, 0x55})
                   .set_your_ip(string_to_ip("192.168.1.100"))
                   .set_server_ip(string_to_ip("192.168.1.1"));
            
            DhcpMessage ack = builder.build();
            
            // Generate the message
            std::vector<uint8_t> data = DhcpParser::generate_message(ack);
            
            // Verify the generated message can be parsed back
            DhcpMessage parsed = DhcpParser::parse_message(data);
            
            if (parsed.message_type != DhcpMessageType::ACK) {
                return false;
            }
            
            if (parsed.header.xid != 0x87654321) {
                return false;
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_message_validation() {
        try {
            // Test valid message
            DhcpMessageBuilder builder;
            builder.set_message_type(DhcpMessageType::OFFER)
                   .set_transaction_id(0x12345678)
                   .set_client_mac({0x00, 0x11, 0x22, 0x33, 0x44, 0x55})
                   .set_your_ip(string_to_ip("192.168.1.100"))
                   .set_server_ip(string_to_ip("192.168.1.1"));
            
            DhcpMessage message = builder.build();
            
            if (!DhcpParser::validate_message(message)) {
                return false;
            }
            
            // Test invalid message (wrong op code)
            message.header.op = 0; // Invalid op code
            if (DhcpParser::validate_message(message)) {
                return false; // Should be invalid
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_option_handling() {
        try {
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
            if (!subnet_mask) {
                return false;
            }
            
            if (subnet_mask->length != 4) {
                return false;
            }
            
            // Find the router option
            const DhcpOption* router = DhcpParser::find_option(message.options, DhcpOptionCode::ROUTER);
            if (!router) {
                return false;
            }
            
            if (router->length != 4) {
                return false;
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static std::vector<uint8_t> create_mock_dhcp_discover() {
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
        
        return data;
    }
    
    static std::vector<uint8_t> create_mock_dhcp_request() {
        std::vector<uint8_t> data(576, 0);
        
        // Set up DHCP header
        DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(data.data());
        header->op = 1; // BOOTREQUEST
        header->htype = 1; // Ethernet
        header->hlen = 6; // MAC address length
        header->hops = 0;
        header->xid = htonl(0x87654321);
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
        
        // Message type option (REQUEST)
        data[offset++] = static_cast<uint8_t>(DhcpOptionCode::DHCP_MESSAGE_TYPE);
        data[offset++] = 1;
        data[offset++] = static_cast<uint8_t>(DhcpMessageType::REQUEST);
        
        // Client identifier option
        data[offset++] = static_cast<uint8_t>(DhcpOptionCode::CLIENT_IDENTIFIER);
        data[offset++] = 7;
        data[offset++] = 1; // Hardware type
        memcpy(data.data() + offset, mac, 6);
        offset += 6;
        
        // Requested IP address
        data[offset++] = static_cast<uint8_t>(DhcpOptionCode::REQUESTED_IP_ADDRESS);
        data[offset++] = 4;
        uint32_t requested_ip = string_to_ip("192.168.1.100");
        memcpy(data.data() + offset, &requested_ip, 4);
        offset += 4;
        
        // END option
        data[offset++] = static_cast<uint8_t>(DhcpOptionCode::END);
        
        return data;
    }
};

// Main function removed - using Google Test framework
