/**
 * @file test_dhcp_parser.hpp
 * @brief Tests for DHCP message parsing and generation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_TEST_DHCP_PARSER_HPP
#define SIMPLE_DHCPD_TEST_DHCP_PARSER_HPP

class DhcpParserTest {
public:
    static int run_tests();
    
private:
    static bool test_parse_dhcp_discover();
    static bool test_parse_dhcp_request();
    static bool test_generate_dhcp_offer();
    static bool test_generate_dhcp_ack();
    static bool test_message_validation();
    static bool test_option_handling();
    static std::vector<uint8_t> create_mock_dhcp_discover();
    static std::vector<uint8_t> create_mock_dhcp_request();
};

#endif // SIMPLE_DHCPD_TEST_DHCP_PARSER_HPP

