/**
 * @file dhcp_security_manager.hpp
 * @brief DHCP security features including snooping, filtering, and validation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_DHCP_SECURITY_MANAGER_HPP
#define SIMPLE_DHCPD_DHCP_SECURITY_MANAGER_HPP

#include "dhcp_types.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <set>
#include <functional>

namespace simple_dhcpd {

/**
 * @brief Security threat level
 */
enum class ThreatLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

/**
 * @brief Security event type
 */
enum class SecurityEventType {
    UNAUTHORIZED_DHCP_SERVER,
    MAC_SPOOFING,
    IP_SPOOFING,
    RATE_LIMIT_EXCEEDED,
    INVALID_OPTION_82,
    SUSPICIOUS_ACTIVITY,
    LEASE_CONFLICT,
    UNAUTHORIZED_CLIENT
};

/**
 * @brief Security event
 */
struct SecurityEvent {
    SecurityEventType type;
    ThreatLevel level;
    std::string description;
    std::string client_mac;
    std::string client_ip;
    std::string source_interface;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> additional_data;
    
    SecurityEvent(SecurityEventType t, ThreatLevel l, const std::string& desc,
                  const std::string& mac = "", const std::string& ip = "",
                  const std::string& iface = "")
        : type(t), level(l), description(desc), client_mac(mac), client_ip(ip),
          source_interface(iface), timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief MAC address filter rule
 */
struct MacFilterRule {
    std::string mac_address;
    std::string mac_mask;
    bool allow;
    std::string description;
    std::chrono::system_clock::time_point created;
    std::chrono::system_clock::time_point expires;
    bool enabled;
    
    MacFilterRule() = default;
    MacFilterRule(const std::string& mac, bool a, const std::string& desc = "",
                  std::chrono::system_clock::time_point exp = std::chrono::system_clock::time_point::max())
        : mac_address(mac), allow(a), description(desc), 
          created(std::chrono::system_clock::now()), expires(exp), enabled(true) {}
};

/**
 * @brief IP address filter rule
 */
struct IpFilterRule {
    IpAddress ip_address;
    IpAddress ip_mask;
    bool allow;
    std::string description;
    std::chrono::system_clock::time_point created;
    std::chrono::system_clock::time_point expires;
    bool enabled;
    
    IpFilterRule() = default;
    IpFilterRule(const IpAddress& ip, const IpAddress& mask, bool a, 
                 const std::string& desc = "",
                 std::chrono::system_clock::time_point exp = std::chrono::system_clock::time_point::max())
        : ip_address(ip), ip_mask(mask), allow(a), description(desc),
          created(std::chrono::system_clock::now()), expires(exp), enabled(true) {}
};

/**
 * @brief Rate limiting rule
 */
struct RateLimitRule {
    std::string identifier;        // MAC, IP, or interface
    std::string identifier_type;   // "mac", "ip", "interface"
    size_t max_requests;
    std::chrono::seconds time_window;
    std::chrono::seconds block_duration;
    std::chrono::system_clock::time_point expires;
    bool enabled;
    
    RateLimitRule() = default;
    RateLimitRule(const std::string& id, const std::string& type, size_t max_req,
                  std::chrono::seconds window, std::chrono::seconds block = std::chrono::seconds(300))
        : identifier(id), identifier_type(type), max_requests(max_req),
          time_window(window), block_duration(block), 
          expires(std::chrono::system_clock::time_point::max()), enabled(true) {}
};

/**
 * @brief DHCP snooping binding
 */
struct DhcpSnoopingBinding {
    std::string mac_address;
    IpAddress ip_address;
    std::string interface;
    std::chrono::system_clock::time_point binding_time;
    std::chrono::seconds lease_time;
    bool trusted;
    
