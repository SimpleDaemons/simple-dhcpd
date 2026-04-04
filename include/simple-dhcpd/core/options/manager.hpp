/**
 * @file options_manager.hpp
 * @brief Advanced DHCP options management with validation and templates
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_OPTIONS_MANAGER_HPP
#define SIMPLE_DHCPD_OPTIONS_MANAGER_HPP

#include "simple-dhcpd/core/types.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <set>
#include <regex>

namespace simple_dhcpd {

/**
 * @brief DHCP option validation result
 */
struct OptionValidationResult {
    bool valid;
    std::string error_message;
    std::string warning_message;
    
    OptionValidationResult() : valid(true) {}
    OptionValidationResult(bool v, const std::string& error = "", const std::string& warning = "")
        : valid(v), error_message(error), warning_message(warning) {}
};

/**
 * @brief DHCP option template
 */
struct OptionTemplate {
    std::string name;
    std::string description;
    DhcpOptionCode code;
    std::vector<uint8_t> default_value;
    std::vector<uint8_t> min_value;
    std::vector<uint8_t> max_value;
    size_t min_length;
    size_t max_length;
    std::string validation_pattern;
    bool required;
    bool vendor_specific;
    std::string vendor_class;
    
    OptionTemplate() = default;
    OptionTemplate(const std::string& n, const std::string& desc, DhcpOptionCode c,
                   const std::vector<uint8_t>& def_val = {}, bool req = false)
        : name(n), description(desc), code(c), default_value(def_val), 
          min_length(0), max_length(255), required(req), vendor_specific(false) {}
};

/**
 * @brief DHCP option inheritance rule
 */
struct OptionInheritanceRule {
    std::string source_scope;      // "global", "subnet", "pool", "host"
    std::string target_scope;      // "subnet", "pool", "host"
    DhcpOptionCode option_code;
    bool inherit;                  // true to inherit, false to override
    std::vector<uint8_t> override_value;
    std::string condition;         // Optional condition for inheritance
    
    OptionInheritanceRule() = default;
    OptionInheritanceRule(const std::string& src, const std::string& tgt, 
                         DhcpOptionCode code, bool inh = true)
        : source_scope(src), target_scope(tgt), option_code(code), inherit(inh) {}
};

/**
 * @brief DHCP options context
 */
struct OptionsContext {
    std::string client_mac;
    std::string client_id;
    std::string vendor_class;
    std::string user_class;
    std::string subnet_name;
    std::string pool_name;
    std::string hostname;
    IpAddress requested_ip;
    std::map<std::string, std::string> custom_attributes;
    
    OptionsContext() = default;
};

/**
 * @brief Advanced DHCP options manager
 */
class DhcpOptionsManager {
public:
    /**
     * @brief Constructor
     */
    DhcpOptionsManager();
    
    /**
     * @brief Destructor
     */
    ~DhcpOptionsManager();
    
    // Option registration and management
    /**
     * @brief Register standard DHCP option
     * @param option_code Option code
     * @param name Option name
     * @param description Option description
     * @param template_data Option template
     */
    void register_standard_option(DhcpOptionCode option_code, const std::string& name,
                                 const std::string& description, const OptionTemplate& template_data);
    
    /**
     * @brief Register vendor-specific option
     * @param option_code Option code
     * @param vendor_class Vendor class identifier
     * @param name Option name
     * @param description Option description
     * @param template_data Option template
     */
    void register_vendor_option(DhcpOptionCode option_code, const std::string& vendor_class,
                               const std::string& name, const std::string& description,
                               const OptionTemplate& template_data);
    
    /**
     * @brief Register custom option
     * @param option_code Option code
     * @param name Option name
     * @param description Option description
     * @param template_data Option template
     */
    void register_custom_option(DhcpOptionCode option_code, const std::string& name,
                               const std::string& description, const OptionTemplate& template_data);
    
