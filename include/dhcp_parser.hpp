/**
 * @file dhcp_parser.hpp
 * @brief DHCP message parsing and generation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_DHCP_PARSER_HPP
#define SIMPLE_DHCPD_DHCP_PARSER_HPP

#include "dhcp_types.hpp"
#include <vector>
#include <string>

namespace simple_dhcpd {

/**
 * @brief DHCP parser exception
 */
class DhcpParserException : public std::exception {
public:
    explicit DhcpParserException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

/**
 * @brief DHCP message parser class
 */
class DhcpParser {
public:
    /**
     * @brief Parse DHCP message from raw data
     * @param data Raw DHCP data
     * @return Parsed DHCP message
     * @throws DhcpParserException if parsing fails
     */
    static DhcpMessage parse_message(const std::vector<uint8_t>& data);
    
    /**
     * @brief Parse DHCP message from raw data
     * @param data Raw DHCP data
     * @param size Data size
     * @return Parsed DHCP message
     * @throws DhcpParserException if parsing fails
     */
    static DhcpMessage parse_message(const uint8_t* data, size_t size);
    
    /**
     * @brief Generate DHCP message to raw data
     * @param message DHCP message to generate
     * @return Raw DHCP data
     * @throws DhcpParserException if generation fails
     */
    static std::vector<uint8_t> generate_message(const DhcpMessage& message);
    
    /**
     * @brief Validate DHCP message
     * @param message DHCP message to validate
     * @return true if message is valid
     */
    static bool validate_message(const DhcpMessage& message);
    
    /**
     * @brief Get DHCP message type from raw data
     * @param data Raw DHCP data
     * @return DHCP message type
     * @throws DhcpParserException if parsing fails
     */
    static DhcpMessageType get_message_type(const std::vector<uint8_t>& data);
    
    /**
     * @brief Get DHCP message type from raw data
     * @param data Raw DHCP data
     * @param size Data size
     * @return DHCP message type
     * @throws DhcpParserException if parsing fails
     */
    static DhcpMessageType get_message_type(const uint8_t* data, size_t size);
    
    /**
     * @brief Find option in options list
     * @param options Options list
     * @param code Option code to find
     * @return Pointer to option if found, nullptr otherwise
     */
    static const DhcpOption* find_option(const std::vector<DhcpOption>& options, DhcpOptionCode code);

private:
    /**
     * @brief Parse DHCP header from raw data
     * @param data Raw DHCP data
     * @param size Data size
     * @return Parsed DHCP header
     * @throws DhcpParserException if parsing fails
     */
    static DhcpMessageHeader parse_header(const uint8_t* data, size_t size);
    
    /**
     * @brief Parse DHCP options from raw data
     * @param data Raw DHCP data
     * @param size Data size
     * @return Parsed DHCP options
     * @throws DhcpParserException if parsing fails
     */
    static std::vector<DhcpOption> parse_options(const uint8_t* data, size_t size);
    
    /**
     * @brief Parse single DHCP option
     * @param data Raw DHCP data
     * @param size Data size
     * @param offset Current offset
     * @return Parsed DHCP option and new offset
     * @throws DhcpParserException if parsing fails
     */
    static std::pair<DhcpOption, size_t> parse_option(const uint8_t* data, size_t size, size_t offset);
    
    /**
     * @brief Generate DHCP header to raw data
     * @param header DHCP header to generate
     * @param data Output buffer
     * @param offset Current offset
     * @return New offset
     */
    static size_t generate_header(const DhcpMessageHeader& header, uint8_t* data, size_t offset);
    
    /**
     * @brief Generate DHCP options to raw data
     * @param options DHCP options to generate
     * @param data Output buffer
     * @param offset Current offset
     * @return New offset
     */
    static size_t generate_options(const std::vector<DhcpOption>& options, uint8_t* data, size_t offset);
    
