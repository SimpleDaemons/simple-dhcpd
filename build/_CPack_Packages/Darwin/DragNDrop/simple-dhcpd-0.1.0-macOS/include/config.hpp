/**
 * @file config.hpp
 * @brief Configuration management
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_CONFIG_HPP
#define SIMPLE_DHCPD_CONFIG_HPP

#include "dhcp_types.hpp"
#include <string>
#include <memory>
#include <exception>

namespace simple_dhcpd {

/**
 * @brief Configuration exception
 */
class ConfigException : public std::exception {
public:
    explicit ConfigException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

/**
 * @brief Configuration manager class
 */
class ConfigManager {
public:
    /**
     * @brief Constructor
     */
    ConfigManager();
    
    /**
     * @brief Destructor
     */
    ~ConfigManager();
    
    /**
     * @brief Load configuration from file
     * @param config_file Path to configuration file
     * @throws ConfigException if configuration cannot be loaded
     */
    void load_config(const std::string& config_file);
    
    /**
     * @brief Load configuration from JSON string
     * @param json_config JSON configuration string
     * @throws ConfigException if configuration cannot be parsed
     */
    void load_config_from_json(const std::string& json_config);
    
    /**
     * @brief Save configuration to file
     * @param config_file Path to configuration file
     * @throws ConfigException if configuration cannot be saved
     */
    void save_config(const std::string& config_file) const;
    
    /**
     * @brief Get current configuration
     * @return Reference to current configuration
     */
    const DhcpConfig& get_config() const;
    
    /**
     * @brief Update configuration
     * @param config New configuration
     */
    void set_config(const DhcpConfig& config);
    
    /**
     * @brief Validate configuration
     * @throws ConfigException if configuration is invalid
     */
    void validate_config() const;
    
    /**
     * @brief Reload configuration from file
     * @throws ConfigException if configuration cannot be reloaded
     */
    void reload_config();
    
    /**
     * @brief Check if configuration has been loaded
     * @return true if configuration is loaded
     */
    bool is_loaded() const;
    
    /**
     * @brief Get configuration file path
     * @return Configuration file path
     */
    std::string get_config_file() const;

private:
    DhcpConfig config_;
    std::string config_file_;
    bool loaded_;
    
    /**
     * @brief Parse JSON configuration
     * @param json_config JSON configuration string
     * @throws ConfigException if parsing fails
     */
    void parse_json_config(const std::string& json_config);
    
    /**
     * @brief Parse subnet configuration
     * @param subnet_json JSON subnet configuration
     * @return Parsed subnet configuration
     * @throws ConfigException if parsing fails
     */
    DhcpSubnet parse_subnet_config(const std::string& subnet_json);
    
    /**
     * @brief Parse DHCP option configuration
     * @param option_json JSON option configuration
     * @return Parsed DHCP option
     * @throws ConfigException if parsing fails
     */
    DhcpOption parse_option_config(const std::string& option_json);
    
    /**
     * @brief Validate subnet configuration
     * @param subnet Subnet configuration to validate
     * @throws ConfigException if validation fails
     */
    void validate_subnet_config(const DhcpSubnet& subnet) const;
    
    /**
     * @brief Validate IP address range
     * @param start Start IP address
     * @param end End IP address
     * @param network Network address
     * @param prefix_length Network prefix length
     * @throws ConfigException if validation fails
     */
    void validate_ip_range(IpAddress start, IpAddress end, 
                          IpAddress network, uint8_t prefix_length) const;
};

/**
 * @brief Get default configuration
 * @return Default configuration
 */
DhcpConfig get_default_config();

/**
 * @brief Create sample configuration file
 * @param config_file Path to configuration file
 */
void create_sample_config(const std::string& config_file);

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_CONFIG_HPP