    DhcpSnoopingBinding() = default;
    DhcpSnoopingBinding(const std::string& mac, const IpAddress& ip, 
                        const std::string& iface, std::chrono::seconds lease,
                        bool trust = false)
        : mac_address(mac), ip_address(ip), interface(iface),
          binding_time(std::chrono::system_clock::now()), lease_time(lease), trusted(trust) {}
};

/**
 * @brief Security statistics
 */
struct SecurityStats {
    size_t total_events;
    size_t events_by_level[4];  // LOW, MEDIUM, HIGH, CRITICAL
    size_t events_by_type[8];   // All SecurityEventType values
    size_t blocked_requests;
    size_t allowed_requests;
    size_t rate_limit_hits;
    size_t mac_filter_hits;
    size_t ip_filter_hits;
    std::map<std::string, size_t> stats;
    std::chrono::system_clock::time_point last_reset;
};

/**
 * @brief Option 82 rule
 */
struct Option82Rule {
    std::string interface;
    bool required;
    bool enabled;
    std::chrono::system_clock::time_point created;
    std::chrono::system_clock::time_point expires;
    
    Option82Rule() = default;
    Option82Rule(const std::string& iface, bool req, bool en = true)
        : interface(iface), required(req), enabled(en),
          created(std::chrono::system_clock::now()),
          expires(std::chrono::system_clock::time_point::max()) {}
};

/**
 * @brief Trusted relay agent
 */
struct TrustedRelayAgent {
    std::string circuit_id;
    std::string remote_id;
    bool enabled;
    std::chrono::system_clock::time_point created_at;
    
    TrustedRelayAgent() = default;
    TrustedRelayAgent(const std::string& circuit, const std::string& remote, bool en = true)
        : circuit_id(circuit), remote_id(remote), enabled(en),
          created_at(std::chrono::system_clock::now()) {}
};

/**
 * @brief Client credentials
 */
struct ClientCredentials {
    std::string password_hash;
    std::string salt;
    bool enabled;
    std::chrono::system_clock::time_point expires;
    
    ClientCredentials() = default;
    ClientCredentials(const std::string& hash, const std::string& s, bool en = true)
        : password_hash(hash), salt(s), enabled(en),
          expires(std::chrono::system_clock::time_point::max()) {}
};

/**
 * @brief DHCP security manager
 */
class DhcpSecurityManager {
public:
    /**
     * @brief Constructor
     */
    DhcpSecurityManager();
    
    /**
     * @brief Destructor
     */
    ~DhcpSecurityManager();
    
    // DHCP snooping
    /**
     * @brief Enable DHCP snooping
     * @param enabled Enable/disable snooping
     */
    void set_dhcp_snooping_enabled(bool enabled);
    
    /**
     * @brief Check if DHCP snooping is enabled
     * @return true if enabled
     */
    bool is_dhcp_snooping_enabled() const;
    
    /**
     * @brief Add trusted interface
     * @param interface_name Interface name
     */
    void add_trusted_interface(const std::string& interface_name);
    
    /**
     * @brief Remove trusted interface
     * @param interface_name Interface name
     */
    void remove_trusted_interface(const std::string& interface_name);
    
    /**
     * @brief Check if interface is trusted
     * @param interface_name Interface name
     * @return true if trusted
     */
    bool is_interface_trusted(const std::string& interface_name) const;
    
    /**
     * @brief Add snooping binding
     * @param binding Snooping binding
     */
    void add_snooping_binding(const DhcpSnoopingBinding& binding);
    
    /**
     * @brief Remove snooping binding
     * @param mac_address MAC address
     * @param ip_address IP address
     */
    void remove_snooping_binding(const std::string& mac_address, const IpAddress& ip_address);
    
    /**
     * @brief Get snooping bindings
     * @return Vector of snooping bindings
     */
    std::vector<DhcpSnoopingBinding> get_snooping_bindings();
    
    /**
     * @brief Validate DHCP message against snooping bindings
     * @param message DHCP message
     * @param source_interface Source interface
     * @return true if valid
     */
    bool validate_dhcp_message(const DhcpMessage& message, const std::string& source_interface);
    
    // MAC address filtering
    /**
     * @brief Add MAC filter rule
     * @param rule MAC filter rule
     */
    void add_mac_filter_rule(const MacFilterRule& rule);
    
    /**
     * @brief Remove MAC filter rule
     * @param mac_address MAC address
     */
    void remove_mac_filter_rule(const std::string& mac_address);
    
    /**
     * @brief Check MAC address against filters
     * @param mac_address MAC address to check
     * @return true if allowed
     */
    bool check_mac_address(const std::string& mac_address);
    
