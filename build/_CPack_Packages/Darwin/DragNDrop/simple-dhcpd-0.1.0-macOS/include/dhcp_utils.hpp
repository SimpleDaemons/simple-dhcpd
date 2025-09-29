/**
 * @file dhcp_utils.hpp
 * @brief DHCP utility functions
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_DHCP_UTILS_HPP
#define SIMPLE_DHCPD_DHCP_UTILS_HPP

#include "dhcp_types.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <arpa/inet.h>

namespace simple_dhcpd {

/**
 * @brief Convert IP address to string
 * @param ip IP address in network byte order
 * @return IP address string
 */
inline std::string ip_to_string(IpAddress ip) {
    struct in_addr addr;
    addr.s_addr = ip;
    return std::string(inet_ntoa(addr));
}

/**
 * @brief Convert string to IP address
 * @param ip_str IP address string
 * @return IP address in network byte order
 */
inline IpAddress string_to_ip(const std::string& ip_str) {
    struct in_addr addr;
    if (inet_aton(ip_str.c_str(), &addr) == 0) {
        return 0;
    }
    return addr.s_addr;
}

/**
 * @brief Convert MAC address to string
 * @param mac MAC address
 * @return MAC address string in format "XX:XX:XX:XX:XX:XX"
 */
inline std::string mac_to_string(const MacAddress& mac) {
    std::ostringstream oss;
    for (size_t i = 0; i < mac.size(); ++i) {
        if (i > 0) oss << ":";
        oss << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(mac[i]);
    }
    return oss.str();
}

/**
 * @brief Convert string to MAC address
 * @param mac_str MAC address string
 * @return MAC address
 */