    /**
     * @brief Get option template
     * @param option_code Option code
     * @param vendor_class Vendor class (for vendor-specific options)
     * @return Option template if found, nullptr otherwise
     */
    std::shared_ptr<OptionTemplate> get_option_template(DhcpOptionCode option_code,
                                                       const std::string& vendor_class = "");
    
    /**
     * @brief Get all registered options
     * @return Map of option codes to templates
     */
    std::map<DhcpOptionCode, std::shared_ptr<OptionTemplate>> get_all_options();
    
    // Option validation
    /**
     * @brief Validate option value
     * @param option_code Option code
     * @param value Option value
     * @param context Options context
     * @return Validation result
     */
    OptionValidationResult validate_option(DhcpOptionCode option_code, 
                                         const std::vector<uint8_t>& value,
                                         const OptionsContext& context);
    
    /**
     * @brief Validate option set
     * @param options Map of options to validate
     * @param context Options context
     * @return Map of validation results
     */
    std::map<DhcpOptionCode, OptionValidationResult> validate_options(
        const std::map<DhcpOptionCode, std::vector<uint8_t>>& options,
        const OptionsContext& context);
    
    /**
     * @brief Set custom validator for option
     * @param option_code Option code
     * @param validator Custom validator function
     */
    void set_custom_validator(DhcpOptionCode option_code,
                             std::function<OptionValidationResult(const std::vector<uint8_t>&, 
                                                                 const OptionsContext&)> validator);
    
    // Option inheritance
    /**
     * @brief Add inheritance rule
     * @param rule Inheritance rule
     */
    void add_inheritance_rule(const OptionInheritanceRule& rule);
    
    /**
     * @brief Remove inheritance rule
     * @param source_scope Source scope
     * @param target_scope Target scope
     * @param option_code Option code
     */
    void remove_inheritance_rule(const std::string& source_scope,
                                const std::string& target_scope,
                                DhcpOptionCode option_code);
    
    /**
     * @brief Get inheritance rules
     * @return Vector of inheritance rules
     */
    std::vector<OptionInheritanceRule> get_inheritance_rules();
    
    /**
     * @brief Apply inheritance rules
     * @param global_options Global options
     * @param subnet_options Subnet options
     * @param pool_options Pool options
     * @param host_options Host options
     * @param context Options context
     * @return Final options after inheritance
     */
    std::map<DhcpOptionCode, std::vector<uint8_t>> apply_inheritance(
        const std::map<DhcpOptionCode, std::vector<uint8_t>>& global_options,
        const std::map<DhcpOptionCode, std::vector<uint8_t>>& subnet_options,
        const std::map<DhcpOptionCode, std::vector<uint8_t>>& pool_options,
        const std::map<DhcpOptionCode, std::vector<uint8_t>>& host_options,
        const OptionsContext& context);
    
    // Option templates
    /**
     * @brief Create option template
     * @param name Template name
     * @param description Template description
     * @param options Map of options in template
     * @return Template ID
     */
    std::string create_template(const std::string& name, const std::string& description,
                               const std::map<DhcpOptionCode, std::vector<uint8_t>>& options);
    
    /**
     * @brief Apply option template
     * @param template_id Template ID
     * @param context Options context
     * @return Options from template
     */
    std::map<DhcpOptionCode, std::vector<uint8_t>> apply_template(
        const std::string& template_id, const OptionsContext& context);
    
    /**
     * @brief Get template by ID
     * @param template_id Template ID
     * @return Template if found, nullptr otherwise
     */
    std::shared_ptr<std::map<DhcpOptionCode, std::vector<uint8_t>>> get_template(
        const std::string& template_id);
    
    /**
     * @brief List all templates
     * @return Map of template IDs to names
     */
    std::map<std::string, std::string> list_templates();
    
    // Option processing
    /**
     * @brief Process client options request
     * @param requested_options Client requested options
     * @param context Options context
     * @return Processed options for client
     */
    std::map<DhcpOptionCode, std::vector<uint8_t>> process_client_request(
        const std::vector<DhcpOptionCode>& requested_options,
        const OptionsContext& context);
    