    /**
     * @brief Get MAC filter rules
     * @return Vector of MAC filter rules
     */
    std::vector<MacFilterRule> get_mac_filter_rules();
    
    // IP address filtering
    /**
     * @brief Add IP filter rule
     * @param rule IP filter rule
     */
    void add_ip_filter_rule(const IpFilterRule& rule);
    
    /**
     * @brief Remove IP filter rule
     * @param ip_address IP address
     */
    void remove_ip_filter_rule(const IpAddress& ip_address);
    
    /**
     * @brief Check IP address against filters
     * @param ip_address IP address to check
     * @return true if allowed
     */
    bool check_ip_address(const IpAddress& ip_address);
    
    /**
     * @brief Get IP filter rules
     * @return Vector of IP filter rules
     */
    std::vector<IpFilterRule> get_ip_filter_rules();
    
    // Rate limiting
    /**
     * @brief Add rate limit rule
     * @param rule Rate limit rule
     */
    void add_rate_limit_rule(const RateLimitRule& rule);
    
    /**
     * @brief Remove rate limit rule
     * @param identifier Identifier
     * @param identifier_type Identifier type
     */
    void remove_rate_limit_rule(const std::string& identifier, const std::string& identifier_type);
    
    /**
     * @brief Check rate limit
     * @param identifier Identifier to check
     * @param identifier_type Identifier type
     * @return true if within limits
     */
    bool check_rate_limit(const std::string& identifier, const std::string& identifier_type);
    
    /**
     * @brief Get rate limit rules
     * @return Vector of rate limit rules
     */
    std::vector<RateLimitRule> get_rate_limit_rules();
    
    // Option 82 support
    /**
     * @brief Enable Option 82 validation
     * @param enabled Enable/disable validation
     */
    void set_option_82_validation_enabled(bool enabled);
    
    /**
     * @brief Check if Option 82 validation is enabled
     * @return true if enabled
     */
    bool is_option_82_validation_enabled() const;
    
    /**
     * @brief Validate Option 82
     * @param option_82_data Option 82 data
     * @param source_interface Source interface
     * @return true if valid
     */
    bool validate_option_82(const std::vector<uint8_t>& option_82_data, 
                           const std::string& source_interface);
    
    /**
     * @brief Manage Option 82 interface rules
     */
    void add_option_82_rule(const Option82Rule& rule);
    void clear_option_82_rules();
    std::vector<Option82Rule> get_option_82_rules() const;
    
    /**
     * @brief Convenience: set Option82 required flag for an interface
     */
    void set_option_82_required_for_interface(const std::string& interface, bool required);
    
    /**
     * @brief Add trusted relay agent
     * @param circuit_id Circuit ID
     * @param remote_id Remote ID
     */
    void add_trusted_relay_agent(const std::string& circuit_id, const std::string& remote_id);
    
    /**
     * @brief Remove trusted relay agent
     * @param circuit_id Circuit ID
     * @param remote_id Remote ID
     */
    void remove_trusted_relay_agent(const std::string& circuit_id, const std::string& remote_id);
    
    // Authentication support
    /**
     * @brief Enable authentication
     * @param enabled Enable/disable authentication
     */
    void set_authentication_enabled(bool enabled);
    
    /**
     * @brief Check if authentication is enabled
     * @return true if enabled
     */
    bool is_authentication_enabled() const;
    
    /**
     * @brief Set authentication key
     * @param key Authentication key
     */
    void set_authentication_key(const std::string& key);
    
    /**
     * @brief Validate client authentication
     * @param client_mac Client MAC address
     * @param auth_data Authentication data
     * @return true if valid
     */
    bool validate_client_authentication(const std::string& client_mac, 
                                       const std::vector<uint8_t>& auth_data);
    
    // Security event handling
    /**
     * @brief Report security event
     * @param event Security event
     */
    void report_security_event(const SecurityEvent& event);
    
    /**
     * @brief Set security event callback
     * @param callback Callback function for security events
     */
    void set_security_event_callback(std::function<void(const SecurityEvent&)> callback);
    
