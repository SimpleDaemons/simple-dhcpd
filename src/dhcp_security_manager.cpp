/**
 * @file dhcp_security_manager.cpp
 * @brief DHCP security features implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "dhcp_security_manager.hpp"
#include "logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <openssl/sha.h>
#include <openssl/hmac.h>

namespace simple_dhcpd {

DhcpSecurityManager::DhcpSecurityManager() 
    : dhcp_snooping_enabled_(false), option_82_validation_enabled_(false),
      authentication_enabled_(false), running_(false) {
}

DhcpSecurityManager::~DhcpSecurityManager() {
    stop();
}

void DhcpSecurityManager::set_dhcp_snooping_enabled(bool enabled) {
    dhcp_snooping_enabled_ = enabled;
    Logger::info("DHCP snooping " + std::string(enabled ? "enabled" : "disabled"));
}

bool DhcpSecurityManager::is_dhcp_snooping_enabled() const {
    return dhcp_snooping_enabled_;
}

void DhcpSecurityManager::add_trusted_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_interfaces_.insert(interface_name);
    Logger::info("Added trusted interface: " + interface_name);
}

void DhcpSecurityManager::remove_trusted_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_interfaces_.erase(interface_name);
    Logger::info("Removed trusted interface: " + interface_name);
}

bool DhcpSecurityManager::is_interface_trusted(const std::string& interface_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return trusted_interfaces_.find(interface_name) != trusted_interfaces_.end();
}

void DhcpSecurityManager::add_snooping_binding(const DhcpSnoopingBinding& binding) {
    std::lock_guard<std::mutex> lock(mutex_);
    snooping_bindings_.push_back(binding);
    Logger::info("Added snooping binding: " + binding.mac_address + " -> " + 
                 binding.ip_address.to_string() + " on " + binding.interface);
}

void DhcpSecurityManager::remove_snooping_binding(const std::string& mac_address, const IpAddress& ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    snooping_bindings_.erase(
        std::remove_if(snooping_bindings_.begin(), snooping_bindings_.end(),
            [&](const DhcpSnoopingBinding& binding) {
                return binding.mac_address == mac_address && binding.ip_address == ip_address;
            }),
        snooping_bindings_.end());
    
    Logger::info("Removed snooping binding: " + mac_address + " -> " + ip_address.to_string());
}

std::vector<DhcpSnoopingBinding> DhcpSecurityManager::get_snooping_bindings() {
    std::lock_guard<std::mutex> lock(mutex_);
    return snooping_bindings_;
}

bool DhcpSecurityManager::validate_dhcp_message(const DhcpMessage& message, const std::string& source_interface) {
    if (!dhcp_snooping_enabled_) {
        return true;
    }
    
    // Check if interface is trusted
    if (is_interface_trusted(source_interface)) {
        return true;
    }
    
    // Validate against snooping bindings
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& binding : snooping_bindings_) {
        if (binding.mac_address == message.client_mac.to_string() && 
            binding.ip_address == message.client_ip) {
            return true;
        }
    }
    
    // Report unauthorized DHCP server
    SecurityEvent event(SecurityEventType::UNAUTHORIZED_DHCP_SERVER, ThreatLevel::HIGH,
                       "Unauthorized DHCP message from " + source_interface,
                       message.client_mac.to_string(), message.client_ip.to_string(), source_interface);
    report_security_event(event);
    
    return false;
}

void DhcpSecurityManager::add_mac_filter_rule(const MacFilterRule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    mac_filter_rules_.push_back(rule);
    Logger::info("Added MAC filter rule: " + rule.mac_address + " (" + 
                 (rule.allow ? "allow" : "deny") + ")");
}

void DhcpSecurityManager::remove_mac_filter_rule(const std::string& mac_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    mac_filter_rules_.erase(
        std::remove_if(mac_filter_rules_.begin(), mac_filter_rules_.end(),
            [&](const MacFilterRule& rule) {
                return rule.mac_address == mac_address;
            }),
        mac_filter_rules_.end());
    
    Logger::info("Removed MAC filter rule: " + mac_address);
}

bool DhcpSecurityManager::check_mac_address(const std::string& mac_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& rule : mac_filter_rules_) {
        if (!rule.enabled) {
            continue;
        }
        
        // Check if rule has expired
        if (rule.expires < std::chrono::system_clock::now()) {
            continue;
        }
        
        if (mac_matches_rule(mac_address, rule)) {
            update_security_stats(rule.allow ? "mac_allowed" : "mac_blocked");
            return rule.allow;
        }
    }
    
    // Default allow if no rules match
    update_security_stats("mac_allowed");
    return true;
}

std::vector<MacFilterRule> DhcpSecurityManager::get_mac_filter_rules() {
    std::lock_guard<std::mutex> lock(mutex_);
    return mac_filter_rules_;
}

void DhcpSecurityManager::add_ip_filter_rule(const IpFilterRule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    ip_filter_rules_.push_back(rule);
    Logger::info("Added IP filter rule: " + rule.ip_address.to_string() + " (" + 
                 (rule.allow ? "allow" : "deny") + ")");
}

void DhcpSecurityManager::remove_ip_filter_rule(const IpAddress& ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ip_filter_rules_.erase(
        std::remove_if(ip_filter_rules_.begin(), ip_filter_rules_.end(),
            [&](const IpFilterRule& rule) {
                return rule.ip_address == ip_address;
            }),
        ip_filter_rules_.end());
    
    Logger::info("Removed IP filter rule: " + ip_address.to_string());
}

bool DhcpSecurityManager::check_ip_address(const IpAddress& ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& rule : ip_filter_rules_) {
        if (!rule.enabled) {
            continue;
        }
        
        // Check if rule has expired
        if (rule.expires < std::chrono::system_clock::now()) {
            continue;
        }
        
        if (ip_matches_rule(ip_address, rule)) {
            update_security_stats(rule.allow ? "ip_allowed" : "ip_blocked");
            return rule.allow;
        }
    }
    
    // Default allow if no rules match
    update_security_stats("ip_allowed");
    return true;
}

std::vector<IpFilterRule> DhcpSecurityManager::get_ip_filter_rules() {
    std::lock_guard<std::mutex> lock(mutex_);
    return ip_filter_rules_;
}

void DhcpSecurityManager::add_rate_limit_rule(const RateLimitRule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    rate_limit_rules_.push_back(rule);
    Logger::info("Added rate limit rule: " + rule.identifier + " (" + 
                 std::to_string(rule.max_requests) + " requests per " + 
                 std::to_string(rule.time_window.count()) + " seconds)");
}

void DhcpSecurityManager::remove_rate_limit_rule(const std::string& identifier, const std::string& identifier_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    rate_limit_rules_.erase(
        std::remove_if(rate_limit_rules_.begin(), rate_limit_rules_.end(),
            [&](const RateLimitRule& rule) {
                return rule.identifier == identifier && rule.identifier_type == identifier_type;
            }),
        rate_limit_rules_.end());
    
    Logger::info("Removed rate limit rule: " + identifier + " (" + identifier_type + ")");
}

bool DhcpSecurityManager::check_rate_limit(const std::string& identifier, const std::string& identifier_type) {
    return update_rate_limit_tracker(identifier, identifier_type);
}

std::vector<RateLimitRule> DhcpSecurityManager::get_rate_limit_rules() {
    std::lock_guard<std::mutex> lock(mutex_);
    return rate_limit_rules_;
}

void DhcpSecurityManager::set_option_82_validation_enabled(bool enabled) {
    option_82_validation_enabled_ = enabled;
    Logger::info("Option 82 validation " + std::string(enabled ? "enabled" : "disabled"));
}

bool DhcpSecurityManager::is_option_82_validation_enabled() const {
    return option_82_validation_enabled_;
}

bool DhcpSecurityManager::validate_option_82(const std::vector<uint8_t>& option_82_data, 
                                            const std::string& source_interface) {
    if (!option_82_validation_enabled_) {
        return true;
    }
    
    // Parse Option 82 sub-options
    std::map<uint8_t, std::vector<uint8_t>> sub_options;
    size_t pos = 0;
    
    while (pos < option_82_data.size()) {
        if (pos + 2 > option_82_data.size()) {
            break;
        }
        
        uint8_t sub_option_type = option_82_data[pos++];
        uint8_t sub_option_length = option_82_data[pos++];
        
        if (pos + sub_option_length > option_82_data.size()) {
            break;
        }
        
        std::vector<uint8_t> sub_option_value(option_82_data.begin() + pos, 
                                            option_82_data.begin() + pos + sub_option_length);
        sub_options[sub_option_type] = sub_option_value;
        
        pos += sub_option_length;
    }
    
    // Check for trusted relay agents
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : trusted_relay_agents_) {
        auto circuit_id_it = sub_options.find(1); // Circuit ID
        auto remote_id_it = sub_options.find(2);  // Remote ID
        
        if (circuit_id_it != sub_options.end() && remote_id_it != sub_options.end()) {
            std::string circuit_id(circuit_id_it->second.begin(), circuit_id_it->second.end());
            std::string remote_id(remote_id_it->second.begin(), remote_id_it->second.end());
            
            if (circuit_id == pair.first && remote_id == pair.second) {
                return true;
            }
        }
    }
    
    // Report invalid Option 82
    SecurityEvent event(SecurityEventType::INVALID_OPTION_82, ThreatLevel::MEDIUM,
                       "Invalid Option 82 from " + source_interface, "", "", source_interface);
    report_security_event(event);
    
    return false;
}

void DhcpSecurityManager::add_trusted_relay_agent(const std::string& circuit_id, const std::string& remote_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_relay_agents_[circuit_id] = remote_id;
    Logger::info("Added trusted relay agent: " + circuit_id + " -> " + remote_id);
}

void DhcpSecurityManager::remove_trusted_relay_agent(const std::string& circuit_id, const std::string& remote_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_relay_agents_.erase(circuit_id);
    Logger::info("Removed trusted relay agent: " + circuit_id + " -> " + remote_id);
}

void DhcpSecurityManager::set_authentication_enabled(bool enabled) {
    authentication_enabled_ = enabled;
    Logger::info("Authentication " + std::string(enabled ? "enabled" : "disabled"));
}

bool DhcpSecurityManager::is_authentication_enabled() const {
    return authentication_enabled_;
}

void DhcpSecurityManager::set_authentication_key(const std::string& key) {
    authentication_key_ = key;
    Logger::info("Authentication key updated");
}

bool DhcpSecurityManager::validate_client_authentication(const std::string& client_mac, 
                                                        const std::vector<uint8_t>& auth_data) {
    if (!authentication_enabled_ || authentication_key_.empty()) {
        return true;
    }
    
    // Generate expected authentication hash
    auto now = std::chrono::system_clock::now();
    std::string expected_hash = generate_auth_hash(client_mac, now);
    
    // Compare with provided authentication data
    std::string provided_hash(auth_data.begin(), auth_data.end());
    
    bool valid = (expected_hash == provided_hash);
    
    if (!valid) {
        SecurityEvent event(SecurityEventType::UNAUTHORIZED_CLIENT, ThreatLevel::HIGH,
                           "Authentication failed for client " + client_mac, client_mac);
        report_security_event(event);
    }
    
    return valid;
}

void DhcpSecurityManager::report_security_event(const SecurityEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    security_events_.push_back(event);
    
    // Update statistics
    security_stats_.total_events++;
    security_stats_.events_by_level[static_cast<int>(event.level)]++;
    security_stats_.events_by_type[static_cast<int>(event.type)]++;
    
    // Call callback if set
    if (security_event_callback_) {
        security_event_callback_(event);
    }
    
    // Log the event
    std::string level_str;
    switch (event.level) {
        case ThreatLevel::LOW: level_str = "LOW"; break;
        case ThreatLevel::MEDIUM: level_str = "MEDIUM"; break;
        case ThreatLevel::HIGH: level_str = "HIGH"; break;
        case ThreatLevel::CRITICAL: level_str = "CRITICAL"; break;
    }
    
    Logger::warning("Security Event [" + level_str + "]: " + event.description);
}

void DhcpSecurityManager::set_security_event_callback(std::function<void(const SecurityEvent&)> callback) {
    security_event_callback_ = callback;
}

std::vector<SecurityEvent> DhcpSecurityManager::get_security_events(
    std::chrono::system_clock::time_point start_time,
    std::chrono::system_clock::time_point end_time,
    SecurityEventType event_type) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<SecurityEvent> result;
    for (const auto& event : security_events_) {
        if (event.timestamp >= start_time && event.timestamp <= end_time) {
            if (event_type == SecurityEventType::UNAUTHORIZED_DHCP_SERVER || event.type == event_type) {
                result.push_back(event);
            }
        }
    }
    
    return result;
}

SecurityStats DhcpSecurityManager::get_security_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return security_stats_;
}

void DhcpSecurityManager::clear_security_statistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    security_stats_ = SecurityStats{};
    security_stats_.last_reset = std::chrono::system_clock::now();
    
    Logger::info("Security statistics cleared");
}

bool DhcpSecurityManager::load_security_configuration(const std::string& config_file) {
    // TODO: Implement configuration loading
    Logger::info("Loading security configuration from: " + config_file);
    return true;
}

bool DhcpSecurityManager::save_security_configuration(const std::string& config_file) {
    // TODO: Implement configuration saving
    Logger::info("Saving security configuration to: " + config_file);
    return true;
}

void DhcpSecurityManager::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    cleanup_thread_ = std::thread(&DhcpSecurityManager::cleanup_worker, this);
    
    Logger::info("Security manager started");
}

void DhcpSecurityManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    
    Logger::info("Security manager stopped");
}

void DhcpSecurityManager::cleanup_expired_items() {
    auto now = std::chrono::system_clock::now();
    
    // Clean up expired MAC filter rules
    mac_filter_rules_.erase(
        std::remove_if(mac_filter_rules_.begin(), mac_filter_rules_.end(),
            [&](const MacFilterRule& rule) {
                return rule.expires < now;
            }),
        mac_filter_rules_.end());
    
    // Clean up expired IP filter rules
    ip_filter_rules_.erase(
        std::remove_if(ip_filter_rules_.begin(), ip_filter_rules_.end(),
            [&](const IpFilterRule& rule) {
                return rule.expires < now;
            }),
        ip_filter_rules_.end());
    
    // Clean up old security events (keep last 1000)
    if (security_events_.size() > 1000) {
        security_events_.erase(security_events_.begin(), 
                              security_events_.end() - 1000);
    }
}

void DhcpSecurityManager::cleanup_worker() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        
        if (running_) {
            cleanup_expired_items();
        }
    }
}

bool DhcpSecurityManager::mac_matches_rule(const std::string& mac_address, const MacFilterRule& rule) {
    if (rule.mac_mask.empty()) {
        return mac_address == rule.mac_address;
    }
    
    // Simple pattern matching - can be enhanced
    std::regex pattern(rule.mac_address);
    return std::regex_match(mac_address, pattern);
}

bool DhcpSecurityManager::ip_matches_rule(const IpAddress& ip_address, const IpFilterRule& rule) {
    uint32_t ip = ip_address.to_uint32();
    uint32_t rule_ip = rule.ip_address.to_uint32();
    uint32_t mask = rule.ip_mask.to_uint32();
    
    return (ip & mask) == (rule_ip & mask);
}

bool DhcpSecurityManager::update_rate_limit_tracker(const std::string& identifier, const std::string& identifier_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find matching rate limit rule
    RateLimitRule* matching_rule = nullptr;
    for (auto& rule : rate_limit_rules_) {
        if (rule.identifier == identifier && rule.identifier_type == identifier_type && rule.enabled) {
            matching_rule = &rule;
            break;
        }
    }
    
    if (!matching_rule) {
        return true; // No rate limit rule, allow
    }
    
    auto now = std::chrono::system_clock::now();
    auto& tracker = rate_limit_trackers_[identifier];
    
    // Check if currently blocked
    if (now < tracker.blocked_until) {
        update_security_stats("rate_limit_blocked");
        return false;
    }
    
    // Reset window if needed
    if (now - tracker.window_start >= matching_rule->time_window) {
        tracker.request_count = 0;
        tracker.window_start = now;
    }
    
    // Check rate limit
    if (tracker.request_count >= matching_rule->max_requests) {
        tracker.blocked_until = now + matching_rule->block_duration;
        update_security_stats("rate_limit_exceeded");
        return false;
    }
    
    // Increment counter
    tracker.request_count++;
    update_security_stats("rate_limit_allowed");
    return true;
}

std::string DhcpSecurityManager::generate_auth_hash(const std::string& client_mac, 
                                                   std::chrono::system_clock::time_point timestamp) {
    // Simple HMAC-based authentication
    std::string data = client_mac + std::to_string(timestamp.time_since_epoch().count());
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len;
    
    HMAC(EVP_sha256(), authentication_key_.c_str(), authentication_key_.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         hash, &hash_len);
    
    std::string result;
    for (unsigned int i = 0; i < hash_len; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", hash[i]);
        result += buf;
    }
    
    return result;
}

bool DhcpSecurityManager::validate_auth_hash(const std::string& client_mac, 
                                            const std::vector<uint8_t>& auth_data,
                                            std::chrono::system_clock::time_point timestamp) {
    std::string expected_hash = generate_auth_hash(client_mac, timestamp);
    std::string provided_hash(auth_data.begin(), auth_data.end());
    
    return expected_hash == provided_hash;
}

void DhcpSecurityManager::update_security_stats(const std::string& stat_name) {
    if (stat_name == "blocked_requests") {
        security_stats_.blocked_requests++;
    } else if (stat_name == "allowed_requests") {
        security_stats_.allowed_requests++;
    } else if (stat_name == "rate_limit_hits") {
        security_stats_.rate_limit_hits++;
    } else if (stat_name == "mac_filter_hits") {
        security_stats_.mac_filter_hits++;
    } else if (stat_name == "ip_filter_hits") {
        security_stats_.ip_filter_hits++;
    }
}

} // namespace simple_dhcpd