inline MacAddress string_to_mac(const std::string& mac_str) {
    MacAddress mac = {0};
    std::istringstream iss(mac_str);
    std::string byte_str;
    
    for (size_t i = 0; i < mac.size() && std::getline(iss, byte_str, ':'); ++i) {
        mac[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
    }
    
    return mac;
}

/**
 * @brief Check if IP address is in subnet
 * @param ip IP address to check
 * @param network Network address
 * @param prefix_length Network prefix length
 * @return true if IP is in subnet
 */
inline bool is_ip_in_subnet(IpAddress ip, IpAddress network, uint8_t prefix_length) {
    uint32_t mask = (prefix_length == 0) ? 0 : (0xFFFFFFFF << (32 - prefix_length));
    return (ip & htonl(mask)) == (network & htonl(mask));
}

/**
 * @brief Calculate network address from IP and prefix length
 * @param ip IP address
 * @param prefix_length Network prefix length
 * @return Network address
 */
inline IpAddress get_network_address(IpAddress ip, uint8_t prefix_length) {
    uint32_t mask = (prefix_length == 0) ? 0 : (0xFFFFFFFF << (32 - prefix_length));
    return ip & htonl(mask);
}

/**
 * @brief Calculate broadcast address from network and prefix length
 * @param network Network address
 * @param prefix_length Network prefix length
 * @return Broadcast address
 */
inline IpAddress get_broadcast_address(IpAddress network, uint8_t prefix_length) {
    uint32_t host_bits = 32 - prefix_length;
    uint32_t host_mask = (host_bits == 0) ? 0 : ((1U << host_bits) - 1);
    return network | htonl(host_mask);
}

/**
 * @brief Get DHCP message type name
 * @param type DHCP message type
 * @return Message type name
 */
inline std::string get_message_type_name(DhcpMessageType type) {
    switch (type) {
        case DhcpMessageType::DISCOVER: return "DISCOVER";
        case DhcpMessageType::OFFER: return "OFFER";
        case DhcpMessageType::REQUEST: return "REQUEST";
        case DhcpMessageType::DECLINE: return "DECLINE";
        case DhcpMessageType::ACK: return "ACK";
        case DhcpMessageType::NAK: return "NAK";
        case DhcpMessageType::RELEASE: return "RELEASE";
        case DhcpMessageType::INFORM: return "INFORM";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Get DHCP option name
 * @param code DHCP option code
 * @return Option name
 */
inline std::string get_option_name(DhcpOptionCode code) {
    switch (code) {
        case DhcpOptionCode::SUBNET_MASK: return "SUBNET_MASK";
        case DhcpOptionCode::ROUTER: return "ROUTER";
        case DhcpOptionCode::DOMAIN_SERVER: return "DOMAIN_SERVER";
        case DhcpOptionCode::DOMAIN_NAME: return "DOMAIN_NAME";
        case DhcpOptionCode::REQUESTED_IP_ADDRESS: return "REQUESTED_IP_ADDRESS";
        case DhcpOptionCode::IP_ADDRESS_LEASE_TIME: return "IP_ADDRESS_LEASE_TIME";
        case DhcpOptionCode::DHCP_MESSAGE_TYPE: return "DHCP_MESSAGE_TYPE";
        case DhcpOptionCode::SERVER_IDENTIFIER: return "SERVER_IDENTIFIER";
        case DhcpOptionCode::PARAMETER_REQUEST_LIST: return "PARAMETER_REQUEST_LIST";
        case DhcpOptionCode::MESSAGE: return "MESSAGE";
        case DhcpOptionCode::RENEWAL_TIME: return "RENEWAL_TIME";
        case DhcpOptionCode::REBINDING_TIME: return "REBINDING_TIME";
        case DhcpOptionCode::VENDOR_CLASS_IDENTIFIER: return "VENDOR_CLASS_IDENTIFIER";
        case DhcpOptionCode::CLIENT_IDENTIFIER: return "CLIENT_IDENTIFIER";
        case DhcpOptionCode::END: return "END";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert DHCP message type to option value
 * @param type DHCP message type
 * @return Option value
 */
inline uint8_t message_type_to_option_value(DhcpMessageType type) {
    return static_cast<uint8_t>(type);
}

/**
 * @brief Convert option value to DHCP message type
 * @param value Option value
 * @return DHCP message type
 */
inline DhcpMessageType option_value_to_message_type(uint8_t value) {
    return static_cast<DhcpMessageType>(value);
}

/**
 * @brief Check if MAC address is valid
 * @param mac MAC address to check
 * @return true if MAC address is valid
 */
inline bool is_valid_mac(const MacAddress& mac) {
    // Check if MAC is not all zeros or all ones
    bool all_zeros = true;
    bool all_ones = true;
    
    for (uint8_t byte : mac) {
        if (byte != 0x00) all_zeros = false;
        if (byte != 0xFF) all_ones = false;
    }
    
    return !all_zeros && !all_ones;
}

/**
 * @brief Check if IP address is valid
 * @param ip IP address to check
 * @return true if IP address is valid
 */
inline bool is_valid_ip(IpAddress ip) {
    // Check if IP is not 0.0.0.0 or 255.255.255.255
    return ip != 0 && ip != 0xFFFFFFFF;
}

/**
 * @brief Get current timestamp
 * @return Current timestamp in seconds since epoch
 */
inline std::chrono::system_clock::time_point get_current_time() {
    return std::chrono::system_clock::now();
}

/**
 * @brief Calculate lease end time
 * @param start_time Lease start time
 * @param lease_duration Lease duration in seconds
 * @return Lease end time
 */
inline std::chrono::system_clock::time_point calculate_lease_end(
    const std::chrono::system_clock::time_point& start_time,
    uint32_t lease_duration) {
    return start_time + std::chrono::seconds(lease_duration);
}

/**
 * @brief Calculate renewal time (50% of lease duration)
 * @param start_time Lease start time
 * @param lease_duration Lease duration in seconds
 * @return Renewal time
 */
inline std::chrono::system_clock::time_point calculate_renewal_time(
    const std::chrono::system_clock::time_point& start_time,
    uint32_t lease_duration) {
    return start_time + std::chrono::seconds(lease_duration / 2);
}

/**
 * @brief Calculate rebinding time (87.5% of lease duration)
 * @param start_time Lease start time
 * @param lease_duration Lease duration in seconds
 * @return Rebinding time
 */
inline std::chrono::system_clock::time_point calculate_rebinding_time(
    const std::chrono::system_clock::time_point& start_time,
    uint32_t lease_duration) {
    return start_time + std::chrono::seconds((lease_duration * 7) / 8);
}

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_DHCP_UTILS_HPP
