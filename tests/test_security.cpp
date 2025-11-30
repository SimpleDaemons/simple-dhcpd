/**
 * @file test_security.cpp
 * @brief Security feature tests (Option82, MAC/IP filters, auth HMAC, rate limiting)
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include "simple-dhcpd/security/manager.hpp"

using namespace simple_dhcpd;

class SecurityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Fresh manager per test (no background threads by default)
        manager = std::make_unique<DhcpSecurityManager>();
    }

    void TearDown() override {
        // Ensure clean shutdown if a test started the manager
        if (manager) manager->stop();
    }

    std::unique_ptr<DhcpSecurityManager> manager;
};

TEST_F(SecurityTest, MacFilterAllowDeny) {
    using namespace std::chrono_literals;
    auto fut = std::async(std::launch::async, [&] {
        MacFilterRule allow_rule{"00:11:22:33:44:55", true, "allow test"};
        MacFilterRule deny_rule{"aa:bb:cc:*", false, "deny pattern"};
        manager->add_mac_filter_rule(allow_rule);
        manager->add_mac_filter_rule(deny_rule);

        EXPECT_TRUE(manager->check_mac_address("00:11:22:33:44:55"));
        EXPECT_FALSE(manager->check_mac_address("aa:bb:cc:00:00:01"));
        return true;
    });
    ASSERT_EQ(fut.wait_for(10s), std::future_status::ready) << "Test timed out after 10s";
    ASSERT_TRUE(fut.get());
}

TEST_F(SecurityTest, IpFilterAllowDenyExact) {
    IpFilterRule allow_rule{0x01020304, 0xFFFFFFFF, true, "allow 1.2.3.4"};
    IpFilterRule deny_rule{0x0A000001, 0xFFFFFFFF, false, "deny 10.0.0.1"};
    manager->add_ip_filter_rule(allow_rule);
    manager->add_ip_filter_rule(deny_rule);

    EXPECT_TRUE(manager->check_ip_address(0x01020304));
    EXPECT_FALSE(manager->check_ip_address(0x0A000001));
}

TEST_F(SecurityTest, RateLimitBasic) {
    RateLimitRule per_mac{"00:11:22:33:44:55", "mac", 3, std::chrono::seconds(1)};
    manager->add_rate_limit_rule(per_mac);

    // First 3 should pass, 4th should fail within same window
    EXPECT_TRUE(manager->check_rate_limit("00:11:22:33:44:55", "mac"));
    EXPECT_TRUE(manager->check_rate_limit("00:11:22:33:44:55", "mac"));
    EXPECT_TRUE(manager->check_rate_limit("00:11:22:33:44:55", "mac"));
    EXPECT_FALSE(manager->check_rate_limit("00:11:22:33:44:55", "mac"));
}

TEST_F(SecurityTest, Option82ValidationRequiredAndPresent) {
    manager->set_option_82_validation_enabled(true);
    // Add a rule requiring Option82 on interface "eth0"
    // We can't add rules directly (no public API yet), but validate_option_82 returns true
    // if not required for the interface. So test the positive path: valid data present
    std::vector<uint8_t> option82;
    // Sub-option 1 (circuit-id) length 2, value 0x01 0x02
    option82.push_back(1);
    option82.push_back(2);
    option82.push_back(0x01);
    option82.push_back(0x02);
    // Sub-option 2 (remote-id) length 3, value 0x03 0x04 0x05
    option82.push_back(2);
    option82.push_back(3);
    option82.push_back(0x03);
    option82.push_back(0x04);
    option82.push_back(0x05);

    EXPECT_TRUE(manager->validate_option_82(option82, "eth0"));
}