    /**
     * @brief Generate single DHCP option
     * @param option DHCP option to generate
     * @param data Output buffer
     * @param offset Current offset
     * @return New offset
     */
    static size_t generate_option(const DhcpOption& option, uint8_t* data, size_t offset);
    
    /**
     * @brief Extract MAC address from DHCP header
     * @param header DHCP header
     * @return MAC address
     */
    static MacAddress extract_mac_address(const DhcpMessageHeader& header);
    
    /**
     * @brief Extract IP address from DHCP header
     * @param header DHCP header
     * @return IP address
     */
    static IpAddress extract_ip_address(const DhcpMessageHeader& header);
};

/**
 * @brief DHCP message builder class
 */
class DhcpMessageBuilder {
public:
    /**
     * @brief Constructor
     */
    DhcpMessageBuilder();
    
    /**
     * @brief Destructor
     */
    ~DhcpMessageBuilder();
    
    /**
     * @brief Set message type
     * @param type DHCP message type
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_message_type(DhcpMessageType type);
    
    /**
     * @brief Set transaction ID
     * @param xid Transaction ID
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_transaction_id(uint32_t xid);
    
    /**
     * @brief Set client MAC address
     * @param mac Client MAC address
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_client_mac(const MacAddress& mac);
    
    /**
     * @brief Set client IP address
     * @param ip Client IP address
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_client_ip(IpAddress ip);
    
    /**
     * @brief Set server IP address
     * @param ip Server IP address
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_server_ip(IpAddress ip);
    
    /**
     * @brief Set relay IP address
     * @param ip Relay IP address
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_relay_ip(IpAddress ip);
    
    /**
     * @brief Set your IP address
     * @param ip Your IP address
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_your_ip(IpAddress ip);
    
    /**
     * @brief Set server hostname
     * @param hostname Server hostname
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_server_hostname(const std::string& hostname);
    
    /**
     * @brief Set boot filename
     * @param filename Boot filename
     * @return Reference to builder
     */
    DhcpMessageBuilder& set_boot_filename(const std::string& filename);
    
    /**
     * @brief Add DHCP option
     * @param option DHCP option to add
     * @return Reference to builder
     */
    DhcpMessageBuilder& add_option(const DhcpOption& option);
    
    /**
     * @brief Add DHCP option
     * @param code Option code
     * @param data Option data
     * @return Reference to builder
     */
    DhcpMessageBuilder& add_option(DhcpOptionCode code, const std::vector<uint8_t>& data);
    
    /**
     * @brief Add DHCP option
     * @param code Option code
     * @param value Option value as string
     * @return Reference to builder
     */
    DhcpMessageBuilder& add_option(DhcpOptionCode code, const std::string& value);
    
    /**
     * @brief Add DHCP option
     * @param code Option code
     * @param value Option value as IP address
     * @return Reference to builder
     */
    DhcpMessageBuilder& add_option_ip(DhcpOptionCode code, IpAddress value);
    
    /**
     * @brief Add DHCP option
     * @param code Option code
     * @param value Option value as 32-bit integer
     * @return Reference to builder
     */
    DhcpMessageBuilder& add_option_int32(DhcpOptionCode code, uint32_t value);
    
    /**
     * @brief Build DHCP message
     * @return Built DHCP message
     * @throws DhcpParserException if building fails
     */
    DhcpMessage build();
    
    /**
     * @brief Reset builder
     * @return Reference to builder
     */
    DhcpMessageBuilder& reset();

private:
    DhcpMessage message_;
    
    /**
     * @brief Initialize message with default values
     */
    void initialize_message();
    
    /**
     * @brief Convert IP address to bytes
     * @param ip IP address
     * @return IP address as bytes
     */
    std::vector<uint8_t> ip_to_bytes(IpAddress ip) const;
    
    /**
     * @brief Convert 32-bit integer to bytes
     * @param value 32-bit integer
     * @return Integer as bytes
     */
    std::vector<uint8_t> int32_to_bytes(uint32_t value) const;
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_DHCP_PARSER_HPP
