/**
 * @file options_manager.cpp
 * @brief Advanced DHCP options management implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-dhcpd/options/manager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace simple_dhcpd {

DhcpOptionsManager::DhcpOptionsManager() {
    initialize_standard_options();
}

DhcpOptionsManager::~DhcpOptionsManager() = default;

void DhcpOptionsManager::register_standard_option(DhcpOptionCode option_code, 
                                                 const std::string& name,
                                                 const std::string& description, 
                                                 const OptionTemplate& template_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto template_ptr = std::make_shared<OptionTemplate>(template_data);
    standard_options_[option_code] = template_ptr;
    
    std::cout << "INFO: Registered standard option: " + name + " (code " + std::to_string(static_cast<int>(option_code)) + ")" << std::endl;
}

void DhcpOptionsManager::register_vendor_option(DhcpOptionCode option_code, 
                                               const std::string& vendor_class,
                                               const std::string& name,
                                               const std::string& description,
                                               const OptionTemplate& template_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto template_ptr = std::make_shared<OptionTemplate>(template_data);
    vendor_options_[vendor_class][option_code] = template_ptr;
    
    std::cout << "INFO: Registered vendor option: " + name + " for vendor " + vendor_class + 
                 " (code " + std::to_string(static_cast<int>(option_code)) + ")" << std::endl;
}

void DhcpOptionsManager::register_custom_option(DhcpOptionCode option_code, 
                                               const std::string& name,
                                               const std::string& description,
                                               const OptionTemplate& template_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto template_ptr = std::make_shared<OptionTemplate>(template_data);
    custom_options_[option_code] = template_ptr;
    
    std::cout << "INFO: Registered custom option: " + name + " (code " + std::to_string(static_cast<int>(option_code)) + ")" << std::endl;
}

std::shared_ptr<OptionTemplate> DhcpOptionsManager::get_option_template(DhcpOptionCode option_code,
                                                                       const std::string& vendor_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check standard options first
    auto it = standard_options_.find(option_code);
    if (it != standard_options_.end()) {
        return it->second;
    }
    
    // Check vendor-specific options
    if (!vendor_class.empty()) {
        auto vendor_it = vendor_options_.find(vendor_class);
        if (vendor_it != vendor_options_.end()) {
            auto option_it = vendor_it->second.find(option_code);
            if (option_it != vendor_it->second.end()) {
                return option_it->second;
            }
        }
    }
    
    // Check custom options
    auto custom_it = custom_options_.find(option_code);
    if (custom_it != custom_options_.end()) {
        return custom_it->second;
    }
    
    return nullptr;
}

std::map<DhcpOptionCode, std::shared_ptr<OptionTemplate>> DhcpOptionsManager::get_all_options() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<DhcpOptionCode, std::shared_ptr<OptionTemplate>> all_options = standard_options_;
    
    // Add vendor options
    for (const auto& vendor_pair : vendor_options_) {
        for (const auto& option_pair : vendor_pair.second) {
            all_options[option_pair.first] = option_pair.second;
        }
    }
    
    // Add custom options
    for (const auto& custom_pair : custom_options_) {
        all_options[custom_pair.first] = custom_pair.second;
    }
    
    return all_options;
}

OptionValidationResult DhcpOptionsManager::validate_option(DhcpOptionCode option_code, 
                                                         const std::vector<uint8_t>& value,
                                                         const OptionsContext& context) {
    auto template_ptr = get_option_template(option_code, context.vendor_class);
    if (!template_ptr) {
        return OptionValidationResult(false, "Unknown option code: " + std::to_string(static_cast<int>(option_code)));
    }
    
    // Check custom validator first
    auto validator_it = custom_validators_.find(option_code);
    if (validator_it != custom_validators_.end()) {
        auto result = validator_it->second(value, context);
        update_validation_stats(result);
        return result;
    }
    
    // Perform standard validations
    auto length_result = validate_length(option_code, value);
    if (!length_result.valid) {
        update_validation_stats(length_result);
        return length_result;
    }
    
    auto format_result = validate_format(option_code, value);
    if (!format_result.valid) {
        update_validation_stats(format_result);
        return format_result;
    }
    
    auto range_result = validate_range(option_code, value);
    if (!range_result.valid) {
        update_validation_stats(range_result);
        return range_result;
    }
    
    update_validation_stats(OptionValidationResult(true));
    return OptionValidationResult(true);
}

std::map<DhcpOptionCode, OptionValidationResult> DhcpOptionsManager::validate_options(
    const std::map<DhcpOptionCode, std::vector<uint8_t>>& options,
    const OptionsContext& context) {
    
    std::map<DhcpOptionCode, OptionValidationResult> results;
    
    for (const auto& option_pair : options) {
        results[option_pair.first] = validate_option(option_pair.first, option_pair.second, context);
    }
    
    return results;
}

void DhcpOptionsManager::set_custom_validator(DhcpOptionCode option_code,
                                             std::function<OptionValidationResult(const std::vector<uint8_t>&, 
                                                                                 const OptionsContext&)> validator) {
    std::lock_guard<std::mutex> lock(mutex_);
    custom_validators_[option_code] = validator;
}

void DhcpOptionsManager::add_inheritance_rule(const OptionInheritanceRule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    inheritance_rules_.push_back(rule);
}

void DhcpOptionsManager::remove_inheritance_rule(const std::string& source_scope,
                                                const std::string& target_scope,
                                                DhcpOptionCode option_code) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    inheritance_rules_.erase(
        std::remove_if(inheritance_rules_.begin(), inheritance_rules_.end(),
            [&](const OptionInheritanceRule& rule) {
                return rule.source_scope == source_scope && 
                       rule.target_scope == target_scope && 
                       rule.option_code == option_code;
            }),
        inheritance_rules_.end());
}

std::vector<OptionInheritanceRule> DhcpOptionsManager::get_inheritance_rules() {
    std::lock_guard<std::mutex> lock(mutex_);
    return inheritance_rules_;
}

std::map<DhcpOptionCode, std::vector<uint8_t>> DhcpOptionsManager::apply_inheritance(
    const std::map<DhcpOptionCode, std::vector<uint8_t>>& global_options,
    const std::map<DhcpOptionCode, std::vector<uint8_t>>& subnet_options,
    const std::map<DhcpOptionCode, std::vector<uint8_t>>& pool_options,
    const std::map<DhcpOptionCode, std::vector<uint8_t>>& host_options,
    const OptionsContext& context) {
    
    std::map<DhcpOptionCode, std::vector<uint8_t>> final_options;
    
    // Start with global options
    final_options = global_options;
    
    // Apply inheritance rules
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& rule : inheritance_rules_) {
        if (!check_inheritance_condition(rule, context)) {
            continue;
        }
        
        std::map<DhcpOptionCode, std::vector<uint8_t>>* source_options = nullptr;
        std::map<DhcpOptionCode, std::vector<uint8_t>>* target_options = nullptr;
        
        // Determine source options
        if (rule.source_scope == "global") {
            source_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&global_options);
        } else if (rule.source_scope == "subnet") {
            source_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&subnet_options);
        } else if (rule.source_scope == "pool") {
            source_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&pool_options);
        } else if (rule.source_scope == "host") {
            source_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&host_options);
        }
        
        // Determine target options
        if (rule.target_scope == "subnet") {
            target_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&subnet_options);
        } else if (rule.target_scope == "pool") {
            target_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&pool_options);
        } else if (rule.target_scope == "host") {
            target_options = const_cast<std::map<DhcpOptionCode, std::vector<uint8_t>>*>(&host_options);
        }
        
        if (source_options && target_options) {
            auto source_it = source_options->find(rule.option_code);
            if (source_it != source_options->end()) {
                if (rule.inherit) {
                    final_options[rule.option_code] = source_it->second;
                } else if (!rule.override_value.empty()) {
                    final_options[rule.option_code] = rule.override_value;
                }
            }
        }
    }
    
    // Apply subnet options
    for (const auto& option_pair : subnet_options) {
        final_options[option_pair.first] = option_pair.second;
    }
    
    // Apply pool options
    for (const auto& option_pair : pool_options) {
        final_options[option_pair.first] = option_pair.second;
    }
    
    // Apply host options (highest priority)
    for (const auto& option_pair : host_options) {
        final_options[option_pair.first] = option_pair.second;
    }
    
    return final_options;
}

std::string DhcpOptionsManager::create_template(const std::string& name, 
                                               const std::string& description,
                                               const std::map<DhcpOptionCode, std::vector<uint8_t>>& options) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string template_id = "template_" + std::to_string(std::time(nullptr));
    option_templates_[template_id] = options;
    
    std::cout << "INFO: Created option template: " + name + " (ID: " + template_id + ")" << std::endl;
    
    return template_id;
}

std::map<DhcpOptionCode, std::vector<uint8_t>> DhcpOptionsManager::apply_template(
    const std::string& template_id, const OptionsContext& context) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = option_templates_.find(template_id);
    if (it != option_templates_.end()) {
        return it->second;
    }
    
    return {};
}

std::shared_ptr<std::map<DhcpOptionCode, std::vector<uint8_t>>> DhcpOptionsManager::get_template(
    const std::string& template_id) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = option_templates_.find(template_id);
    if (it != option_templates_.end()) {
        return std::make_shared<std::map<DhcpOptionCode, std::vector<uint8_t>>>(it->second);
    }
    
    return nullptr;
}

std::map<std::string, std::string> DhcpOptionsManager::list_templates() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<std::string, std::string> result;
    for (const auto& pair : option_templates_) {
        result[pair.first] = "Template " + pair.first;
    }
    
    return result;
}

std::map<DhcpOptionCode, std::vector<uint8_t>> DhcpOptionsManager::process_client_request(
    const std::vector<DhcpOptionCode>& requested_options,
    const OptionsContext& context) {
    
    std::map<DhcpOptionCode, std::vector<uint8_t>> response_options;
    
    for (const auto& option_code : requested_options) {
        auto template_ptr = get_option_template(option_code, context.vendor_class);
        if (template_ptr && !template_ptr->default_value.empty()) {
            response_options[option_code] = template_ptr->default_value;
        }
    }
    
    return response_options;
}

std::vector<uint8_t> DhcpOptionsManager::generate_option_82(const std::string& circuit_id,
                                                           const std::string& remote_id,
                                                           const std::string& subscriber_id) {
    std::vector<uint8_t> option_82;
    
    // Circuit ID sub-option (1)
    if (!circuit_id.empty()) {
        option_82.push_back(1); // Sub-option type
        option_82.push_back(static_cast<uint8_t>(circuit_id.length()));
        option_82.insert(option_82.end(), circuit_id.begin(), circuit_id.end());
    }
    
    // Remote ID sub-option (2)
    if (!remote_id.empty()) {
        option_82.push_back(2); // Sub-option type
        option_82.push_back(static_cast<uint8_t>(remote_id.length()));
        option_82.insert(option_82.end(), remote_id.begin(), remote_id.end());
    }
    
    // Subscriber ID sub-option (6)
    if (!subscriber_id.empty()) {
        option_82.push_back(6); // Sub-option type
        option_82.push_back(static_cast<uint8_t>(subscriber_id.length()));
        option_82.insert(option_82.end(), subscriber_id.begin(), subscriber_id.end());
    }
    
    return option_82;
}

std::map<uint8_t, std::vector<uint8_t>> DhcpOptionsManager::parse_option_82(
    const std::vector<uint8_t>& option_data) {
    
    std::map<uint8_t, std::vector<uint8_t>> sub_options;
    
    size_t pos = 0;
    while (pos < option_data.size()) {
        if (pos + 2 > option_data.size()) {
            break; // Not enough data for sub-option header
        }
        
        uint8_t sub_option_type = option_data[pos++];
        uint8_t sub_option_length = option_data[pos++];
        
        if (pos + sub_option_length > option_data.size()) {
            break; // Not enough data for sub-option value
        }
        
        std::vector<uint8_t> sub_option_value(option_data.begin() + pos, 
                                            option_data.begin() + pos + sub_option_length);
        sub_options[sub_option_type] = sub_option_value;
        
        pos += sub_option_length;
    }
    
    return sub_options;
}

bool DhcpOptionsManager::load_configuration(const std::string& config_file) {
    // TODO: Implement configuration loading
    std::cout << "INFO: Loading options configuration from: " + config_file << std::endl;
    return true;
}

bool DhcpOptionsManager::save_configuration(const std::string& config_file) {
    // TODO: Implement configuration saving
    std::cout << "INFO: Saving options configuration to: " + config_file << std::endl;
    return true;
}

void DhcpOptionsManager::reset_to_defaults() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    standard_options_.clear();
    vendor_options_.clear();
    custom_options_.clear();
    inheritance_rules_.clear();
    option_templates_.clear();
    custom_validators_.clear();
    
    initialize_standard_options();
    
    std::cout << "INFO: Reset options manager to defaults" << std::endl;
}

std::map<DhcpOptionCode, size_t> DhcpOptionsManager::get_option_usage_stats() {
    std::lock_guard<std::mutex> lock(mutex_);
    return option_usage_stats_;
}

std::map<std::string, size_t> DhcpOptionsManager::get_validation_stats() {
    std::lock_guard<std::mutex> lock(mutex_);
    return validation_stats_;
}

void DhcpOptionsManager::clear_statistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    option_usage_stats_.clear();
    validation_stats_.clear();
}

void DhcpOptionsManager::initialize_standard_options() {
    // Register common DHCP options
    register_standard_option(DhcpOptionCode::SUBNET_MASK, "Subnet Mask", 
                           "Subnet mask for the client's network", 
                           OptionTemplate("Subnet Mask", "Subnet mask for the client's network", 
                                        DhcpOptionCode::SUBNET_MASK, {}, true));
    
    register_standard_option(DhcpOptionCode::ROUTER, "Router", 
                           "List of routers in order of preference", 
                           OptionTemplate("Router", "List of routers in order of preference", 
                                        DhcpOptionCode::ROUTER));
    
    register_standard_option(DhcpOptionCode::DOMAIN_SERVER, "Domain Name Server", 
                           "List of DNS servers in order of preference", 
                           OptionTemplate("Domain Name Server", "List of DNS servers in order of preference", 
                                        DhcpOptionCode::DOMAIN_SERVER));
    
    register_standard_option(DhcpOptionCode::DOMAIN_NAME, "Domain Name", 
                           "Domain name for the client", 
                           OptionTemplate("Domain Name", "Domain name for the client", 
                                        DhcpOptionCode::DOMAIN_NAME));
    
    register_standard_option(DhcpOptionCode::IP_ADDRESS_LEASE_TIME, "IP Address Lease Time", 
                           "Lease time for the IP address", 
                           OptionTemplate("IP Address Lease Time", "Lease time for the IP address", 
                                        DhcpOptionCode::IP_ADDRESS_LEASE_TIME, {}, true));
    
    register_standard_option(DhcpOptionCode::RENEWAL_TIME, "Renewal Time", 
                           "Time at which client should renew its lease", 
                           OptionTemplate("Renewal Time", "Time at which client should renew its lease", 
                                        DhcpOptionCode::RENEWAL_TIME));
    
    register_standard_option(DhcpOptionCode::REBINDING_TIME, "Rebinding Time", 
                           "Time at which client should rebind its lease", 
                           OptionTemplate("Rebinding Time", "Time at which client should rebind its lease", 
                                        DhcpOptionCode::REBINDING_TIME));
    
    register_standard_option(DhcpOptionCode::HOST_NAME, "Host Name", 
                           "Host name for the client", 
                           OptionTemplate("Host Name", "Host name for the client", 
                                        DhcpOptionCode::HOST_NAME));
    
    register_standard_option(DhcpOptionCode::CLIENT_IDENTIFIER, "Client Identifier", 
                           "Client identifier", 
                           OptionTemplate("Client Identifier", "Client identifier", 
                                        DhcpOptionCode::CLIENT_IDENTIFIER));
    
    register_standard_option(DhcpOptionCode::REQUESTED_IP_ADDRESS, "Requested IP Address", 
                           "Requested IP address", 
                           OptionTemplate("Requested IP Address", "Requested IP address", 
                                        DhcpOptionCode::REQUESTED_IP_ADDRESS));
    
    register_standard_option(DhcpOptionCode::SERVER_IDENTIFIER, "Server Identifier", 
                           "Server identifier", 
                           OptionTemplate("Server Identifier", "Server identifier", 
                                        DhcpOptionCode::SERVER_IDENTIFIER));
    
    register_standard_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, "DHCP Message Type", 
                           "DHCP message type", 
                           OptionTemplate("DHCP Message Type", "DHCP message type", 
                                        DhcpOptionCode::DHCP_MESSAGE_TYPE, {}, true));
    
    register_standard_option(DhcpOptionCode::PARAMETER_REQUEST_LIST, "Parameter Request List", 
                           "List of requested parameters", 
                           OptionTemplate("Parameter Request List", "List of requested parameters", 
                                        DhcpOptionCode::PARAMETER_REQUEST_LIST));
    
    register_standard_option(DhcpOptionCode::MESSAGE, "Message", 
                           "Error message", 
                           OptionTemplate("Message", "Error message", 
                                        DhcpOptionCode::MESSAGE));
    
    register_standard_option(DhcpOptionCode::MAX_DHCP_MESSAGE_SIZE, "Maximum DHCP Message Size", 
                           "Maximum DHCP message size", 
                           OptionTemplate("Maximum DHCP Message Size", "Maximum DHCP message size", 
                                        DhcpOptionCode::MAX_DHCP_MESSAGE_SIZE));
    
    register_standard_option(DhcpOptionCode::VENDOR_CLASS_IDENTIFIER, "Vendor Class Identifier", 
                           "Vendor class identifier", 
                           OptionTemplate("Vendor Class Identifier", "Vendor class identifier", 
                                        DhcpOptionCode::VENDOR_CLASS_IDENTIFIER));
    
    register_standard_option(DhcpOptionCode::CLIENT_FQDN, "Client FQDN", 
                           "Client fully qualified domain name", 
                           OptionTemplate("Client FQDN", "Client fully qualified domain name", 
                                        DhcpOptionCode::CLIENT_FQDN));
    
    register_standard_option(DhcpOptionCode::RELAY_AGENT_INFORMATION, "Relay Agent Information", 
                           "Relay agent information", 
                           OptionTemplate("Relay Agent Information", "Relay agent information", 
                                        DhcpOptionCode::RELAY_AGENT_INFORMATION));
    
    register_standard_option(DhcpOptionCode::END, "End", 
                           "End of options marker", 
                           OptionTemplate("End", "End of options marker", 
                                        DhcpOptionCode::END, {}, true));
}

OptionValidationResult DhcpOptionsManager::validate_length(DhcpOptionCode option_code, 
                                                         const std::vector<uint8_t>& value) {
    auto template_ptr = get_option_template(option_code);
    if (!template_ptr) {
        return OptionValidationResult(false, "Unknown option code");
    }
    
    if (value.size() < template_ptr->min_length) {
        return OptionValidationResult(false, "Option value too short");
    }
    
    if (value.size() > template_ptr->max_length) {
        return OptionValidationResult(false, "Option value too long");
    }
    
    return OptionValidationResult(true);
}

OptionValidationResult DhcpOptionsManager::validate_format(DhcpOptionCode option_code, 
                                                         const std::vector<uint8_t>& value) {
    // Basic format validation - can be extended for specific options
    return OptionValidationResult(true);
}

OptionValidationResult DhcpOptionsManager::validate_range(DhcpOptionCode option_code, 
                                                        const std::vector<uint8_t>& value) {
    // Basic range validation - can be extended for specific options
    return OptionValidationResult(true);
}

bool DhcpOptionsManager::check_inheritance_condition(const OptionInheritanceRule& rule,
                                                    const OptionsContext& context) {
    if (rule.condition.empty()) {
        return true;
    }
    
    // Simple condition checking - can be extended
    if (rule.condition == "vendor_class" && !context.vendor_class.empty()) {
        return true;
    }
    
    if (rule.condition == "user_class" && !context.user_class.empty()) {
        return true;
    }
    
    return false;
}

void DhcpOptionsManager::update_usage_stats(DhcpOptionCode option_code) {
    option_usage_stats_[option_code]++;
}

void DhcpOptionsManager::update_validation_stats(const OptionValidationResult& result) {
    if (result.valid) {
        validation_stats_["valid"]++;
    } else {
        validation_stats_["invalid"]++;
    }
    
    if (!result.error_message.empty()) {
        validation_stats_["errors"]++;
    }
    
    if (!result.warning_message.empty()) {
        validation_stats_["warnings"]++;
    }
}

} // namespace simple_dhcpd
