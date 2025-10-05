/**
 * @file test_basic.hpp
 * @brief Basic test framework for Simple DHCP Daemon
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_TEST_BASIC_HPP
#define SIMPLE_DHCPD_TEST_BASIC_HPP

class BasicTest {
public:
    static int run_tests();
    
private:
    static bool test_dhcp_message_parsing();
    static bool test_lease_management();
    static bool test_configuration_parsing();
};

#endif // SIMPLE_DHCPD_TEST_BASIC_HPP
