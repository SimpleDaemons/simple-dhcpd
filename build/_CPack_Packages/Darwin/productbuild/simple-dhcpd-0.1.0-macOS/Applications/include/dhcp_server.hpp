/**
 * @file dhcp_server.hpp
 * @brief Main DHCP server class
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_DHCP_SERVER_HPP
#define SIMPLE_DHCPD_DHCP_SERVER_HPP

#include "dhcp_types.hpp"
#include "config.hpp"
#include "udp_socket.hpp"
#include "dhcp_parser.hpp"
#include "lease_manager.hpp"
#include "logger.hpp"
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

namespace simple_dhcpd {

/**
 * @brief DHCP server exception
 */
class DhcpServerException : public std::exception {
public:
    explicit DhcpServerException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

/**
 * @brief Main DHCP server class
 */
class DhcpServer {
public:
    /**
     * @brief Constructor
     * @param config_file Configuration file path
     */
    explicit DhcpServer(const std::string& config_file = "");
    
    /**
     * @brief Destructor
     */
    ~DhcpServer();
    
    /**
     * @brief Initialize server
     * @throws DhcpServerException if initialization fails
     */
    void initialize();
    
    /**
     * @brief Start server
     * @throws DhcpServerException if starting fails
     */
    void start();
    
    /**
     * @brief Stop server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    bool is_running() const;
    
    /**
     * @brief Reload configuration
     * @throws DhcpServerException if reload fails
     */
    void reload_config();
    
    /**
     * @brief Get server statistics
     * @return Server statistics
     */
    DhcpStats get_statistics() const;
    
    /**
     * @brief Set signal handler
     * @param handler Signal handler function
     */
    void set_signal_handler(std::function<void(int)> handler);

private:
    std::string config_file_;
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<DhcpSocketManager> socket_manager_;
    std::unique_ptr<LeaseManager> lease_manager_;
    std::atomic<bool> running_;
    std::atomic<bool> initialized_;
    mutable std::mutex mutex_;
    
    /**
     * @brief Handle received DHCP message
     * @param data Received data
     * @param client_address Client address
     * @param client_port Client port
     */
    void handle_dhcp_message(const std::vector<uint8_t>& data, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Handle DHCP Discover message
     * @param message DHCP message
     * @param client_address Client address
     * @param client_port Client port
     */
    void handle_discover(const DhcpMessage& message, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Handle DHCP Request message
     * @param message DHCP message
     * @param client_address Client address
     * @param client_port Client port
     */
    void handle_request(const DhcpMessage& message, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Handle DHCP Release message
     * @param message DHCP message
     * @param client_address Client address
     * @param client_port Client port
     */
    void handle_release(const DhcpMessage& message, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Handle DHCP Decline message
     * @param message DHCP message
     * @param client_address Client address
     * @param client_port Client port
     */
    void handle_decline(const DhcpMessage& message, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Handle DHCP Inform message
     * @param message DHCP message
     * @param client_address Client address
     * @param client_port Client port
     */
    void handle_inform(const DhcpMessage& message, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Send DHCP Offer message
     * @param message Original DHCP message
     * @param lease Allocated lease
     * @param client_address Client address
     * @param client_port Client port
     */
    void send_offer(const DhcpMessage& message, const DhcpLease& lease, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Send DHCP ACK message
     * @param message Original DHCP message
     * @param lease Allocated lease
     * @param client_address Client address
     * @param client_port Client port
     */
    void send_ack(const DhcpMessage& message, const DhcpLease& lease, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Send DHCP NAK message
     * @param message Original DHCP message
     * @param client_address Client address
     * @param client_port Client port
     */
    void send_nak(const DhcpMessage& message, const std::string& client_address, uint16_t client_port);
    
    /**
     * @brief Find appropriate subnet for client
     * @param message DHCP message
     * @return Subnet name
     * @throws DhcpServerException if no subnet found
     */
    std::string find_subnet_for_client(const DhcpMessage& message);
    
    /**
     * @brief Build DHCP options for subnet
     * @param subnet Subnet configuration
     * @return DHCP options
     */
    std::vector<DhcpOption> build_subnet_options(const DhcpSubnet& subnet);
    
    /**
     * @brief Build DHCP options for lease
     * @param lease Lease information
     * @param subnet Subnet configuration
     * @return DHCP options
     */
    std::vector<DhcpOption> build_lease_options(const DhcpLease& lease, const DhcpSubnet& subnet);
    
    /**
     * @brief Log DHCP message
     * @param message DHCP message
     * @param action Action taken
     */
    void log_dhcp_message(const DhcpMessage& message, const std::string& action);
    
    /**
     * @brief Update server statistics
     * @param message_type Message type
     */
    void update_statistics(DhcpMessageType message_type);
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_DHCP_SERVER_HPP
