/**
 * @file test_runner.cpp
 * @brief Main test runner for all Simple DHCP Daemon tests
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <iostream>
#include "test_basic.hpp"
#include "test_dhcp_parser.hpp"
#include "test_lease_manager.hpp"

int main() {
    int total_passed = 0;
    int total_failed = 0;
    
    std::cout << "=========================================" << std::endl;
    std::cout << "Simple DHCP Daemon - Test Suite" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
    
    // Run basic tests
    std::cout << "Running Basic Tests..." << std::endl;
    std::cout << "---------------------" << std::endl;
    int basic_result = BasicTest::run_tests();
    if (basic_result == 0) {
        total_passed++;
    } else {
        total_failed++;
    }
    std::cout << std::endl;
    
    // Run DHCP parser tests
    std::cout << "Running DHCP Parser Tests..." << std::endl;
    std::cout << "---------------------------" << std::endl;
    int parser_result = DhcpParserTest::run_tests();
    if (parser_result == 0) {
        total_passed++;
    } else {
        total_failed++;
    }
    std::cout << std::endl;
    
    // Run lease manager tests
    std::cout << "Running Lease Manager Tests..." << std::endl;
    std::cout << "-----------------------------" << std::endl;
    int lease_result = LeaseManagerTest::run_tests();
    if (lease_result == 0) {
        total_passed++;
    } else {
        total_failed++;
    }
    std::cout << std::endl;
    
    // Summary
    std::cout << "=========================================" << std::endl;
    std::cout << "Test Suite Summary" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Test Suites: " << (total_passed + total_failed) << " total" << std::endl;
    std::cout << "Passed: " << total_passed << std::endl;
    std::cout << "Failed: " << total_failed << std::endl;
    std::cout << "=========================================" << std::endl;
    
    return (total_failed == 0) ? 0 : 1;
}

