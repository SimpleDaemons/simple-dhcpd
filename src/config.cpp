/**
 * @file config.cpp
 * @brief Configuration management implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "config.hpp"
#include "logger.hpp"
#include "dhcp_utils.hpp"
#include <fstream>
#include <sstream>
#include <json/json.h>

namespace simple_dhcpd {

ConfigManager::ConfigManager() : loaded_(false) {
    config_ = get_default_config();
}

ConfigManager::~ConfigManager() = default;

void ConfigManager::load_config(const std::string& config_file) {
    config_file_ = config_file;
    
    std::ifstream file(config_file);
    if (!file.is_open()) {
        throw ConfigException("Cannot open configuration file: " + config_file);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    load_config_from_json(buffer.str());
    loaded_ = true;
    
    LOG_INFO("Configuration loaded from: " + config_file);
}

void ConfigManager::load_config_from_json(const std::string& json_config) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream json_stream(json_config);
    if (!Json::parseFromStream(builder, json_stream, &root, &errors)) {
        throw ConfigException("JSON parsing error: " + errors);
    }
    
    parse_json_config(json_config);
}

void ConfigManager::save_config(const std::string& config_file) const {
    Json::Value root;
    
    // Basic configuration
    root["dhcp"]["listen"] = Json::Value(Json::arrayValue);
    for (const auto& addr : config_.listen_addresses) {
        root["dhcp"]["listen"].append(addr);
    }
    
    // Subnets
    root["dhcp"]["subnets"] = Json::Value(Json::arrayValue);
    for (const auto& subnet : config_.subnets) {
        Json::Value subnet_json;
        subnet_json["name"] = subnet.name;
        subnet_json["network"] = ip_to_string(subnet.network) + "/" + std::to_string(subnet.prefix_length);
        subnet_json["range"] = ip_to_string(subnet.range_start) + "-" + ip_to_string(subnet.range_end);
        subnet_json["gateway"] = ip_to_string(subnet.gateway);
        subnet_json["domain_name"] = subnet.domain_name;
        subnet_json["lease_time"] = subnet.lease_time;
        subnet_json["max_lease_time"] = subnet.max_lease_time;
        
        // DNS servers
        subnet_json["dns_servers"] = Json::Value(Json::arrayValue);
        for (const auto& dns : subnet.dns_servers) {
            subnet_json["dns_servers"].append(ip_to_string(dns));
        }
        
        // Options
        subnet_json["options"] = Json::Value(Json::arrayValue);
        for (const auto& option : subnet.options) {
            Json::Value option_json;
            option_json["name"] = get_option_name(option.code);
            option_json["value"] = std::string(option.data.begin(), option.data.end());
            subnet_json["options"].append(option_json);
        }
        
        root["dhcp"]["subnets"].append(subnet_json);
    }
    
    // Global options
    root["dhcp"]["global_options"] = Json::Value(Json::arrayValue);
    for (const auto& option : config_.global_options) {
        Json::Value option_json;
        option_json["name"] = get_option_name(option.code);
        option_json["value"] = std::string(option.data.begin(), option.data.end());
        root["dhcp"]["global_options"].append(option_json);
    }
    
    // Security settings
    root["dhcp"]["security"]["enable"] = config_.enable_security;
    
    // Performance settings
    root["dhcp"]["performance"]["max_leases"] = static_cast<int>(config_.max_leases);
    
    // Logging settings
    root["dhcp"]["logging"]["enable"] = config_.enable_logging;
    root["dhcp"]["logging"]["log_file"] = config_.log_file;
    
    // Write to file
    std::ofstream file(config_file);
    if (!file.is_open()) {
        throw ConfigException("Cannot open configuration file for writing: " + config_file);
    }
    
    Json::StreamWriterBuilder writer_builder;
    writer_builder["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(writer_builder.newStreamWriter());
    writer->write(root, &file);
    file.close();
    
    LOG_INFO("Configuration saved to: " + config_file);
}

const DhcpConfig& ConfigManager::get_config() const {
    return config_;
}

void ConfigManager::set_config(const DhcpConfig& config) {
    config_ = config;
    validate_config();
}

void ConfigManager::validate_config() const {
    if (config_.listen_addresses.empty()) {
        throw ConfigException("No listen addresses configured");
    }
    
    if (config_.subnets.empty()) {
        throw ConfigException("No subnets configured");
    }
    
    for (const auto& subnet : config_.subnets) {
        validate_subnet_config(subnet);
    }
    
    LOG_DEBUG("Configuration validation passed");
}

void ConfigManager::reload_config() {
    if (config_file_.empty()) {
        throw ConfigException("No configuration file specified for reload");
    }
    
    load_config(config_file_);
    LOG_INFO("Configuration reloaded");
}

bool ConfigManager::is_loaded() const {
    return loaded_;
}

std::string ConfigManager::get_config_file() const {
    return config_file_;
}

void ConfigManager::parse_json_config(const std::string& json_config) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream json_stream(json_config);
    if (!Json::parseFromStream(builder, json_stream, &root, &errors)) {
        throw ConfigException("JSON parsing error: " + errors);
    }
    
    // Parse basic configuration
    if (root.isMember("dhcp")) {
        const Json::Value& dhcp = root["dhcp"];
        
        // Listen addresses
        if (dhcp.isMember("listen")) {
            config_.listen_addresses.clear();
            const Json::Value& listen = dhcp["listen"];
            for (const auto& addr : listen) {
                config_.listen_addresses.push_back(addr.asString());
            }
        }
        
        // Subnets
        if (dhcp.isMember("subnets")) {
            config_.subnets.clear();
            const Json::Value& subnets = dhcp["subnets"];
            for (const auto& subnet : subnets) {
                config_.subnets.push_back(parse_subnet_config(subnet.toStyledString()));
            }
        }
        
        // Global options
        if (dhcp.isMember("global_options")) {
            config_.global_options.clear();
            const Json::Value& options = dhcp["global_options"];
            for (const auto& option : options) {
                config_.global_options.push_back(parse_option_config(option.toStyledString()));
            }
        }
        
        // Security settings
        if (dhcp.isMember("security")) {
            const Json::Value& security = dhcp["security"];
            if (security.isMember("enable")) {
                config_.enable_security = security["enable"].asBool();
            }
        }
        
        // Performance settings
        if (dhcp.isMember("performance")) {
            const Json::Value& performance = dhcp["performance"];
            if (performance.isMember("max_leases")) {
                config_.max_leases = static_cast<uint32_t>(performance["max_leases"].asInt());
            }
        }
        
        // Logging settings
        if (dhcp.isMember("logging")) {
            const Json::Value& logging = dhcp["logging"];
            if (logging.isMember("enable")) {
                config_.enable_logging = logging["enable"].asBool();
            }
            if (logging.isMember("log_file")) {
                config_.log_file = logging["log_file"].asString();
            }
        }
    }
}

DhcpSubnet ConfigManager::parse_subnet_config(const std::string& subnet_json) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream json_stream(subnet_json);
    if (!Json::parseFromStream(builder, json_stream, &root, &errors)) {
        throw ConfigException("Subnet JSON parsing error: " + errors);
    }
    
    DhcpSubnet subnet;
    
    if (root.isMember("name")) {
        subnet.name = root["name"].asString();
    }
    
    if (root.isMember("network")) {
        std::string network_str = root["network"].asString();
        size_t slash_pos = network_str.find('/');
        if (slash_pos != std::string::npos) {
            subnet.network = string_to_ip(network_str.substr(0, slash_pos));
            subnet.prefix_length = static_cast<uint8_t>(std::stoi(network_str.substr(slash_pos + 1)));
        } else {
            throw ConfigException("Invalid network format: " + network_str);
        }
    }
    
    if (root.isMember("range")) {
        std::string range_str = root["range"].asString();
        size_t dash_pos = range_str.find('-');
        if (dash_pos != std::string::npos) {
            subnet.range_start = string_to_ip(range_str.substr(0, dash_pos));
            subnet.range_end = string_to_ip(range_str.substr(dash_pos + 1));
        } else {
            throw ConfigException("Invalid range format: " + range_str);
        }
    }
    
    if (root.isMember("gateway")) {
        subnet.gateway = string_to_ip(root["gateway"].asString());
    }
    
    if (root.isMember("domain_name")) {
        subnet.domain_name = root["domain_name"].asString();
    }
    
    if (root.isMember("lease_time")) {
        subnet.lease_time = static_cast<uint32_t>(root["lease_time"].asInt());
    }
    
    if (root.isMember("max_lease_time")) {
        subnet.max_lease_time = static_cast<uint32_t>(root["max_lease_time"].asInt());
    }
    
    if (root.isMember("dns_servers")) {
        const Json::Value& dns_servers = root["dns_servers"];
        for (const auto& dns : dns_servers) {
            subnet.dns_servers.push_back(string_to_ip(dns.asString()));
        }
    }
    
    if (root.isMember("options")) {
        const Json::Value& options = root["options"];
        for (const auto& option : options) {
            subnet.options.push_back(parse_option_config(option.toStyledString()));
        }
    }
    
    return subnet;
}

DhcpOption ConfigManager::parse_option_config(const std::string& option_json) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream json_stream(option_json);
    if (!Json::parseFromStream(builder, json_stream, &root, &errors)) {
        throw ConfigException("Option JSON parsing error: " + errors);
    }
    
    DhcpOption option;
    
    if (root.isMember("name")) {
        std::string name = root["name"].asString();
        // Convert name to option code (simplified)
        if (name == "subnet-mask") {
            option.code = DhcpOptionCode::SUBNET_MASK;
        } else if (name == "routers") {
            option.code = DhcpOptionCode::ROUTER;
        } else if (name == "domain-name-servers") {
            option.code = DhcpOptionCode::DOMAIN_SERVER;
        } else if (name == "domain-name") {
            option.code = DhcpOptionCode::DOMAIN_NAME;
        } else {
            option.code = DhcpOptionCode::VENDOR_SPECIFIC;
        }
    }
    
    if (root.isMember("value")) {
        std::string value = root["value"].asString();
        option.data = std::vector<uint8_t>(value.begin(), value.end());
        option.length = static_cast<uint8_t>(option.data.size());
    }
    
    return option;
}

void ConfigManager::validate_subnet_config(const DhcpSubnet& subnet) const {
    if (subnet.name.empty()) {
        throw ConfigException("Subnet name cannot be empty");
    }
    
    if (subnet.prefix_length > 32) {
        throw ConfigException("Invalid prefix length: " + std::to_string(subnet.prefix_length));
    }
    
    validate_ip_range(subnet.range_start, subnet.range_end, subnet.network, subnet.prefix_length);
    
    if (subnet.lease_time == 0) {
        throw ConfigException("Lease time cannot be zero");
    }
    
    if (subnet.max_lease_time < subnet.lease_time) {
        throw ConfigException("Max lease time cannot be less than lease time");
    }
}

void ConfigManager::validate_ip_range(IpAddress start, IpAddress end, 
                                     IpAddress network, uint8_t prefix_length) const {
    if (start == 0 || end == 0) {
        throw ConfigException("IP range cannot contain zero addresses");
    }
    
    if (ntohl(start) > ntohl(end)) {
        throw ConfigException("Start IP cannot be greater than end IP");
    }
    
    if (!is_ip_in_subnet(start, network, prefix_length)) {
        throw ConfigException("Start IP is not in subnet");
    }
    
    if (!is_ip_in_subnet(end, network, prefix_length)) {
        throw ConfigException("End IP is not in subnet");
    }
}

DhcpConfig get_default_config() {
    DhcpConfig config;
    
    // Default listen addresses
    config.listen_addresses = {"0.0.0.0:67", "[::]:67"};
    
    // Default subnet
    DhcpSubnet subnet;
    subnet.name = "default";
    subnet.network = string_to_ip("192.168.1.0");
    subnet.prefix_length = 24;
    subnet.range_start = string_to_ip("192.168.1.100");
    subnet.range_end = string_to_ip("192.168.1.200");
    subnet.gateway = string_to_ip("192.168.1.1");
    subnet.domain_name = "local";
    subnet.lease_time = 86400; // 24 hours
    subnet.max_lease_time = 172800; // 48 hours
    
    // Default DNS servers
    subnet.dns_servers = {string_to_ip("8.8.8.8"), string_to_ip("8.8.4.4")};
    
    // Default options
    subnet.options.push_back({DhcpOptionCode::SUBNET_MASK, {255, 255, 255, 0}});
    subnet.options.push_back({DhcpOptionCode::ROUTER, {192, 168, 1, 1}});
    
    config.subnets.push_back(subnet);
    
    // Default global options
    config.global_options.push_back({DhcpOptionCode::DOMAIN_SERVER, {8, 8, 8, 8, 8, 8, 4, 4}});
    
    // Default settings
    config.enable_logging = true;
    config.enable_security = true;
    config.max_leases = 10000;
    config.log_file = "/var/log/simple-dhcpd.log";
    config.lease_file = "/var/lib/simple-dhcpd/leases.db";
    
    return config;
}

void create_sample_config(const std::string& config_file) {
    ConfigManager manager;
    manager.set_config(get_default_config());
    manager.save_config(config_file);
}

} // namespace simple_dhcpd