    /**
     * @brief Get security events
     * @param start_time Start time filter
     * @param end_time End time filter
     * @param event_type Event type filter
     * @return Vector of security events
     */
    std::vector<SecurityEvent> get_security_events(
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::time_point::min(),
        std::chrono::system_clock::time_point end_time = std::chrono::system_clock::time_point::max(),
        SecurityEventType event_type = SecurityEventType::UNAUTHORIZED_DHCP_SERVER);
    
    /**
     * @brief Get security statistics
     * @return Security statistics
     */
    SecurityStats get_security_statistics() const;
    
    /**
     * @brief Clear security statistics
     */
    void clear_security_statistics();
    
    // Configuration management
    /**
     * @brief Load security configuration
     * @param config_file Configuration file path
     * @return true if loaded successfully
     */
    bool load_security_configuration(const std::string& config_file);
    
    /**
     * @brief Save security configuration
     * @param config_file Configuration file path
     * @return true if saved successfully
     */
    bool save_security_configuration(const std::string& config_file);
    
    /**
     * @brief Start security manager
     */
    void start();
    
    /**
     * @brief Stop security manager
     */
    void stop();

private:
    std::atomic<bool> dhcp_snooping_enabled_;
    std::atomic<bool> option_82_validation_enabled_;
    std::atomic<bool> authentication_enabled_;
    std::atomic<bool> running_;
    
    std::set<std::string> trusted_interfaces_;
    std::vector<DhcpSnoopingBinding> snooping_bindings_;
    std::vector<MacFilterRule> mac_filter_rules_;
    std::vector<IpFilterRule> ip_filter_rules_;
    std::vector<RateLimitRule> rate_limit_rules_;
    std::vector<Option82Rule> option_82_rules_;
    std::vector<TrustedRelayAgent> trusted_relay_agents_;
    std::map<std::string, ClientCredentials> client_credentials_;
    
    std::string authentication_key_;
    std::vector<SecurityEvent> security_events_;
    SecurityStats security_stats_;
    
    std::function<void(const SecurityEvent&)> security_event_callback_;
    
    mutable std::mutex mutex_;
    std::thread cleanup_thread_;
    
    // Rate limiting tracking
    struct RateLimitTracker {
        std::vector<std::chrono::system_clock::time_point> requests;
        std::chrono::system_clock::time_point window_start;
        std::chrono::system_clock::time_point blocked_until;
    };
    std::map<std::string, RateLimitTracker> rate_limit_trackers_;
    
    // Helper for updating security statistics
    void update_security_stats(const std::string& stat_name);
    
    /**
     * @brief Cleanup expired rules and bindings
     */
    void cleanup_expired_items();
    
    /**
     * @brief Cleanup thread function
     */
    void cleanup_worker();
    
    /**
     * @brief Check if MAC address matches filter rule
     * @param mac_address MAC address
     * @param rule Filter rule
     * @return true if matches
     */
    bool mac_matches_rule(const std::string& mac_address, const MacFilterRule& rule);
    
    /**
     * @brief Check if IP address matches filter rule
     * @param ip_address IP address
     * @param rule Filter rule
     * @return true if matches
     */
    bool ip_matches_rule(const IpAddress& ip_address, const IpFilterRule& rule);
    
    /**
     * @brief Update rate limit tracker
     * @param identifier Identifier
     * @param identifier_type Identifier type
     * @return true if within limits
     */
    bool update_rate_limit_tracker(const std::string& identifier, const std::string& identifier_type);
    
    /**
     * @brief Generate authentication hash
     * @param client_mac Client MAC address
     * @param timestamp Timestamp
     * @return Authentication hash
     */
    std::string generate_auth_hash(const std::string& client_mac, 
                                  std::chrono::system_clock::time_point timestamp);
    
    /**
     * @brief Validate authentication hash
     * @param client_mac Client MAC address
     * @param auth_data Authentication data
     * @param timestamp Timestamp
     * @return true if valid
     */
    bool validate_auth_hash(const std::string& client_mac, 
                           const std::vector<uint8_t>& auth_data,
                           std::chrono::system_clock::time_point timestamp);
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_DHCP_SECURITY_MANAGER_HPP
