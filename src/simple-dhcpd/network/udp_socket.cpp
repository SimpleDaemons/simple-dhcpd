/**
 * @file network/udp_socket.cpp
 * @brief UDP socket handling implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-dhcpd/network/udp_socket.hpp"
#include "simple-dhcpd/utils/logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>

namespace simple_dhcpd {

UdpSocket::UdpSocket(const std::string& address, uint16_t port)
    : address_(address), port_(port), socket_fd_(-1), bound_(false), receiving_(false) {
    create_socket();
}

UdpSocket::~UdpSocket() {
    stop_receiving();
    close_socket();
}

void UdpSocket::create_socket() {
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        throw UdpSocketException("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close_socket();
        throw UdpSocketException("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
    }
    
    // Enable broadcast
    enable_broadcast();
    
    LOG_DEBUG("UDP socket created for " + address_ + ":" + std::to_string(port_));
}

void UdpSocket::bind() {
    if (bound_) {
        return;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    
    if (address_ == "0.0.0.0" || address_ == "::") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_aton(address_.c_str(), &addr.sin_addr) == 0) {
            close_socket();
            throw UdpSocketException("Invalid address: " + address_);
        }
    }
    
    if (::bind(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close_socket();
        throw UdpSocketException("Failed to bind socket: " + std::string(strerror(errno)));
    }
    
    bound_ = true;
    LOG_INFO("UDP socket bound to " + address_ + ":" + std::to_string(port_));
}

void UdpSocket::start_receiving(std::function<void(const std::vector<uint8_t>&, const std::string&, uint16_t)> callback) {
    if (!bound_) {
        throw UdpSocketException("Socket not bound");
    }
    
    if (receiving_) {
        return;
    }
    
    callback_ = callback;
    receiving_ = true;
    receive_thread_ = std::thread(&UdpSocket::receive_loop, this);
    
    LOG_DEBUG("Started receiving on " + address_ + ":" + std::to_string(port_));
}

void UdpSocket::stop_receiving() {
    if (!receiving_) {
        return;
    }
    
    receiving_ = false;
    
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    
    LOG_DEBUG("Stopped receiving on " + address_ + ":" + std::to_string(port_));
}

ssize_t UdpSocket::send_to(const std::vector<uint8_t>& data, const std::string& address, uint16_t port) {
    return send_to(data.data(), data.size(), address, port);
}

ssize_t UdpSocket::send_to(const uint8_t* data, size_t size, const std::string& address, uint16_t port) {
    if (!bound_) {
        throw UdpSocketException("Socket not bound");
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_aton(address.c_str(), &addr.sin_addr) == 0) {
        throw UdpSocketException("Invalid destination address: " + address);
    }
    
    ssize_t bytes_sent = sendto(socket_fd_, data, size, 0, (struct sockaddr*)&addr, sizeof(addr));
    if (bytes_sent < 0) {
        throw UdpSocketException("Failed to send data: " + std::string(strerror(errno)));
    }
    
    LOG_DEBUG("Sent " + std::to_string(bytes_sent) + " bytes to " + address + ":" + std::to_string(port));
    return bytes_sent;
}

ssize_t UdpSocket::send_broadcast(const std::vector<uint8_t>& data, uint16_t port) {
    if (!bound_) {
        throw UdpSocketException("Socket not bound");
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    ssize_t bytes_sent = sendto(socket_fd_, data.data(), data.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (bytes_sent < 0) {
        throw UdpSocketException("Failed to send broadcast: " + std::string(strerror(errno)));
    }
    
    LOG_DEBUG("Sent " + std::to_string(bytes_sent) + " bytes broadcast to port " + std::to_string(port));
    return bytes_sent;
}

bool UdpSocket::is_bound() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bound_;
}

bool UdpSocket::is_receiving() const {
    return receiving_;
}

std::string UdpSocket::get_address() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return address_;
}

uint16_t UdpSocket::get_port() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return port_;
}

void UdpSocket::set_socket_option(int option, int value) {
    if (setsockopt(socket_fd_, SOL_SOCKET, option, &value, sizeof(value)) < 0) {
        throw UdpSocketException("Failed to set socket option: " + std::string(strerror(errno)));
    }
}

void UdpSocket::enable_broadcast() {
    int opt = 1;
    set_socket_option(SO_BROADCAST, opt);
}

void UdpSocket::set_receive_timeout(int timeout_seconds) {
    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw UdpSocketException("Failed to set receive timeout: " + std::string(strerror(errno)));
    }
}

void UdpSocket::receive_loop() {
    std::vector<uint8_t> buffer(1500); // Standard MTU
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    while (receiving_) {
        ssize_t bytes_received = recvfrom(socket_fd_, buffer.data(), buffer.size(), 0,
                                         (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Timeout, continue
                continue;
            } else if (errno == EINTR) {
                // Interrupted, continue
                continue;
            } else {
                LOG_ERROR("Failed to receive data: " + std::string(strerror(errno)));
                break;
            }
        }
        
        if (bytes_received > 0) {
            std::string client_address = inet_ntoa(client_addr.sin_addr);
            uint16_t client_port = ntohs(client_addr.sin_port);
            
            std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytes_received);
            
            if (callback_) {
                callback_(data, client_address, client_port);
            }
        }
    }
}

void UdpSocket::close_socket() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    bound_ = false;
}

// DhcpSocketManager implementation

DhcpSocketManager::DhcpSocketManager() = default;

DhcpSocketManager::~DhcpSocketManager() {
    stop_all();
}

void DhcpSocketManager::initialize(const DhcpConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    sockets_.clear();
    
    for (const auto& address : config.listen_addresses) {
        size_t colon_pos = address.find(':');
        if (colon_pos == std::string::npos) {
            throw UdpSocketException("Invalid address format: " + address);
        }
        
        std::string addr = address.substr(0, colon_pos);
        uint16_t port = static_cast<uint16_t>(std::stoi(address.substr(colon_pos + 1)));
        
        auto socket = create_socket(addr, port);
        socket->bind();
        sockets_.push_back(std::move(socket));
    }
    
    LOG_INFO("Initialized " + std::to_string(sockets_.size()) + " UDP sockets");
}

void DhcpSocketManager::start_all(std::function<void(const std::vector<uint8_t>&, const std::string&, uint16_t)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& socket : sockets_) {
        socket->start_receiving(callback);
    }
    
    LOG_INFO("Started all UDP sockets");
}

void DhcpSocketManager::stop_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& socket : sockets_) {
        socket->stop_receiving();
    }
    
    LOG_INFO("Stopped all UDP sockets");
}

ssize_t DhcpSocketManager::send_dhcp_message(const DhcpMessage& message, const std::string& address, uint16_t port) {
    // Convert DHCP message to bytes (simplified)
    std::vector<uint8_t> data;
    data.resize(sizeof(DhcpMessageHeader) + 312); // Header + options
    
    // Copy header
    memcpy(data.data(), &message.header, sizeof(DhcpMessageHeader));
    
    // Copy options (simplified)
    size_t offset = sizeof(DhcpMessageHeader);
    for (const auto& option : message.options) {
        if (offset + 2 + option.length > data.size()) {
            break;
        }
        
        data[offset++] = static_cast<uint8_t>(option.code);
        data[offset++] = option.length;
        memcpy(data.data() + offset, option.data.data(), option.length);
        offset += option.length;
    }
    
    // Add END option
    if (offset < data.size()) {
        data[offset] = static_cast<uint8_t>(DhcpOptionCode::END);
    }
    
    // Use first socket for sending
    if (sockets_.empty()) {
        throw UdpSocketException("No sockets available");
    }
    
    return sockets_[0]->send_to(data, address, port);
}

ssize_t DhcpSocketManager::send_dhcp_broadcast(const DhcpMessage& message, uint16_t port) {
    // Convert DHCP message to bytes (simplified)
    std::vector<uint8_t> data;
    data.resize(sizeof(DhcpMessageHeader) + 312); // Header + options
    
    // Copy header
    memcpy(data.data(), &message.header, sizeof(DhcpMessageHeader));
    
    // Copy options (simplified)
    size_t offset = sizeof(DhcpMessageHeader);
    for (const auto& option : message.options) {
        if (offset + 2 + option.length > data.size()) {
            break;
        }
        
        data[offset++] = static_cast<uint8_t>(option.code);
        data[offset++] = option.length;
        memcpy(data.data() + offset, option.data.data(), option.length);
        offset += option.length;
    }
    
    // Add END option
    if (offset < data.size()) {
        data[offset] = static_cast<uint8_t>(DhcpOptionCode::END);
    }
    
    // Use first socket for sending
    if (sockets_.empty()) {
        throw UdpSocketException("No sockets available");
    }
    
    return sockets_[0]->send_broadcast(data, port);
}

bool DhcpSocketManager::is_receiving() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& socket : sockets_) {
        if (socket->is_receiving()) {
            return true;
        }
    }
    
    return false;
}

std::unique_ptr<UdpSocket> DhcpSocketManager::create_socket(const std::string& address, uint16_t port) {
    return std::make_unique<UdpSocket>(address, port);
}

} // namespace simple_dhcpd
