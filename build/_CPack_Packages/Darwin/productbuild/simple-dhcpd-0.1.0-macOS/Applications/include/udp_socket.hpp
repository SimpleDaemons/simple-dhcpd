/**
 * @file udp_socket.hpp
 * @brief UDP socket handling for DHCP communication
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_UDP_SOCKET_HPP
#define SIMPLE_DHCPD_UDP_SOCKET_HPP

#include "dhcp_types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

namespace simple_dhcpd {

/**
 * @brief UDP socket exception
 */
class UdpSocketException : public std::exception {
public:
    explicit UdpSocketException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

/**
 * @brief UDP socket class for DHCP communication
 */
class UdpSocket {
public:
    /**
     * @brief Constructor
     * @param address Address to bind to
     * @param port Port to bind to
     */
    UdpSocket(const std::string& address = "0.0.0.0", uint16_t port = 67);
    
    /**
     * @brief Destructor
     */
    ~UdpSocket();
    
    /**
     * @brief Bind socket to address and port
     * @throws UdpSocketException if binding fails
     */
    void bind();
    
    /**
     * @brief Start receiving data
     * @param callback Callback function for received data
     */
    void start_receiving(std::function<void(const std::vector<uint8_t>&, const std::string&, uint16_t)> callback);
    
    /**
     * @brief Stop receiving data
     */
    void stop_receiving();
    
    /**
     * @brief Send data to specific address and port
     * @param data Data to send
     * @param address Destination address
     * @param port Destination port
     * @return Number of bytes sent
     * @throws UdpSocketException if sending fails
     */
    ssize_t send_to(const std::vector<uint8_t>& data, const std::string& address, uint16_t port);
    
    /**
     * @brief Send data to specific address and port
     * @param data Data to send
     * @param address Destination address
     * @param port Destination port
     * @return Number of bytes sent
     * @throws UdpSocketException if sending fails
     */
    ssize_t send_to(const uint8_t* data, size_t size, const std::string& address, uint16_t port);
    
    /**
     * @brief Send broadcast data
     * @param data Data to send
     * @param port Destination port
     * @return Number of bytes sent
     * @throws UdpSocketException if sending fails
     */
    ssize_t send_broadcast(const std::vector<uint8_t>& data, uint16_t port);
    
    /**
     * @brief Check if socket is bound
     * @return true if socket is bound
     */
    bool is_bound() const;
    
    /**
     * @brief Check if socket is receiving
     * @return true if socket is receiving
     */
    bool is_receiving() const;
    
    /**
     * @brief Get bound address
     * @return Bound address
     */
    std::string get_address() const;
    
    /**
     * @brief Get bound port
     * @return Bound port
     */
    uint16_t get_port() const;
    
    /**
     * @brief Set socket options
     * @param option Socket option
     * @param value Option value
     * @throws UdpSocketException if setting option fails
     */
    void set_socket_option(int option, int value);
    
    /**
     * @brief Enable broadcast
     * @throws UdpSocketException if enabling broadcast fails
     */
    void enable_broadcast();
    
    /**
     * @brief Set receive timeout
     * @param timeout_seconds Timeout in seconds
     * @throws UdpSocketException if setting timeout fails
     */
    void set_receive_timeout(int timeout_seconds);

private:
    std::string address_;
    uint16_t port_;
    int socket_fd_;
    bool bound_;
    std::atomic<bool> receiving_;
    std::thread receive_thread_;
    std::function<void(const std::vector<uint8_t>&, const std::string&, uint16_t)> callback_;
    mutable std::mutex mutex_;
    
    /**
     * @brief Receive loop
     */
    void receive_loop();
    
    /**
     * @brief Create socket
     * @throws UdpSocketException if socket creation fails
     */
    void create_socket();
    
    /**
     * @brief Close socket
     */
    void close_socket();
};

/**
 * @brief DHCP socket manager
 */
class DhcpSocketManager {
public:
    /**
     * @brief Constructor
     */
    DhcpSocketManager();
    
    /**
     * @brief Destructor
     */
    ~DhcpSocketManager();
    
    /**
     * @brief Initialize sockets
     * @param config DHCP configuration
     * @throws UdpSocketException if initialization fails
     */
    void initialize(const DhcpConfig& config);
    
    /**
     * @brief Start all sockets
     * @param callback Callback function for received data
     */
    void start_all(std::function<void(const std::vector<uint8_t>&, const std::string&, uint16_t)> callback);
    
    /**
     * @brief Stop all sockets
     */
    void stop_all();
    
    /**
     * @brief Send DHCP message
     * @param message DHCP message to send
     * @param address Destination address
     * @param port Destination port
     * @return Number of bytes sent
     * @throws UdpSocketException if sending fails
     */
    ssize_t send_dhcp_message(const DhcpMessage& message, const std::string& address, uint16_t port);
    
    /**
     * @brief Send DHCP broadcast message
     * @param message DHCP message to send
     * @param port Destination port
     * @return Number of bytes sent
     * @throws UdpSocketException if sending fails
     */
    ssize_t send_dhcp_broadcast(const DhcpMessage& message, uint16_t port);
    
    /**
     * @brief Check if any socket is receiving
     * @return true if any socket is receiving
     */
    bool is_receiving() const;

private:
    std::vector<std::unique_ptr<UdpSocket>> sockets_;
    mutable std::mutex mutex_;
    
    /**
     * @brief Create socket for address
     * @param address Address to bind to
     * @param port Port to bind to
     * @return Created socket
     * @throws UdpSocketException if creation fails
     */
    std::unique_ptr<UdpSocket> create_socket(const std::string& address, uint16_t port);
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_UDP_SOCKET_HPP
