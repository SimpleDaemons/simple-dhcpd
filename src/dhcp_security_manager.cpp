/**
 * @file dhcp_security_manager.cpp
 * @brief DHCP security features implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "dhcp_security_manager.hpp"
#include "dhcp_utils.hpp"
#include <iostream>
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
    std::cout << "INFO: DHCP snooping " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool DhcpSecurityManager::is_dhcp_snooping_enabled() const {
    return dhcp_snooping_enabled_;
}

void DhcpSecurityManager::add_trusted_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_interfaces_.insert(interface_name);
    std::cout << "INFO: Added trusted interface: " << interface_name << std::endl;
}

void DhcpSecurityManager::remove_trusted_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    trusted_interfaces_.erase(interface_name);
    std::cout << "INFO: Removed trusted interface: " << interface_name << std::endl;
}

bool DhcpSecurityManager::is_interface_trusted(const std::string& interface_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return trusted_interfaces_.find(interface_name) != trusted_interfaces_.end();
}

void DhcpSecurityManager::add_snooping_binding(const DhcpSnoopingBinding& binding) {
    std::lock_guard<std::mutex> lock(mutex_);
    snooping_bindings_.push_back(binding);
    std::cout << "INFO: Added snooping binding: " << binding.mac_address << " -> " << 
                 ip_to_string(binding.ip_address) << " on " << binding.interface << std::endl;
}

void DhcpSecurityManager::remove_snooping_binding(const std::string& mac_address, const IpAddress& ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    snooping_bindings_.erase(
        std::remove_if(snooping_bindings_.begin(), snooping_bindings_.end(),
            [&](const DhcpSnoopingBinding& binding) {
                return binding.mac_address == mac_address && binding.ip_address == ip_address;
            }),
        snooping_bindings_.end());
    
    std::cout << "INFO: Removed snooping binding: " << mac_address << " -> " << ip_to_string(ip_address) << std::endl;
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
        // Monitoring hook: record allowed message on trusted interface
        report_security_event(SecurityEvent(SecurityEventType::SUSPICIOUS_ACTIVITY, ThreatLevel::LOW,
                                            "DHCP message allowed from trusted interface",
                                            mac_to_string(message.client_mac),
                                            ip_to_string(message.client_ip), source_interface));
        return true;
    }
    
    // Validate against snooping bindings
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& binding : snooping_bindings_) {
        if (binding.mac_address == mac_to_string(message.client_mac) && 
            binding.ip_address == message.client_ip) {
            // Ensure binding's learned interface matches the source interface
            if (binding.interface == source_interface) {
                return true;
            }
            report_security_event(SecurityEvent(SecurityEventType::SUSPICIOUS_ACTIVITY, ThreatLevel::MEDIUM,
                                                "Snooping binding interface mismatch",
                                                mac_to_string(message.client_mac),
                                                ip_to_string(message.client_ip), source_interface));
            return false;
        }
    }
    
    // Classify event based on message type
    if (message.message_type == DhcpMessageType::OFFER || message.message_type == DhcpMessageType::ACK) {
        report_security_event(SecurityEvent(SecurityEventType::UNAUTHORIZED_DHCP_SERVER, ThreatLevel::HIGH,
                                            "Unauthorized DHCP server activity detected",
                                            mac_to_string(message.client_mac),
                                            ip_to_string(message.client_ip), source_interface));
    } else {
        report_security_event(SecurityEvent(SecurityEventType::SUSPICIOUS_ACTIVITY, ThreatLevel::MEDIUM,
                                            "DHCP message failed snooping validation",
                                            mac_to_string(message.client_mac),
                                            ip_to_string(message.client_ip), source_interface));
    }
    
    return false;
}

// Placeholder implementations for remaining methods
void DhcpSecurityManager::add_mac_filter_rule(const MacFilterRule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    mac_filter_rules_.push_back(rule);
    std::cout << "INFO: Added MAC filter rule: " << rule.mac_address << " (" << 
                 (rule.allow ? "allow" : "deny") << ")" << std::endl;
}

void DhcpSecurityManager::remove_mac_filter_rule(const std::string& mac_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    mac_filter_rules_.erase(
        std::remove_if(mac_filter_rules_.begin(), mac_filter_rules_.end(),
            [&](const MacFilterRule& rule) {
                return rule.mac_address == mac_address;
            }),
        mac_filter_rules_.end());
    
    std::cout << "INFO: Removed MAC filter rule: " << mac_address << std::endl;
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
    std::cout << "INFO: Added IP filter rule: " << ip_to_string(rule.ip_address) << " (" << 
                 (rule.allow ? "allow" : "deny") << ")" << std::endl;
}

void DhcpSecurityManager::remove_ip_filter_rule(const IpAddress& ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ip_filter_rules_.erase(
        std::remove_if(ip_filter_rules_.begin(), ip_filter_rules_.end(),
            [&](const IpFilterRule& rule) {
                return rule.ip_address == ip_address;
            }),
        ip_filter_rules_.end());
    
    std::cout << "INFO: Removed IP filter rule: " << ip_to_string(ip_address) << std::endl;
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
    std::cout << "INFO: Added rate limit rule: " << rule.identifier << " (" << 
                 std::to_string(rule.max_requests) << " requests per " << 
                 std::to_string(rule.time_window.count()) << " seconds)" << std::endl;
}

void DhcpSecurityManager::remove_rate_limit_rule(const std::string& identifier, const std::string& identifier_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    rate_limit_rules_.erase(
        std::remove_if(rate_limit_rules_.begin(), rate_limit_rules_.end(),
            [&](const RateLimitRule& rule) {
                return rule.identifier == identifier && rule.identifier_type == identifier_type;
            }),
        rate_limit_rules_.end());
    
    std::cout << "INFO: Removed rate limit rule: " << identifier << " (" << identifier_type << ")" << std::endl;
}

bool DhcpSecurityManager::check_rate_limit(const std::string& identifier, const std::string& identifier_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find applicable rate limit rule
    RateLimitRule* applicable_rule = nullptr;
    for (auto& rule : rate_limit_rules_) {
        if (!rule.enabled) {
            continue;
        }
        
        if (rule.identifier_type == identifier_type && 
            (rule.identifier == identifier || rule.identifier == "*")) {
            applicable_rule = &rule;
            break;
        }
    }
    
    if (!applicable_rule) {
        // No rate limiting rule applies
        update_security_stats("rate_limit_allowed");
        return true;
    }
    
    // Check if rule has expired
    if (applicable_rule->expires < std::chrono::system_clock::now()) {
        update_security_stats("rate_limit_allowed");
        return true;
    }
    
    // Get or create rate limit tracker
    std::string tracker_key = identifier_type + ":" + identifier;
    auto& tracker = rate_limit_trackers_[tracker_key];
    
    auto now = std::chrono::system_clock::now();
    
    // Clean up old requests outside the time window
    auto cutoff_time = now - applicable_rule->time_window;
    tracker.requests.erase(
        std::remove_if(tracker.requests.begin(), tracker.requests.end(),
            [&](const std::chrono::system_clock::time_point& request_time) {
                return request_time < cutoff_time;
            }),
        tracker.requests.end());
    
    // Check if we're within the rate limit
    if (tracker.requests.size() >= applicable_rule->max_requests) {
        update_security_stats("rate_limit_exceeded");
        std::cout << "WARNING: Rate limit exceeded for " << identifier << " (" << identifier_type << ")" << std::endl;
        return false;
    }
    
    // Add current request
    tracker.requests.push_back(now);
    update_security_stats("rate_limit_allowed");
    return true;
}

std::vector<RateLimitRule> DhcpSecurityManager::get_rate_limit_rules() {
    std::lock_guard<std::mutex> lock(mutex_);
    return rate_limit_rules_;
}

void DhcpSecurityManager::set_option_82_validation_enabled(bool enabled) {
    option_82_validation_enabled_ = enabled;
    std::cout << "INFO: Option 82 validation " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool DhcpSecurityManager::is_option_82_validation_enabled() const {
    return option_82_validation_enabled_;
}

bool DhcpSecurityManager::validate_option_82(const std::vector<uint8_t>& option_82_data, 
                                            const std::string& source_interface) {
    if (!option_82_validation_enabled_) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if Option 82 is required for this interface
    bool required = false;
    for (const auto& rule : option_82_rules_) {
        if (!rule.enabled) {
            continue;
        }
        if (rule.interface == source_interface || rule.interface == "*") {
            required = rule.required;
            break;
        }
    }
    
    if (!required) {
        // Option 82 not required for this interface
        update_security_stats("option_82_allowed");
        return true;
    }
    
    // Validate Option 82 data
    if (option_82_data.empty()) {
        update_security_stats("option_82_missing");
        std::cout << "WARNING: Option 82 required but missing for interface " << source_interface << std::endl;
        return false;
    }
    
    // Basic Option 82 validation (circuit-id and remote-id)
    if (option_82_data.size() < 4) {
        update_security_stats("option_82_invalid");
        std::cout << "WARNING: Option 82 data too short for interface " << source_interface << std::endl;
        return false;
    }
    
    // Check for valid sub-options
    size_t pos = 0;
    bool has_circuit_id = false;
    bool has_remote_id = false;
    
    while (pos < option_82_data.size()) {
        if (pos + 2 > option_82_data.size()) {
            break; // Not enough data for sub-option header
        }
        
        uint8_t sub_option = option_82_data[pos];
        uint8_t length = option_82_data[pos + 1];
        
        if (pos + 2 + length > option_82_data.size()) {
            break; // Not enough data for sub-option content
        }
        
        if (sub_option == 1) { // Circuit ID
            has_circuit_id = true;
        } else if (sub_option == 2) { // Remote ID
            has_remote_id = true;
        }
        
        pos += 2 + length;
    }
    
    if (!has_circuit_id || !has_remote_id) {
        update_security_stats("option_82_incomplete");
        std::cout << "WARNING: Option 82 missing required sub-options for interface " << source_interface << std::endl;
        return false;
    }
    
    update_security_stats("option_82_valid");
    return true;
}

// Option 82 rule management
void DhcpSecurityManager::add_option_82_rule(const Option82Rule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    option_82_rules_.push_back(rule);
}

void DhcpSecurityManager::clear_option_82_rules() {
    std::lock_guard<std::mutex> lock(mutex_);
    option_82_rules_.clear();
}

std::vector<Option82Rule> DhcpSecurityManager::get_option_82_rules() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return option_82_rules_;
}

void DhcpSecurityManager::set_option_82_required_for_interface(const std::string& interface, bool required) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Try to update existing
    for (auto& rule : option_82_rules_) {
        if (rule.interface == interface) {
            rule.required = required;
            rule.enabled = true;
            return;
        }
    }
    // Otherwise add
    option_82_rules_.push_back(Option82Rule{interface, required, true});
}

void DhcpSecurityManager::add_trusted_relay_agent(const std::string& circuit_id, const std::string& remote_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TrustedRelayAgent agent;
    agent.circuit_id = circuit_id;
    agent.remote_id = remote_id;
    agent.enabled = true;
    agent.created_at = std::chrono::system_clock::now();
    
    trusted_relay_agents_.push_back(agent);
    std::cout << "INFO: Added trusted relay agent: circuit_id=" << circuit_id << ", remote_id=" << remote_id << std::endl;
}

void DhcpSecurityManager::remove_trusted_relay_agent(const std::string& circuit_id, const std::string& remote_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    trusted_relay_agents_.erase(
        std::remove_if(trusted_relay_agents_.begin(), trusted_relay_agents_.end(),
            [&](const TrustedRelayAgent& agent) {
                return agent.circuit_id == circuit_id && agent.remote_id == remote_id;
            }),
        trusted_relay_agents_.end());
    
    std::cout << "INFO: Removed trusted relay agent: circuit_id=" << circuit_id << ", remote_id=" << remote_id << std::endl;
}

void DhcpSecurityManager::set_authentication_enabled(bool enabled) {
    authentication_enabled_ = enabled;
    std::cout << "INFO: Authentication " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool DhcpSecurityManager::is_authentication_enabled() const {
    return authentication_enabled_;
}

void DhcpSecurityManager::set_authentication_key(const std::string& key) {
    authentication_key_ = key;
    std::cout << "INFO: Authentication key updated" << std::endl;
}

bool DhcpSecurityManager::validate_client_authentication(const std::string& client_mac, 
                                                        const std::vector<uint8_t>& auth_data) {
    if (!authentication_enabled_) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find client credentials
    auto it = client_credentials_.find(client_mac);
    if (it == client_credentials_.end()) {
        update_security_stats("auth_client_not_found");
        std::cout << "WARNING: Authentication failed - client not found: " << client_mac << std::endl;
        return false;
    }
    
    const auto& credentials = it->second;
    
    if (!credentials.enabled) {
        update_security_stats("auth_client_disabled");
        std::cout << "WARNING: Authentication failed - client disabled: " << client_mac << std::endl;
        return false;
    }
    
    if (credentials.expires < std::chrono::system_clock::now()) {
        update_security_stats("auth_client_expired");
        std::cout << "WARNING: Authentication failed - client expired: " << client_mac << std::endl;
        return false;
    }

    if (auth_data.empty()) {
        update_security_stats("auth_data_missing");
        std::cout << "WARNING: Authentication failed - no auth data for client: " << client_mac << std::endl;
        return false;
    }

    // Validate HMAC using current time window (allow small skew of +/-60s)
    const auto now = std::chrono::system_clock::now();
    const std::array<int,3> offsets = {0, -60, 60};
    for (int offset : offsets) {
        const auto ts = now + std::chrono::seconds(offset);
        if (validate_auth_hash(client_mac, auth_data, ts)) {
            update_security_stats("auth_success");
            std::cout << "INFO: Client authenticated successfully: " << client_mac << std::endl;
            return true;
        }
    }

    update_security_stats("auth_failed");
    std::cout << "WARNING: Authentication failed - invalid HMAC for client: " << client_mac << std::endl;
    return false;
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
    
    std::cout << "WARNING: Security Event [" << level_str << "]: " << event.description << std::endl;
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
    
    std::cout << "INFO: Security statistics cleared" << std::endl;
}

bool DhcpSecurityManager::load_security_configuration(const std::string& config_file) {
    std::cout << "INFO: Loading security configuration from: " << config_file << std::endl;
    return true;
}

bool DhcpSecurityManager::save_security_configuration(const std::string& config_file) {
    std::cout << "INFO: Saving security configuration to: " << config_file << std::endl;
    return true;
}

void DhcpSecurityManager::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    cleanup_thread_ = std::thread(&DhcpSecurityManager::cleanup_worker, this);
    
    std::cout << "INFO: Security manager started" << std::endl;
}

void DhcpSecurityManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    
    std::cout << "INFO: Security manager stopped" << std::endl;
}

void DhcpSecurityManager::cleanup_expired_items() {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto now = std::chrono::system_clock::now();

    // Expire MAC filter rules
    mac_filter_rules_.erase(
        std::remove_if(mac_filter_rules_.begin(), mac_filter_rules_.end(),
            [&](const MacFilterRule& rule) { return rule.expires < now; }),
        mac_filter_rules_.end());

    // Expire IP filter rules
    ip_filter_rules_.erase(
        std::remove_if(ip_filter_rules_.begin(), ip_filter_rules_.end(),
            [&](const IpFilterRule& rule) { return rule.expires < now; }),
        ip_filter_rules_.end());

    // Expire Option82 rules
    option_82_rules_.erase(
        std::remove_if(option_82_rules_.begin(), option_82_rules_.end(),
            [&](const Option82Rule& rule) { return rule.expires < now; }),
        option_82_rules_.end());

    // Expire trusted relay agents
    trusted_relay_agents_.erase(
        std::remove_if(trusted_relay_agents_.begin(), trusted_relay_agents_.end(),
            [&](const TrustedRelayAgent& /*agent*/) { return false; }),
        trusted_relay_agents_.end());

    // Expire rate limit rules
    rate_limit_rules_.erase(
        std::remove_if(rate_limit_rules_.begin(), rate_limit_rules_.end(),
            [&](const RateLimitRule& rule) { return rule.expires < now; }),
        rate_limit_rules_.end());

    // Clean old entries from trackers and drop blocked entries that have unblocked
    for (auto it = rate_limit_trackers_.begin(); it != rate_limit_trackers_.end(); ) {
        auto& tracker = it->second;
        // Drop request timestamps older than 1 hour
        const auto cutoff = now - std::chrono::hours(1);
        tracker.requests.erase(
            std::remove_if(tracker.requests.begin(), tracker.requests.end(),
                [&](const auto& t) { return t < cutoff; }),
            tracker.requests.end());

        // Remove empty trackers that are not blocked
        if (tracker.requests.empty() && tracker.blocked_until <= now) {
            it = rate_limit_trackers_.erase(it);
        } else {
            ++it;
        }
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

// Helper method implementations

bool DhcpSecurityManager::update_rate_limit_tracker(const std::string& identifier, const std::string& identifier_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    std::string tracker_key = identifier_type + ":" + identifier;
    auto& tracker = rate_limit_trackers_[tracker_key];
    
    // Clean up old entries (older than 1 hour)
    auto cutoff = now - std::chrono::hours(1);
    tracker.requests.erase(std::remove_if(tracker.requests.begin(), tracker.requests.end(),
        [cutoff](const auto& entry) { return entry < cutoff; }), tracker.requests.end());
    
    // Add current request
    tracker.requests.push_back(now);
    
    // Check rate limits
    auto minute_ago = now - std::chrono::minutes(1);
    auto hour_ago = now - std::chrono::hours(1);
    
    size_t requests_last_minute = std::count_if(tracker.requests.begin(), tracker.requests.end(),
        [minute_ago](const auto& entry) { return entry >= minute_ago; });
    
    size_t requests_last_hour = std::count_if(tracker.requests.begin(), tracker.requests.end(),
        [hour_ago](const auto& entry) { return entry >= hour_ago; });
    
    // Check if limits are exceeded (using default values for now)
    const size_t max_requests_per_minute = 100;
    const size_t max_requests_per_hour = 1000;
    
    if (requests_last_minute > max_requests_per_minute || 
        requests_last_hour > max_requests_per_hour) {
        std::cout << "WARNING: Rate limit exceeded for " << identifier_type 
                  << " " << identifier << " (minute: " << requests_last_minute 
                  << ", hour: " << requests_last_hour << ")" << std::endl;
        return false;
    }
    
    return true;
}

std::string DhcpSecurityManager::generate_auth_hash(const std::string& client_mac, 
                                                   std::chrono::system_clock::time_point timestamp) {
    // HMAC-SHA256 over (client_mac | timestamp_seconds) using authentication_key_
    // If no key is set, return empty string to indicate invalid/non-configured
    if (authentication_key_.empty()) {
        return std::string();
    }

    // Prepare message bytes
    const auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(
        timestamp.time_since_epoch()).count();

    std::string message = client_mac + ":" + std::to_string(seconds_since_epoch);

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    HMAC(EVP_sha256(), authentication_key_.data(), static_cast<int>(authentication_key_.size()),
         reinterpret_cast<const unsigned char*>(message.data()), message.size(), digest, &digest_len);

    // Return raw bytes as hex string
    static const char hex_chars[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(digest_len * 2);
    for (unsigned int i = 0; i < digest_len; ++i) {
        hex.push_back(hex_chars[(digest[i] >> 4) & 0x0F]);
        hex.push_back(hex_chars[digest[i] & 0x0F]);
    }
    return hex;
}

bool DhcpSecurityManager::validate_auth_hash(const std::string& client_mac, 
                                            const std::vector<uint8_t>& auth_data,
                                            std::chrono::system_clock::time_point timestamp) {
    if (authentication_key_.empty()) {
        return false;
    }

    // Compute expected hex digest
    const std::string expected_hex = generate_auth_hash(client_mac, timestamp);
    if (expected_hex.empty()) {
        return false;
    }

    // auth_data could be raw bytes of HMAC or hex-encoded; support both.
    // If raw bytes length matches SHA-256 (32), convert to hex for compare.
    std::string provided_hex;
    if (auth_data.size() == 32) {
        static const char hex_chars[] = "0123456789abcdef";
        provided_hex.reserve(64);
        for (uint8_t b : auth_data) {
            provided_hex.push_back(hex_chars[(b >> 4) & 0x0F]);
            provided_hex.push_back(hex_chars[b & 0x0F]);
        }
    } else {
        // Treat as hex bytes
        provided_hex.assign(auth_data.begin(), auth_data.end());
        // Normalize to lowercase
        std::transform(provided_hex.begin(), provided_hex.end(), provided_hex.begin(), ::tolower);
    }

    return provided_hex == expected_hex;
}

void DhcpSecurityManager::update_security_stats(const std::string& stat_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    security_stats_.stats[stat_name]++;
    
    // Update specific counters
    if (stat_name.find("blocked") != std::string::npos || 
        stat_name.find("exceeded") != std::string::npos ||
        stat_name.find("invalid") != std::string::npos) {
        security_stats_.blocked_requests++;
    } else if (stat_name.find("allowed") != std::string::npos) {
        security_stats_.allowed_requests++;
    }
    
    // Log significant security events
    if (stat_name.find("blocked") != std::string::npos || 
        stat_name.find("exceeded") != std::string::npos) {
        std::cout << "SECURITY: " << stat_name << " (total: " << security_stats_.stats[stat_name] << ")" << std::endl;
    }
}

bool DhcpSecurityManager::mac_matches_rule(const std::string& mac_address, const MacFilterRule& rule) {
    // Normalize: lowercase and remove separators
    auto normalize = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        s.erase(std::remove(s.begin(), s.end(), ':'), s.end());
        s.erase(std::remove(s.begin(), s.end(), '-'), s.end());
        return s;
    };
    const std::string target = normalize(mac_address);
    std::string pattern = normalize(rule.mac_address);

    if (pattern == "*" || pattern == target) {
        return true;
    }

    // Wildcard to regex: '*' -> ".*", '?' -> '.'
    std::string regex_str;
    regex_str.reserve(pattern.size() * 2);
    regex_str.push_back('^');
    for (char c : pattern) {
        if (c == '*') { regex_str += ".*"; }
        else if (c == '?') { regex_str += '.'; }
        else if (std::isalnum(static_cast<unsigned char>(c))) { regex_str.push_back(c); }
        else { regex_str.push_back('\\'); regex_str.push_back(c); }
    }
    regex_str.push_back('$');
    try {
        std::regex re(regex_str);
        return std::regex_match(target, re);
    } catch (...) {
        return false;
    }
}

bool DhcpSecurityManager::ip_matches_rule(const IpAddress& ip_address, const IpFilterRule& rule) {
    // Apply mask if provided, else exact match
    if (rule.ip_mask != 0) {
        return (ip_address & rule.ip_mask) == (rule.ip_address & rule.ip_mask);
    }
    return rule.ip_address == ip_address;
}

} // namespace simple_dhcpd