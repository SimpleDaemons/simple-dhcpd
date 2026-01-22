/**
 * @file test_performance.cpp
 * @brief Performance benchmarking tests for Simple DHCP Daemon
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <cmath>
#include "simple-dhcpd/core/parser.hpp"
#include "simple-dhcpd/core/types.hpp"
#include "simple-dhcpd/lease/manager.hpp"
#include "simple-dhcpd/config/manager.hpp"
#include "simple-dhcpd/utils/utils.hpp"

using namespace simple_dhcpd;
using namespace std::chrono;

// Performance Test: Throughput
class ThroughputTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "perf-test";
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

TEST_F(ThroughputTest, MessageParsingThroughput) {
    // Create test message
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

    const int iterations = 10000;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        DhcpMessage msg;
        parser_->parse_message(discover, msg);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    double rps = (iterations * 1000000.0) / duration.count();

    // Should be able to parse at least 10,000 messages per second
    EXPECT_GT(rps, 10000.0) << "Parsing throughput: " << rps << " messages/sec";

    std::cout << "Message parsing throughput: " << rps << " messages/sec" << std::endl;
}

TEST_F(ThroughputTest, LeaseAllocationThroughput) {
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    const int iterations = 1000;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        MacAddress mac = {
            0x00, 0x11, 0x22, 0x33, 0x44,
            static_cast<uint8_t>(i & 0xFF)
        };
        lease_manager_->allocate_lease(mac, subnet.name);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    double leases_per_sec = (iterations * 1000000.0) / duration.count();

    // Should be able to allocate at least 1,000 leases per second
    EXPECT_GT(leases_per_sec, 1000.0) << "Lease allocation throughput: " << leases_per_sec << " leases/sec";

    std::cout << "Lease allocation throughput: " << leases_per_sec << " leases/sec" << std::endl;
}

// Performance Test: Latency
class LatencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser_ = std::make_unique<DhcpParser>();
    }

    void TearDown() override {
        parser_.reset();
    }

    std::unique_ptr<DhcpParser> parser_;
};

TEST_F(LatencyTest, MessageParsingLatency) {
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

    const int iterations = 1000;
    uint64_t total_latency = 0;

    for (int i = 0; i < iterations; ++i) {
        auto start = high_resolution_clock::now();
        DhcpMessage msg;
        parser_->parse_message(discover, msg);
        auto end = high_resolution_clock::now();

        total_latency += duration_cast<nanoseconds>(end - start).count();
    }

    double avg_latency_ms = (total_latency / iterations) / 1000000.0;

    // Average latency should be less than 5ms
    EXPECT_LT(avg_latency_ms, 5.0) << "Average parsing latency: " << avg_latency_ms << " ms";

    std::cout << "Average message parsing latency: " << avg_latency_ms << " ms" << std::endl;
}

// Performance Test: Resource Usage
class ResourceUsageTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = std::make_unique<ConfigManager>();
        DhcpConfig config = get_default_config();
        config.listen_addresses.push_back("127.0.0.1");

        DhcpSubnet subnet;
        subnet.name = "resource-test";
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

TEST_F(ResourceUsageTest, MemoryUsagePerLease) {
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    const int num_leases = 100;

    // Allocate leases
    std::vector<IpAddress> leases;
    for (int i = 0; i < num_leases; ++i) {
        MacAddress mac = {
            0x00, 0x11, 0x22, 0x33, 0x44,
            static_cast<uint8_t>(i & 0xFF)
        };
        IpAddress ip = lease_manager_->allocate_lease(mac, subnet.name);
        leases.push_back(ip);
    }

    // Basic memory usage check - verify we can handle 100 leases
    // Actual memory measurement would require platform-specific code
    EXPECT_EQ(leases.size(), num_leases);

    // Verify all leases are valid
    for (const auto& ip : leases) {
        EXPECT_NE(ip, 0);
    }
}

TEST_F(ResourceUsageTest, ConcurrentLeaseAllocation) {
    const DhcpSubnet& subnet = config_manager_->get_config().subnets[0];
    const int num_threads = 4;
    const int leases_per_thread = 25;
    std::atomic<int> success_count(0);

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

    EXPECT_EQ(success_count.load(), num_threads * leases_per_thread);
    EXPECT_LT(duration.count(), 1000) << "Concurrent allocation took " << duration.count() << " ms";

    std::cout << "Concurrent lease allocation: " << success_count.load()
              << " leases in " << duration.count() << " ms" << std::endl;
}
