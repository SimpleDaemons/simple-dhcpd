/**
 * @file test_lease_manager.cpp
 * @brief Tests for DHCP lease management
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <iostream>
#include <cassert>
#include <vector>
#include "lease_manager.hpp"
#include "dhcp_types.hpp"
#include "dhcp_utils.hpp"

using namespace simple_dhcpd;

class LeaseManagerTest {
public:
    static int run_tests() {
        int passed = 0;
        int failed = 0;
        
        std::cout << "Running Lease Manager Tests..." << std::endl;
        std::cout << "=============================" << std::endl;
        
        // Test lease allocation
        if (test_lease_allocation()) {
            std::cout << "✓ Lease allocation test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Lease allocation test failed" << std::endl;
            failed++;
        }
        
        // Test lease renewal
        if (test_lease_renewal()) {
            std::cout << "✓ Lease renewal test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Lease renewal test failed" << std::endl;
            failed++;
        }
        
        // Test lease release
        if (test_lease_release()) {
            std::cout << "✓ Lease release test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Lease release test failed" << std::endl;
            failed++;
        }
        
        // Test IP range validation
        if (test_ip_range_validation()) {
            std::cout << "✓ IP range validation test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ IP range validation test failed" << std::endl;
            failed++;
        }
        
        // Test lease conflict resolution
        if (test_lease_conflict_resolution()) {
            std::cout << "✓ Lease conflict resolution test passed" << std::endl;
            passed++;
        } else {
            std::cout << "✗ Lease conflict resolution test failed" << std::endl;
            failed++;
        }
        
        std::cout << "=============================" << std::endl;
        std::cout << "Lease Manager Tests completed: " << passed << " passed, " << failed << " failed" << std::endl;
        
        return failed == 0 ? 0 : 1;
    }
    
private:
    static bool test_lease_allocation() {
        try {
            // Create a test configuration
            DhcpConfig config = create_test_config();
            LeaseManager manager(config);
            manager.start();
            
            // Test MAC address
            MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            
            // Allocate a lease
            DhcpLease lease = manager.allocate_lease(mac, 0, "test-subnet");
            
            // Verify lease properties
            if (lease.mac_address != mac) {
                return false;
            }
            
            if (lease.ip_address == 0) {
                return false;
            }
            
            if (!lease.is_active) {
                return false;
            }
            
            if (lease.is_static) {
                return false; // Should be dynamic
            }
            
            // Verify the lease can be retrieved
            auto retrieved_lease = manager.get_lease_by_mac(mac);
            if (!retrieved_lease) {
                return false;
            }
            
            if (retrieved_lease->ip_address != lease.ip_address) {
                return false;
            }
            
            manager.stop();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_lease_renewal() {
        try {
            // Create a test configuration
            DhcpConfig config = create_test_config();
            LeaseManager manager(config);
            manager.start();
            
            // Test MAC address
            MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            
            // Allocate a lease
            DhcpLease lease = manager.allocate_lease(mac, 0, "test-subnet");
            IpAddress original_ip = lease.ip_address;
            
            // Renew the lease
            DhcpLease renewed_lease = manager.renew_lease(mac, original_ip);
            
            // Verify renewal properties
            if (renewed_lease.mac_address != mac) {
                return false;
            }
            
            if (renewed_lease.ip_address != original_ip) {
                return false;
            }
            
            if (!renewed_lease.is_active) {
                return false;
            }
            
            // Verify the lease time was updated
            if (renewed_lease.lease_start <= lease.lease_start) {
                return false;
            }
            
            manager.stop();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_lease_release() {
        try {
            // Create a test configuration
            DhcpConfig config = create_test_config();
            LeaseManager manager(config);
            manager.start();
            
            // Test MAC address
            MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            
            // Allocate a lease
            DhcpLease lease = manager.allocate_lease(mac, 0, "test-subnet");
            IpAddress allocated_ip = lease.ip_address;
            
            // Verify lease exists
            auto existing_lease = manager.get_lease_by_mac(mac);
            if (!existing_lease) {
                return false;
            }
            
            // Release the lease
            bool released = manager.release_lease(mac, allocated_ip);
            if (!released) {
                return false;
            }
            
            // Verify lease is no longer active
            auto released_lease = manager.get_lease_by_mac(mac);
            if (released_lease && released_lease->is_active) {
                return false;
            }
            
            manager.stop();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_ip_range_validation() {
        try {
            // Create a test configuration
            DhcpConfig config = create_test_config();
            LeaseManager manager(config);
            manager.start();
            
            // Test MAC address
            MacAddress mac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            
            // Allocate multiple leases and verify they're in range
            for (int i = 0; i < 5; ++i) {
                DhcpLease lease = manager.allocate_lease(mac, 0, "test-subnet");
                
                // Verify IP is in the configured range
                if (ntohl(lease.ip_address) < ntohl(config.subnets[0].range_start) ||
                    ntohl(lease.ip_address) > ntohl(config.subnets[0].range_end)) {
                    return false;
                }
                
                // Use a different MAC for next iteration
                mac[5] = static_cast<uint8_t>(0x55 + i);
            }
            
            manager.stop();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool test_lease_conflict_resolution() {
        try {
            // Create a test configuration
            DhcpConfig config = create_test_config();
            LeaseManager manager(config);
            manager.start();
            
            // Test MAC addresses
            MacAddress mac1 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            MacAddress mac2 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x56};
            
            // Allocate first lease
            DhcpLease lease1 = manager.allocate_lease(mac1, 0, "test-subnet");
            IpAddress allocated_ip = lease1.ip_address;
            
            // Try to allocate the same IP to a different MAC
            // This should either fail or allocate a different IP
            try {
                DhcpLease lease2 = manager.allocate_lease(mac2, allocated_ip, "test-subnet");
                
                // If it succeeds, the IPs should be different
                if (lease2.ip_address == allocated_ip) {
                    return false; // Conflict not resolved
                }
            } catch (const LeaseManagerException&) {
                // This is also acceptable - the manager rejected the conflicting allocation
            }
            
            manager.stop();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static simple_dhcpd::DhcpConfig create_test_config() {
        simple_dhcpd::DhcpConfig config;
        config.enable_logging = false; // Disable logging for tests
        
        // Create a test subnet
        simple_dhcpd::DhcpSubnet subnet;
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
};

// Main function removed - using Google Test framework
