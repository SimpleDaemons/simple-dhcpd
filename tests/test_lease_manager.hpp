/**
 * @file test_lease_manager.hpp
 * @brief Tests for DHCP lease management
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_TEST_LEASE_MANAGER_HPP
#define SIMPLE_DHCPD_TEST_LEASE_MANAGER_HPP

#include "dhcp_types.hpp"

class LeaseManagerTest {
public:
    static int run_tests();
    
private:
    static bool test_lease_allocation();
    static bool test_lease_renewal();
    static bool test_lease_release();
    static bool test_ip_range_validation();
    static bool test_lease_conflict_resolution();
    static simple_dhcpd::DhcpConfig create_test_config();
};

#endif // SIMPLE_DHCPD_TEST_LEASE_MANAGER_HPP