    /**
     * @brief Generate option 82 (Relay Agent Information)
     * @param circuit_id Circuit ID
     * @param remote_id Remote ID
     * @param subscriber_id Subscriber ID
     * @return Option 82 data
     */
    std::vector<uint8_t> generate_option_82(const std::string& circuit_id,
                                           const std::string& remote_id = "",
                                           const std::string& subscriber_id = "");
    
    /**
     * @brief Parse option 82 (Relay Agent Information)
     * @param option_data Option 82 data
     * @return Map of sub-options
     */
    std::map<uint8_t, std::vector<uint8_t>> parse_option_82(
        const std::vector<uint8_t>& option_data);
    
    // Configuration management
    /**
     * @brief Load options configuration
     * @param config_file Configuration file path
     * @return true if loaded successfully
     */
    bool load_configuration(const std::string& config_file);
    
    /**
     * @brief Save options configuration
     * @param config_file Configuration file path
     * @return true if saved successfully
     */
    bool save_configuration(const std::string& config_file);
    
    /**
     * @brief Reset to default configuration
     */
    void reset_to_defaults();
    
    // Statistics and monitoring
    /**
     * @brief Get option usage statistics
     * @return Map of option codes to usage counts
     */
    std::map<DhcpOptionCode, size_t> get_option_usage_stats();
    
    /**
     * @brief Get validation statistics
     * @return Map of validation results counts
     */
    std::map<std::string, size_t> get_validation_stats();
    
    /**
     * @brief Clear statistics
     */
    void clear_statistics();

private:
    std::map<DhcpOptionCode, std::shared_ptr<OptionTemplate>> standard_options_;
    std::map<std::string, std::map<DhcpOptionCode, std::shared_ptr<OptionTemplate>>> vendor_options_;
    std::map<DhcpOptionCode, std::shared_ptr<OptionTemplate>> custom_options_;
    std::vector<OptionInheritanceRule> inheritance_rules_;
    std::map<std::string, std::map<DhcpOptionCode, std::vector<uint8_t>>> option_templates_;
    
    std::map<DhcpOptionCode, std::function<OptionValidationResult(const std::vector<uint8_t>&, 
                                                                 const OptionsContext&)>> custom_validators_;
    
    std::map<DhcpOptionCode, size_t> option_usage_stats_;
    std::map<std::string, size_t> validation_stats_;
    
    mutable std::mutex mutex_;
    
    /**
     * @brief Initialize standard DHCP options
     */
    void initialize_standard_options();
    
    /**
     * @brief Validate option length
     * @param option_code Option code
     * @param value Option value
     * @return Validation result
     */
    OptionValidationResult validate_length(DhcpOptionCode option_code, 
                                         const std::vector<uint8_t>& value);
    
    /**
     * @brief Validate option format
     * @param option_code Option code
     * @param value Option value
     * @return Validation result
     */
    OptionValidationResult validate_format(DhcpOptionCode option_code, 
                                         const std::vector<uint8_t>& value);
    
    /**
     * @brief Validate option range
     * @param option_code Option code
     * @param value Option value
     * @return Validation result
     */
    OptionValidationResult validate_range(DhcpOptionCode option_code, 
                                        const std::vector<uint8_t>& value);
    
    /**
     * @brief Check inheritance condition
     * @param rule Inheritance rule
     * @param context Options context
     * @return true if condition is met
     */
    bool check_inheritance_condition(const OptionInheritanceRule& rule,
                                    const OptionsContext& context);
    
    /**
     * @brief Update usage statistics
     * @param option_code Option code
     */
    void update_usage_stats(DhcpOptionCode option_code);
    
    /**
     * @brief Update validation statistics
     * @param result Validation result
     */
    void update_validation_stats(const OptionValidationResult& result);
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_OPTIONS_MANAGER_HPP
