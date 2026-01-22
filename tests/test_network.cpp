/**
 * @file test_network.cpp
 * @brief Google Test tests for network layer (UDP socket, network interface, IP validation)
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include "simple-dhcpd/network/udp_socket.hpp"
#include "simple-dhcpd/utils/utils.hpp"

using namespace simple_dhcpd;

// UDP Socket Tests
class UdpSocketTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

TEST_F(UdpSocketTest, SocketCreation) {
    EXPECT_NO_THROW({
        UdpSocket socket("127.0.0.1", 6767);
        EXPECT_FALSE(socket.is_bound());
        EXPECT_FALSE(socket.is_receiving());
    });
}

TEST_F(UdpSocketTest, SocketBinding) {
    UdpSocket socket("127.0.0.1", 6768);
    EXPECT_NO_THROW(socket.bind());
    EXPECT_TRUE(socket.is_bound());
    EXPECT_EQ(socket.get_address(), "127.0.0.1");
    EXPECT_EQ(socket.get_port(), 6768);
}

TEST_F(UdpSocketTest, SocketBindingAnyAddress) {
    UdpSocket socket("0.0.0.0", 6769);
    EXPECT_NO_THROW(socket.bind());
    EXPECT_TRUE(socket.is_bound());
}

TEST_F(UdpSocketTest, SocketSendReceive) {
    UdpSocket server("127.0.0.1", 6770);
    UdpSocket client("127.0.0.1", 6771);

    server.bind();
    client.bind();

    std::vector<uint8_t> received_data;
    std::string received_address;
    uint16_t received_port = 0;
    bool data_received = false;

    server.start_receiving([&](const std::vector<uint8_t>& data, const std::string& addr, uint16_t port) {
        received_data = data;
        received_address = addr;
        received_port = port;
        data_received = true;
    });

    // Wait a bit for server to start receiving
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
    ssize_t sent = client.send_to(test_data, "127.0.0.1", 6770);
    EXPECT_GT(sent, 0);

    // Wait for data to be received
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(data_received);
    EXPECT_EQ(received_data, test_data);
    EXPECT_EQ(received_address, "127.0.0.1");

    server.stop_receiving();
}

TEST_F(UdpSocketTest, SocketBroadcast) {
    UdpSocket socket("0.0.0.0", 6772);
    socket.bind();

    std::vector<uint8_t> test_data = {0xAA, 0xBB, 0xCC};
    EXPECT_NO_THROW({
        ssize_t sent = socket.send_broadcast(test_data, 6773);
        EXPECT_GT(sent, 0);
    });
}

TEST_F(UdpSocketTest, SocketExceptionOnInvalidAddress) {
    EXPECT_THROW({
        UdpSocket socket("999.999.999.999", 6774);
        socket.bind();
    }, UdpSocketException);
}

TEST_F(UdpSocketTest, SocketExceptionOnReceiveWithoutBind) {
    UdpSocket socket("127.0.0.1", 6775);
    EXPECT_THROW({
        socket.start_receiving([](const std::vector<uint8_t>&, const std::string&, uint16_t) {});
    }, UdpSocketException);
}

// IP Address Validation Tests
class IpValidationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(IpValidationTest, StringToIpConversion) {
    IpAddress ip = string_to_ip("192.168.1.1");
    EXPECT_NE(ip, 0);

    std::string ip_str = ip_to_string(ip);
    EXPECT_EQ(ip_str, "192.168.1.1");
}

TEST_F(IpValidationTest, InvalidIpString) {
    IpAddress ip = string_to_ip("invalid.ip.address");
    EXPECT_EQ(ip, 0);
}

TEST_F(IpValidationTest, IpInSubnet) {
    IpAddress network = string_to_ip("192.168.1.0");
    IpAddress ip = string_to_ip("192.168.1.100");

    EXPECT_TRUE(is_ip_in_subnet(ip, network, 24));
    EXPECT_FALSE(is_ip_in_subnet(ip, network, 32));
}

TEST_F(IpValidationTest, IpNotInSubnet) {
    IpAddress network = string_to_ip("192.168.1.0");
    IpAddress ip = string_to_ip("192.168.2.100");

    EXPECT_FALSE(is_ip_in_subnet(ip, network, 24));
}

TEST_F(IpValidationTest, NetworkAddressCalculation) {
    IpAddress ip = string_to_ip("192.168.1.100");
    IpAddress network = get_network_address(ip, 24);

    EXPECT_EQ(ip_to_string(network), "192.168.1.0");
}

TEST_F(IpValidationTest, BroadcastAddressCalculation) {
    IpAddress network = string_to_ip("192.168.1.0");
    IpAddress broadcast = get_broadcast_address(network, 24);

    EXPECT_EQ(ip_to_string(broadcast), "192.168.1.255");
}

TEST_F(IpValidationTest, SubnetMaskCalculation) {
    IpAddress network = string_to_ip("10.0.0.0");
    IpAddress ip1 = string_to_ip("10.0.0.1");
    IpAddress ip2 = string_to_ip("10.0.255.254");
    IpAddress ip3 = string_to_ip("11.0.0.1");

    EXPECT_TRUE(is_ip_in_subnet(ip1, network, 8));
    EXPECT_TRUE(is_ip_in_subnet(ip2, network, 8));
    EXPECT_FALSE(is_ip_in_subnet(ip3, network, 8));
}

// Network Interface Tests (basic tests - full implementation would require platform-specific code)
class NetworkInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(NetworkInterfaceTest, LocalhostInterface) {
    // Test that we can bind to localhost
    UdpSocket socket("127.0.0.1", 6776);
    EXPECT_NO_THROW(socket.bind());
    EXPECT_TRUE(socket.is_bound());
}

TEST_F(NetworkInterfaceTest, AnyInterface) {
    // Test that we can bind to any interface
    UdpSocket socket("0.0.0.0", 6777);
    EXPECT_NO_THROW(socket.bind());
    EXPECT_TRUE(socket.is_bound());
}
