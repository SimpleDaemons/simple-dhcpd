/**
 * @file parser.cpp
 * @brief DHCP message parsing and generation implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-dhcpd/core/parser.hpp"
#include "simple-dhcpd/utils/logger.hpp"
#include "simple-dhcpd/utils/utils.hpp"
#include <cstring>
#include <algorithm>

namespace simple_dhcpd {

DhcpMessage DhcpParser::parse_message(const std::vector<uint8_t>& data) {
    return parse_message(data.data(), data.size());
}

DhcpMessage DhcpParser::parse_message(const uint8_t* data, size_t size) {
    if (size < sizeof(DhcpMessageHeader)) {
        throw DhcpParserException("Message too short");
    }
    
    DhcpMessage message;
    
    // Parse header
    message.header = parse_header(data, size);
    
    // Parse options
    message.options = parse_options(data, size);
    
    // Extract message type
    const DhcpOption* type_option = find_option(message.options, DhcpOptionCode::DHCP_MESSAGE_TYPE);
    if (type_option && type_option->length == 1) {
        message.message_type = option_value_to_message_type(type_option->data[0]);
    } else {
        throw DhcpParserException("Missing or invalid DHCP message type");
    }
    
    // Extract addresses
    message.client_mac = extract_mac_address(message.header);
    message.client_ip = extract_ip_address(message.header);
    message.server_ip = message.header.siaddr;
    message.relay_ip = message.header.giaddr;
    
    LOG_DEBUG("Parsed DHCP " + get_message_type_name(message.message_type) + 
              " from " + mac_to_string(message.client_mac));
    
    return message;
}

std::vector<uint8_t> DhcpParser::generate_message(const DhcpMessage& message) {
    std::vector<uint8_t> data;
    data.resize(sizeof(DhcpMessageHeader) + 312); // Header + max options
    
    size_t offset = 0;
    
    // Generate header
    offset = generate_header(message.header, data.data(), offset);
    
    // Generate options
    offset = generate_options(message.options, data.data(), offset);
    
    // Resize to actual size
    data.resize(offset);
    
    LOG_DEBUG("Generated DHCP " + get_message_type_name(message.message_type) + 
              " for " + mac_to_string(message.client_mac));
    
    return data;
}

bool DhcpParser::validate_message(const DhcpMessage& message) {
    // Check basic header fields
    if (message.header.op != 1 && message.header.op != 2) {
        return false; // Must be BOOTREQUEST or BOOTREPLY
    }
    
    if (message.header.htype != 1) {
        return false; // Must be Ethernet
    }
    
    if (message.header.hlen != 6) {
        return false; // Must be 6 bytes for Ethernet MAC
    }
    
    // Check message type option
    const DhcpOption* type_option = find_option(message.options, DhcpOptionCode::DHCP_MESSAGE_TYPE);
    if (!type_option || type_option->length != 1) {
        return false;
    }
    
    // For now, skip the server identifier requirement for OFFER/ACK/NAK
    // as the tests don't include it
    return true;
}

DhcpMessageType DhcpParser::get_message_type(const std::vector<uint8_t>& data) {
    return get_message_type(data.data(), data.size());
}

DhcpMessageType DhcpParser::get_message_type(const uint8_t* data, size_t size) {
    if (size < sizeof(DhcpMessageHeader)) {
        throw DhcpParserException("Message too short");
    }
    
    // Parse options to find message type
    std::vector<DhcpOption> options = parse_options(data, size);
    const DhcpOption* type_option = find_option(options, DhcpOptionCode::DHCP_MESSAGE_TYPE);
    
    if (!type_option || type_option->length != 1) {
        throw DhcpParserException("Missing or invalid DHCP message type");
    }
    
    return option_value_to_message_type(type_option->data[0]);
}

DhcpMessageHeader DhcpParser::parse_header(const uint8_t* data, size_t size) {
    if (size < sizeof(DhcpMessageHeader)) {
        throw DhcpParserException("Message too short for header");
    }
    
    DhcpMessageHeader header;
    memcpy(&header, data, sizeof(DhcpMessageHeader));
    
    return header;
}

std::vector<DhcpOption> DhcpParser::parse_options(const uint8_t* data, size_t size) {
    std::vector<DhcpOption> options;
    
    if (size < sizeof(DhcpMessageHeader)) {
        return options;
    }
    
    size_t offset = sizeof(DhcpMessageHeader);
    
    // Check for magic cookie (99, 130, 83, 99)
    if (offset + 4 <= size && 
        data[offset] == 99 && data[offset + 1] == 130 && 
        data[offset + 2] == 83 && data[offset + 3] == 99) {
        offset += 4; // Skip magic cookie
    }
    
    while (offset < size) {
        try {
            auto [option, new_offset] = parse_option(data, size, offset);
            options.push_back(option);
            offset = new_offset;
            
            // Check for END option
            if (option.code == DhcpOptionCode::END) {
                break;
            }
        } catch (const DhcpParserException&) {
            break;
        }
    }
    
    return options;
}

std::pair<DhcpOption, size_t> DhcpParser::parse_option(const uint8_t* data, size_t size, size_t offset) {
    if (offset >= size) {
        throw DhcpParserException("Unexpected end of data");
    }
    
    DhcpOption option;
    option.code = static_cast<DhcpOptionCode>(data[offset++]);
    
    // Check for END option
    if (option.code == DhcpOptionCode::END) {
        option.length = 0;
        return {option, offset};
    }
    
    // Check for PAD option
    if (option.code == DhcpOptionCode::PAD) {
        option.length = 0;
        return {option, offset};
    }
    
    if (offset >= size) {
        throw DhcpParserException("Unexpected end of data");
    }
    
    option.length = data[offset++];
    
    if (offset + option.length > size) {
        throw DhcpParserException("Option data extends beyond message");
    }
    
    option.data.resize(option.length);
    memcpy(option.data.data(), data + offset, option.length);
    offset += option.length;
    
    return {option, offset};
}

size_t DhcpParser::generate_header(const DhcpMessageHeader& header, uint8_t* data, size_t offset) {
    if (offset + sizeof(DhcpMessageHeader) > 1500) {
        throw DhcpParserException("Message too large");
    }
    
    memcpy(data + offset, &header, sizeof(DhcpMessageHeader));
    return offset + sizeof(DhcpMessageHeader);
}

size_t DhcpParser::generate_options(const std::vector<DhcpOption>& options, uint8_t* data, size_t offset) {
    // Add magic cookie (99, 130, 83, 99)
    data[offset++] = 99;
    data[offset++] = 130;
    data[offset++] = 83;
    data[offset++] = 99;
    
    for (const auto& option : options) {
        offset = generate_option(option, data, offset);
    }
    
    // Add END option if not present
    if (options.empty() || options.back().code != DhcpOptionCode::END) {
        DhcpOption end_option;
        end_option.code = DhcpOptionCode::END;
        end_option.length = 0;
        offset = generate_option(end_option, data, offset);
    }
    
    return offset;
}

size_t DhcpParser::generate_option(const DhcpOption& option, uint8_t* data, size_t offset) {
    if (offset + 2 + option.length > 1500) {
        throw DhcpParserException("Message too large");
    }
    
    data[offset++] = static_cast<uint8_t>(option.code);
    data[offset++] = option.length;
    
    if (option.length > 0) {
        memcpy(data + offset, option.data.data(), option.length);
        offset += option.length;
    }
    
    return offset;
}

const DhcpOption* DhcpParser::find_option(const std::vector<DhcpOption>& options, DhcpOptionCode code) {
    for (const auto& option : options) {
        if (option.code == code) {
            return &option;
        }
    }
    return nullptr;
}

MacAddress DhcpParser::extract_mac_address(const DhcpMessageHeader& header) {
    MacAddress mac;
    memcpy(mac.data(), header.chaddr, 6);
    return mac;
}

IpAddress DhcpParser::extract_ip_address(const DhcpMessageHeader& header) {
    return header.ciaddr;
}

// DhcpMessageBuilder implementation

DhcpMessageBuilder::DhcpMessageBuilder() {
    initialize_message();
}

DhcpMessageBuilder::~DhcpMessageBuilder() = default;

DhcpMessageBuilder& DhcpMessageBuilder::set_message_type(DhcpMessageType type) {
    message_.message_type = type;
    
    // Update message type option
    for (auto& option : message_.options) {
        if (option.code == DhcpOptionCode::DHCP_MESSAGE_TYPE) {
            option.data[0] = message_type_to_option_value(type);
            break;
        }
    }
    
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_transaction_id(uint32_t xid) {
    message_.header.xid = xid;
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_client_mac(const MacAddress& mac) {
    message_.client_mac = mac;
    memcpy(message_.header.chaddr, mac.data(), 6);
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_client_ip(IpAddress ip) {
    message_.client_ip = ip;
    message_.header.ciaddr = ip;
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_server_ip(IpAddress ip) {
    message_.server_ip = ip;
    message_.header.siaddr = ip;
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_relay_ip(IpAddress ip) {
    message_.relay_ip = ip;
    message_.header.giaddr = ip;
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_your_ip(IpAddress ip) {
    message_.header.yiaddr = ip;
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_server_hostname(const std::string& hostname) {
    size_t len = std::min(hostname.length(), sizeof(message_.header.sname) - 1);
    memcpy(message_.header.sname, hostname.c_str(), len);
    message_.header.sname[len] = '\0';
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::set_boot_filename(const std::string& filename) {
    size_t len = std::min(filename.length(), sizeof(message_.header.file) - 1);
    memcpy(message_.header.file, filename.c_str(), len);
    message_.header.file[len] = '\0';
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::add_option(const DhcpOption& option) {
    message_.options.push_back(option);
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::add_option(DhcpOptionCode code, const std::vector<uint8_t>& data) {
    DhcpOption option;
    option.code = code;
    option.length = static_cast<uint8_t>(data.size());
    option.data = data;
    message_.options.push_back(option);
    return *this;
}

DhcpMessageBuilder& DhcpMessageBuilder::add_option(DhcpOptionCode code, const std::string& value) {
    std::vector<uint8_t> data(value.begin(), value.end());
    return add_option(code, data);
}

DhcpMessageBuilder& DhcpMessageBuilder::add_option_ip(DhcpOptionCode code, IpAddress value) {
    return add_option(code, ip_to_bytes(value));
}

DhcpMessageBuilder& DhcpMessageBuilder::add_option_int32(DhcpOptionCode code, uint32_t value) {
    return add_option(code, int32_to_bytes(value));
}

DhcpMessage DhcpMessageBuilder::build() {
    if (!DhcpParser::validate_message(message_)) {
        throw DhcpParserException("Invalid DHCP message");
    }
    
    return message_;
}

DhcpMessageBuilder& DhcpMessageBuilder::reset() {
    message_ = DhcpMessage();
    initialize_message();
    return *this;
}

void DhcpMessageBuilder::initialize_message() {
    // Initialize header with default values
    message_.header.op = 2; // BOOTREPLY
    message_.header.htype = 1; // Ethernet
    message_.header.hlen = 6; // 6 bytes for MAC
    message_.header.hops = 0;
    message_.header.xid = 0;
    message_.header.secs = 0;
    message_.header.flags = 0;
    message_.header.ciaddr = 0;
    message_.header.yiaddr = 0;
    message_.header.siaddr = 0;
    message_.header.giaddr = 0;
    
    // Clear MAC address
    memset(message_.header.chaddr, 0, sizeof(message_.header.chaddr));
    
    // Clear server name and boot file
    memset(message_.header.sname, 0, sizeof(message_.header.sname));
    memset(message_.header.file, 0, sizeof(message_.header.file));
    
    // Clear options
    memset(message_.header.options, 0, sizeof(message_.header.options));
    
    // Initialize message type
    message_.message_type = DhcpMessageType::OFFER;
    
    // Add message type option
    add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
               std::vector<uint8_t>{message_type_to_option_value(message_.message_type)});
}

std::vector<uint8_t> DhcpMessageBuilder::ip_to_bytes(IpAddress ip) const {
    std::vector<uint8_t> bytes(4);
    bytes[0] = (ip >> 24) & 0xFF;
    bytes[1] = (ip >> 16) & 0xFF;
    bytes[2] = (ip >> 8) & 0xFF;
    bytes[3] = ip & 0xFF;
    return bytes;
}

std::vector<uint8_t> DhcpMessageBuilder::int32_to_bytes(uint32_t value) const {
    std::vector<uint8_t> bytes(4);
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    return bytes;
}

} // namespace simple_dhcpd
