/**
 * @file test_load.cpp
 * @brief Load and stress tests for Simple DHCP Daemon
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include "simple-dhcpd/core/parser.hpp"
#include "simple-dhcpd/core/types.hpp"
#include "simple-dhcpd/lease/manager.hpp"
#include "simple-dhcpd/config/manager.hpp"
#include "simple-dhcpd/utils/utils.hpp"

using namespace simple_dhcpd;
using namespace std::chrono;

// Load Test: High Request Rate
class HighRequestRateTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "load-test";
        subnet.network = string_to_ip("192.168.1.0");
        subnet.prefix_length = 24;
        subnet.range_start = string_to_ip("192.168.1.100");
        subnet.range_end = string_to_ip("192.168.1.254");
        subnet.lease_time = 3600;
        config.subnets.push_back(subnet);

        config_manager_->set_config(config);
        lease_manager_ = std::make_unique<LeaseManager>(config_manager_->get_config());
        parser_ = std::make_unique<DhcpParser>();
    }

    void TearDown() override {
        parser_.reset();
        lease_manager_.reset();
        config_manager_.reset();
    }

    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<LeaseManager> lease_manager_;
    std::unique_ptr<DhcpParser> parser_;
};

TEST_F(HighRequestRateTest, RPSLimitTest) {
    // Test handling high request rate
    const int total_requests = 10000;
    const int target_rps = 1000; // Target: 1000 requests per second
    std::atomic<int> processed(0);

    auto process_request = [&]() {
        std::vector<uint8_t> discover(576, 0);
        DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(discover.data());
        header->op = 1;
        header->htype = 1;
        header->hlen = 6;

        size_t offset = sizeof(DhcpMessageHeader);
        discover[offset++] = 99;
        discover[offset++] = 130;
        discover[offset++] = 83;
        discover[offset++] = 99;
        discover[offset++] = 53;
        discover[offset++] = 1;
        discover[offset++] = 1;
        discover[offset++] = 255;

        DhcpMessage msg;
        if (parser_->parse_message(discover, msg)) {
            processed++;
        }
    };

    auto start = high_resolution_clock::now();

    for (int i = 0; i < total_requests; ++i) {
        process_request();
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double actual_rps = (processed.load() * 1000.0) / duration.count();

    EXPECT_GE(actual_rps, target_rps * 0.8) << "RPS: " << actual_rps << " (target: " << target_rps << ")";
    EXPECT_EQ(processed.load(), total_requests);

    std::cout << "RPS Test: " << actual_rps << " requests/sec (processed "
              << processed.load() << " requests)" << std::endl;
}

TEST_F(HighRequestRateTest, RPSStabilityTest) {
    // Test stability under sustained load
    const int iterations = 5;
    const int requests_per_iteration = 2000;
    std::vector<double> rps_measurements;

    for (int iter = 0; iter < iterations; ++iter) {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < requests_per_iteration; ++i) {
            std::vector<uint8_t> discover(576, 0);
            DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(discover.data());
            header->op = 1;
            header->htype = 1;
            header->hlen = 6;

            size_t offset = sizeof(DhcpMessageHeader);
            discover[offset++] = 99;
            discover[offset++] = 130;
            discover[offset++] = 83;
            discover[offset++] = 99;
            discover[offset++] = 53;
            discover[offset++] = 1;
            discover[offset++] = 1;
            discover[offset++] = 255;

            DhcpMessage msg;
            parser_->parse_message(discover, msg);
        }

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        double rps = (requests_per_iteration * 1000.0) / duration.count();
        rps_measurements.push_back(rps);
    }

    // Calculate variance
    double avg_rps = 0;
    for (double rps : rps_measurements) {
        avg_rps += rps;
    }
    avg_rps /= iterations;

    double variance = 0;
    for (double rps : rps_measurements) {
        variance += (rps - avg_rps) * (rps - avg_rps);
    }
    variance /= iterations;
    double std_dev = sqrt(variance);
    double cv = (std_dev / avg_rps) * 100; // Coefficient of variation

    // Coefficient of variation should be less than 20% (stable performance)
    EXPECT_LT(cv, 20.0) << "RPS stability CV: " << cv << "% (avg: " << avg_rps << ")";

    std::cout << "RPS Stability: avg=" << avg_rps << ", CV=" << cv << "%" << std::endl;
}

// Load Test: Concurrent Leases
class ConcurrentLeaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "concurrent-test";
        subnet.network = string_to_ip("10.0.0.0");
        subnet.prefix_length = 24;
        subnet.range_start = string_to_ip("10.0.0.100");
        subnet.range_end = string_to_ip("10.0.0.254");
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

TEST_F(ConcurrentLeaseTest, LeaseLimitTest) {
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    const int max_leases = 155; // 10.0.0.100 to 10.0.0.254

    std::vector<IpAddress> leases;
    for (int i = 0; i < max_leases; ++i) {
        MacAddress mac = {
            0x00, 0x11, 0x22, 0x33, 0x44,
            static_cast<uint8_t>(i & 0xFF)
        };
        IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
        if (ip != 0) {
            leases.push_back(ip);
        }
    }

    // Should be able to allocate all available leases
    EXPECT_GE(leases.size(), max_leases * 0.9) << "Allocated " << leases.size() << " of " << max_leases << " leases";

    std::cout << "Lease Limit Test: Allocated " << leases.size() << " leases" << std::endl;
}

TEST_F(ConcurrentLeaseTest, ConcurrentLeaseStability) {
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    const int num_threads = 10;
    const int leases_per_thread = 10;
    std::atomic<int> success_count(0);
    std::atomic<int> failure_count(0);

    auto allocate_leases = [&](int thread_id) {
        for (int i = 0; i < leases_per_thread; ++i) {
            MacAddress mac = {
                static_cast<uint8_t>(thread_id),
                0x11, 0x22, 0x33, 0x44,
                static_cast<uint8_t>(i & 0xFF)
            };
            IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
            if (ip != 0) {
                success_count++;
            } else {
                failure_count++;
            }
        }
    };

    std::vector<std::thread> threads;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(allocate_leases, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // Should have high success rate
    double success_rate = (success_count.load() * 100.0) / (num_threads * leases_per_thread);
    EXPECT_GT(success_rate, 90.0) << "Success rate: " << success_rate << "%";

    std::cout << "Concurrent Lease Stability: " << success_count.load() << " successes, "
              << failure_count.load() << " failures in " << duration.count() << " ms" << std::endl;
}

// Load Test: Stress Testing
class StressTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "stress-test";
        subnet.network = string_to_ip("172.16.0.0");
        subnet.prefix_length = 24;
        subnet.range_start = string_to_ip("172.16.0.100");
        subnet.range_end = string_to_ip("172.16.0.254");
        subnet.lease_time = 3600;
        config.subnets.push_back(subnet);

        config_manager_->set_config(config);
        lease_manager_ = std::make_unique<LeaseManager>(config_manager_->get_config());
        parser_ = std::make_unique<DhcpParser>();
    }

    void TearDown() override {
        parser_.reset();
        lease_manager_.reset();
        config_manager_.reset();
    }

    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<LeaseManager> lease_manager_;
    std::unique_ptr<DhcpParser> parser_;
};

TEST_F(StressTest, HighLoadTest) {
    // Sustained high load test
    const int duration_seconds = 5;
    const int target_rps = 500;
    std::atomic<int> total_processed(0);
    std::atomic<bool> running(true);

    auto worker = [&]() {
        int local_count = 0;
        auto end_time = high_resolution_clock::now() + seconds(duration_seconds);

        while (high_resolution_clock::now() < end_time && running.load()) {
            std::vector<uint8_t> discover(576, 0);
            DhcpMessageHeader* header = reinterpret_cast<DhcpMessageHeader*>(discover.data());
            header->op = 1;
            header->htype = 1;
            header->hlen = 6;

            size_t offset = sizeof(DhcpMessageHeader);
            discover[offset++] = 99;
            discover[offset++] = 130;
            discover[offset++] = 83;
            discover[offset++] = 99;
            discover[offset++] = 53;
            discover[offset++] = 1;
            discover[offset++] = 1;
            discover[offset++] = 255;

            DhcpMessage msg;
            if (parser_->parse_message(discover, msg)) {
                local_count++;
            }
        }

        total_processed += local_count;
    };

    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back(worker);
    }

    for (auto& w : workers) {
        w.join();
    }

    double actual_rps = total_processed.load() / duration_seconds;

    EXPECT_GE(actual_rps, target_rps * 0.8) << "Sustained RPS: " << actual_rps;

    std::cout << "High Load Test: " << actual_rps << " RPS sustained for "
              << duration_seconds << " seconds" << std::endl;
}

TEST_F(StressTest, FailureRecoveryTest) {
    // Test recovery from failures
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];

    // Allocate leases until pool is exhausted
    std::vector<IpAddress> leases;
    for (int i = 0; i < 200; ++i) {
        MacAddress mac = {
            0x00, 0x11, 0x22, 0x33, 0x44,
            static_cast<uint8_t>(i & 0xFF)
        };
        IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
        if (ip != 0) {
            leases.push_back(ip);
        }
    }

    // System should handle exhaustion gracefully
    EXPECT_GT(leases.size(), 0) << "Should allocate some leases";

    // Try to allocate more - should handle gracefully
    MacAddress test_mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // This might fail, but shouldn't crash
    EXPECT_NO_THROW({
        IpAddress ip = lease_manager_->allocate_lease(test_mac, subnet.name);
        // Result doesn't matter, just shouldn't crash
    });

    std::cout << "Failure Recovery Test: Handled " << leases.size() << " leases gracefully" << std::endl;
}

TEST_F(StressTest, StabilityTest) {
    // Long-running stability test
    const int iterations = 1000;
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    std::atomic<int> errors(0);

    for (int i = 0; i < iterations; ++i) {
        MacAddress mac = {
            0x00, 0x11, 0x22, 0x33, 0x44,
            static_cast<uint8_t>(i & 0xFF)
        };

        try {
            IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
            if (ip == 0) {
                errors++;
            }
        } catch (...) {
            errors++;
        }
    }

    // Error rate should be very low
    double error_rate = (errors.load() * 100.0) / iterations;
    EXPECT_LT(error_rate, 5.0) << "Error rate: " << error_rate << "%";

    std::cout << "Stability Test: " << errors.load() << " errors in "
              << iterations << " iterations (" << error_rate << "%)" << std::endl;
}
