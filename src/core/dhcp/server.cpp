/**
 * @file server.cpp
 * @brief Main DHCP server implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-dhcpd/core/server.hpp"
#include "simple-dhcpd/production/features/advanced_manager.hpp"
#include "simple-dhcpd/core/utils/utils.hpp"
#include <csignal>
#include <cstring>

namespace simple_dhcpd {

DhcpServer::DhcpServer(const std::string& config_file)
    : config_file_(config_file), running_(false), initialized_(false) {
    LOG_DEBUG("DHCP server created");
}

DhcpServer::~DhcpServer() {
    stop();
}

void DhcpServer::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    try {
        // Initialize configuration manager
        config_manager_ = std::make_unique<ConfigManager>();
        if (!config_file_.empty()) {
            config_manager_->load_config(config_file_);
        } else {
            config_manager_->set_config(get_default_config());
        }
        
        // Initialize logger
        const auto& config = config_manager_->get_config();
        if (config.enable_logging) {
            init_logger(config.log_file, LogLevel::INFO);
        } else {
            init_logger("", LogLevel::WARN);
        }
        
        // Initialize socket manager
        socket_manager_ = std::make_unique<DhcpSocketManager>();
        socket_manager_->initialize(config);
        
        if (!config.advanced_lease_database.empty()) {
            lease_manager_ = std::make_unique<AdvancedLeaseManager>(config, config.advanced_lease_database);
        } else {
            lease_manager_ = std::make_unique<LeaseManager>(config);
        }
        lease_manager_->start();

        if (!config.lease_file.empty() && config.advanced_lease_database.empty()) {
            lease_manager_->load_leases(config.lease_file);
        }

        if (config.enable_security) {
            security_manager_ = std::make_unique<DhcpSecurityManager>();
            security_manager_->start();
            if (!config.security_policy_file.empty()) {
                security_manager_->load_security_configuration(config.security_policy_file);
            }
        }
        
        initialized_ = true;
        LOG_INFO("DHCP server initialized successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize DHCP server: " + std::string(e.what()));
        throw DhcpServerException("Initialization failed: " + std::string(e.what()));
    }
}

void DhcpServer::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        throw DhcpServerException("Server not initialized");
    }
    
    if (running_) {
        return;
    }
    
    try {
        // Start socket manager
        socket_manager_->start_all([this](const std::vector<uint8_t>& data, const std::string& client_address, uint16_t client_port) {
            handle_dhcp_message(data, client_address, client_port);
        });
        
        running_ = true;
        LOG_INFO("DHCP server started");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to start DHCP server: " + std::string(e.what()));
        throw DhcpServerException("Start failed: " + std::string(e.what()));
    }
}

void DhcpServer::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!running_) {
        return;
    }
    
    try {
        // Stop socket manager
        if (socket_manager_) {
            socket_manager_->stop_all();
        }
        
        // Save leases
        if (lease_manager_ && !config_manager_->get_config().lease_file.empty()) {
            lease_manager_->save_leases(config_manager_->get_config().lease_file);
        }
        
        if (security_manager_) {
            security_manager_->stop();
            security_manager_.reset();
        }

        // Stop lease manager
        if (lease_manager_) {
            lease_manager_->stop();
        }
        
        running_ = false;
        LOG_INFO("DHCP server stopped");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error stopping DHCP server: " + std::string(e.what()));
    }
}

bool DhcpServer::is_running() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
}

void DhcpServer::reload_config() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        throw DhcpServerException("Server not initialized");
    }
    
    try {
        // Reload configuration
        config_manager_->reload_config();
        
        // Reinitialize components with new configuration
        const auto& config = config_manager_->get_config();
        
        // Update logger
        if (config.enable_logging) {
            init_logger(config.log_file, LogLevel::INFO);
        }
        
        // Reinitialize socket manager
        socket_manager_->initialize(config);

        if (security_manager_) {
            security_manager_->stop();
            security_manager_.reset();
        }
        if (config.enable_security) {
            security_manager_ = std::make_unique<DhcpSecurityManager>();
            security_manager_->start();
            if (!config.security_policy_file.empty()) {
                security_manager_->load_security_configuration(config.security_policy_file);
            }
        }

        if (!config.advanced_lease_database.empty()) {
            lease_manager_ = std::make_unique<AdvancedLeaseManager>(config, config.advanced_lease_database);
        } else {
            lease_manager_ = std::make_unique<LeaseManager>(config);
        }
        lease_manager_->start();
        if (!config.lease_file.empty() && config.advanced_lease_database.empty()) {
            lease_manager_->load_leases(config.lease_file);
        }
        
        LOG_INFO("Configuration reloaded successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to reload configuration: " + std::string(e.what()));
        throw DhcpServerException("Configuration reload failed: " + std::string(e.what()));
    }
}

DhcpStats DhcpServer::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    DhcpStats merged;
    if (lease_manager_) {
        merged = lease_manager_->get_statistics();
    }
    {
        std::lock_guard<std::mutex> sl(stats_mutex_);
        merged.total_requests = packet_stats_.total_requests;
        merged.discover_count = packet_stats_.discover_count;
        merged.request_count = packet_stats_.request_count;
        merged.release_count = packet_stats_.release_count;
        merged.decline_count = packet_stats_.decline_count;
        merged.inform_count = packet_stats_.inform_count;
        merged.offer_count = packet_stats_.offer_count;
        merged.ack_count = packet_stats_.ack_count;
        merged.nak_count = packet_stats_.nak_count;
        merged.total_errors = packet_stats_.total_errors;
    }
    return merged;
}

IpAddress DhcpServer::dhcp_server_ip(const DhcpSubnet* subnet) const {
    const auto& c = config_manager_->get_config();
    if (c.server_identifier != 0) {
        return c.server_identifier;
    }
    if (subnet && subnet->gateway != 0) {
        return subnet->gateway;
    }
    if (!c.subnets.empty() && c.subnets[0].gateway != 0) {
        return c.subnets[0].gateway;
    }
    return string_to_ip("192.168.1.1");
}

bool DhcpServer::security_allow_message(const DhcpMessage& message, const std::string& recv_interface) {
    if (!security_manager_) {
        return true;
    }
    if (!security_manager_->validate_dhcp_message(message, recv_interface)) {
        return false;
    }
    const std::string mac_str = mac_to_string(message.client_mac);
    if (!security_manager_->check_mac_address(mac_str)) {
        return false;
    }
    if (!security_manager_->check_ip_address(message.client_ip)) {
        return false;
    }
    if (!security_manager_->check_rate_limit(mac_str, "mac")) {
        return false;
    }
    for (const auto& opt : message.options) {
        if (opt.code == DhcpOptionCode::RELAY_AGENT_INFORMATION) {
            if (!security_manager_->validate_option_82(opt.data, recv_interface)) {
                return false;
            }
        }
    }
    return true;
}

void DhcpServer::set_signal_handler(std::function<void(int)> handler) {
    // Note: Signal handlers are set up in main.cpp
    // This method is kept for interface compatibility
}

void DhcpServer::handle_dhcp_message(const std::vector<uint8_t>& data, const std::string& client_address, uint16_t client_port) {
    try {
        // Parse DHCP message
        DhcpMessage message = DhcpParser::parse_message(data);

        if (!security_allow_message(message, std::string())) {
            LOG_WARN("DHCP message rejected by security policy");
            std::lock_guard<std::mutex> sl(stats_mutex_);
            packet_stats_.total_errors++;
            return;
        }
        
        // Log message
        log_dhcp_message(message, "Received");
        
        // Update statistics
        update_statistics(message.message_type);
        
        // Handle message based on type
        switch (message.message_type) {
            case DhcpMessageType::DISCOVER:
                handle_discover(message, client_address, client_port);
                break;
                
            case DhcpMessageType::REQUEST:
                handle_request(message, client_address, client_port);
                break;
                
            case DhcpMessageType::RELEASE:
                handle_release(message, client_address, client_port);
                break;
                
            case DhcpMessageType::DECLINE:
                handle_decline(message, client_address, client_port);
                break;
                
            case DhcpMessageType::INFORM:
                handle_inform(message, client_address, client_port);
                break;
                
            default:
                LOG_WARN("Unsupported DHCP message type: " + get_message_type_name(message.message_type));
                break;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP message: " + std::string(e.what()));
        std::lock_guard<std::mutex> sl(stats_mutex_);
        packet_stats_.total_errors++;
    }
}

void DhcpServer::handle_discover(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        // Find appropriate subnet
        std::string subnet_name = find_subnet_for_client(message);
        
        // Allocate lease
        DhcpLease lease = lease_manager_->allocate_lease(message.client_mac, message.client_ip, subnet_name);
        
        // Send offer
        send_offer(message, lease, client_address, client_port);
        
        LOG_INFO("Sent DHCP Offer to " + mac_to_string(message.client_mac) + 
                 " for " + ip_to_string(lease.ip_address));
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP Discover: " + std::string(e.what()));
    }
}

void DhcpServer::handle_request(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        // Check if client has existing lease
        auto existing_lease = lease_manager_->get_lease_by_mac(message.client_mac);
        
        if (existing_lease) {
            // Renew existing lease
            DhcpLease lease = lease_manager_->renew_lease(message.client_mac, message.client_ip);
            
            // Send ACK
            send_ack(message, lease, client_address, client_port);
            
            LOG_INFO("Sent DHCP ACK to " + mac_to_string(message.client_mac) + 
                     " for " + ip_to_string(lease.ip_address));
        } else {
            // Allocate new lease
            std::string subnet_name = find_subnet_for_client(message);
            DhcpLease lease = lease_manager_->allocate_lease(message.client_mac, message.client_ip, subnet_name);
            
            // Send ACK
            send_ack(message, lease, client_address, client_port);
            
            LOG_INFO("Sent DHCP ACK to " + mac_to_string(message.client_mac) + 
                     " for " + ip_to_string(lease.ip_address));
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP Request: " + std::string(e.what()));
        
        // Send NAK
        send_nak(message, client_address, client_port);
    }
}

void DhcpServer::handle_release(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        // Release lease
        bool released = lease_manager_->release_lease(message.client_mac, message.client_ip);
        
        if (released) {
            LOG_INFO("Released lease for " + mac_to_string(message.client_mac) + 
                     " at " + ip_to_string(message.client_ip));
        } else {
            LOG_WARN("Failed to release lease for " + mac_to_string(message.client_mac) + 
                     " at " + ip_to_string(message.client_ip));
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP Release: " + std::string(e.what()));
    }
}

void DhcpServer::handle_decline(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        auto existing_lease = lease_manager_->get_lease_by_mac(message.client_mac);
        IpAddress declined_ip = message.client_ip;
        if (declined_ip == 0 && existing_lease) {
            declined_ip = existing_lease->ip_address;
        }
        LOG_INFO("Client declined IP " + ip_to_string(declined_ip) +
                 " for " + mac_to_string(message.client_mac));

        if (existing_lease) {
            lease_manager_->release_lease(message.client_mac, existing_lease->ip_address);
        }
        if (declined_ip != 0) {
            const auto& cfg = config_manager_->get_config();
            lease_manager_->add_declined_ip(declined_ip, std::chrono::seconds(cfg.decline_hold_seconds));
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP Decline: " + std::string(e.what()));
    }
}

void DhcpServer::handle_inform(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        // Handle inform request (client already has IP)
        LOG_INFO("Received DHCP Inform from " + mac_to_string(message.client_mac));
        
        // Find appropriate subnet
        std::string subnet_name = find_subnet_for_client(message);
        const auto& config = config_manager_->get_config();
        const DhcpSubnet* subnet = nullptr;
        
        for (const auto& s : config.subnets) {
            if (s.name == subnet_name) {
                subnet = &s;
                break;
            }
        }
        
        if (!subnet) {
            LOG_WARN("No subnet found for DHCP Inform from " + mac_to_string(message.client_mac));
            return;
        }

        const IpAddress sid = dhcp_server_ip(subnet);
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::ACK)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_your_ip(message.client_ip)
               .set_server_ip(sid)
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::ACK)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, sid);
        
        // Add subnet options
        auto options = build_subnet_options(*subnet);
        for (const auto& option : options) {
            builder.add_option(option);
        }
        
        DhcpMessage ack = builder.build();
        
        // Send ACK
        socket_manager_->send_dhcp_message(ack, client_address, client_port);
        {
            std::lock_guard<std::mutex> sl(stats_mutex_);
            packet_stats_.ack_count++;
        }
        
        LOG_INFO("Sent DHCP ACK to " + mac_to_string(message.client_mac) + " for Inform");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP Inform: " + std::string(e.what()));
    }
}

void DhcpServer::send_offer(const DhcpMessage& message, const DhcpLease& lease, const std::string& client_address, uint16_t client_port) {
    try {
        // Find subnet for lease
        std::string subnet_name = find_subnet_for_client(message);
        const auto& config = config_manager_->get_config();
        const DhcpSubnet* subnet = nullptr;
        
        for (const auto& s : config.subnets) {
            if (s.name == subnet_name) {
                subnet = &s;
                break;
            }
        }
        
        if (!subnet) {
            throw DhcpServerException("Subnet not found: " + subnet_name);
        }

        const IpAddress sid = dhcp_server_ip(subnet);
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::OFFER)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_your_ip(lease.ip_address)
               .set_server_ip(sid)
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::OFFER)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, sid);
        
        // Add subnet options
        auto options = build_subnet_options(*subnet);
        for (const auto& option : options) {
            builder.add_option(option);
        }
        
        // Add lease options
        auto lease_options = build_lease_options(lease, *subnet);
        for (const auto& option : lease_options) {
            builder.add_option(option);
        }
        
        DhcpMessage offer = builder.build();
        
        socket_manager_->send_dhcp_message(offer, client_address, client_port);
        {
            std::lock_guard<std::mutex> sl(stats_mutex_);
            packet_stats_.offer_count++;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error sending DHCP Offer: " + std::string(e.what()));
    }
}

void DhcpServer::send_ack(const DhcpMessage& message, const DhcpLease& lease, const std::string& client_address, uint16_t client_port) {
    try {
        // Find subnet for lease
        std::string subnet_name = find_subnet_for_client(message);
        const auto& config = config_manager_->get_config();
        const DhcpSubnet* subnet = nullptr;
        
        for (const auto& s : config.subnets) {
            if (s.name == subnet_name) {
                subnet = &s;
                break;
            }
        }
        
        if (!subnet) {
            throw DhcpServerException("Subnet not found: " + subnet_name);
        }

        const IpAddress sid = dhcp_server_ip(subnet);
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::ACK)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_your_ip(lease.ip_address)
               .set_server_ip(sid)
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::ACK)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, sid);
        
        // Add subnet options
        auto options = build_subnet_options(*subnet);
        for (const auto& option : options) {
            builder.add_option(option);
        }
        
        // Add lease options
        auto lease_options = build_lease_options(lease, *subnet);
        for (const auto& option : lease_options) {
            builder.add_option(option);
        }
        
        DhcpMessage ack = builder.build();
        
        socket_manager_->send_dhcp_message(ack, client_address, client_port);
        {
            std::lock_guard<std::mutex> sl(stats_mutex_);
            packet_stats_.ack_count++;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error sending DHCP ACK: " + std::string(e.what()));
    }
}

void DhcpServer::send_nak(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        const IpAddress sid = dhcp_server_ip(nullptr);
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::NAK)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_server_ip(sid)
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::NAK)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, sid);
        
        DhcpMessage nak = builder.build();
        
        socket_manager_->send_dhcp_message(nak, client_address, client_port);
        {
            std::lock_guard<std::mutex> sl(stats_mutex_);
            packet_stats_.nak_count++;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error sending DHCP NAK: " + std::string(e.what()));
    }
}

std::string DhcpServer::find_subnet_for_client(const DhcpMessage& message) {
    const auto& config = config_manager_->get_config();
    if (config.subnets.empty()) {
        throw DhcpServerException("No subnets configured");
    }
    
    // If client has an IP, try to find matching subnet
    if (message.client_ip != 0) {
        for (const auto& subnet : config.subnets) {
            if (is_ip_in_subnet(message.client_ip, subnet)) {
                return subnet.name;
            }
        }
    }
    
    // If client has relay IP, try to find matching subnet
    if (message.relay_ip != 0) {
        for (const auto& subnet : config.subnets) {
            if (is_ip_in_subnet(message.relay_ip, subnet)) {
                return subnet.name;
            }
        }
    }
    
    // Default to first subnet
    return config.subnets[0].name;
}

std::vector<DhcpOption> DhcpServer::build_subnet_options(const DhcpSubnet& subnet) {
    std::vector<DhcpOption> options;
    
    const IpAddress mask = subnet_mask_for_prefix(subnet.prefix_length);
    options.push_back({DhcpOptionCode::SUBNET_MASK, ip_to_bytes_be(mask)});
    
    if (subnet.gateway != 0) {
        options.push_back({DhcpOptionCode::ROUTER, ip_to_bytes_be(subnet.gateway)});
    }
    
    if (!subnet.dns_servers.empty()) {
        std::vector<uint8_t> dns_data;
        for (IpAddress d : subnet.dns_servers) {
            const auto o = ip_to_bytes_be(d);
            dns_data.insert(dns_data.end(), o.begin(), o.end());
        }
        options.push_back({DhcpOptionCode::DOMAIN_SERVER, std::move(dns_data)});
    }
    
    if (!subnet.domain_name.empty()) {
        options.push_back({DhcpOptionCode::DOMAIN_NAME, 
                          std::vector<uint8_t>(subnet.domain_name.begin(), subnet.domain_name.end())});
    }
    
    return options;
}

std::vector<DhcpOption> DhcpServer::build_lease_options(const DhcpLease& lease, const DhcpSubnet& subnet) {
    std::vector<DhcpOption> options;
    
    options.push_back({DhcpOptionCode::IP_ADDRESS_LEASE_TIME, uint32_to_option_bytes(subnet.lease_time)});
    
    const uint32_t renewal = subnet.lease_time / 2;
    const uint32_t rebind = (subnet.lease_time * 7) / 8;
    options.push_back({DhcpOptionCode::RENEWAL_TIME, uint32_to_option_bytes(renewal)});
    options.push_back({DhcpOptionCode::REBINDING_TIME, uint32_to_option_bytes(rebind)});
    
    return options;
}

void DhcpServer::log_dhcp_message(const DhcpMessage& message, const std::string& action) {
    LOG_INFO(action + " DHCP " + get_message_type_name(message.message_type) + 
             " from " + mac_to_string(message.client_mac) + 
             " (" + ip_to_string(message.client_ip) + ")");
}

void DhcpServer::update_statistics(DhcpMessageType message_type) {
    std::lock_guard<std::mutex> sl(stats_mutex_);
    packet_stats_.total_requests++;
    switch (message_type) {
        case DhcpMessageType::DISCOVER: packet_stats_.discover_count++; break;
        case DhcpMessageType::REQUEST: packet_stats_.request_count++; break;
        case DhcpMessageType::RELEASE: packet_stats_.release_count++; break;
        case DhcpMessageType::DECLINE: packet_stats_.decline_count++; break;
        case DhcpMessageType::INFORM: packet_stats_.inform_count++; break;
        default: break;
    }
}

bool DhcpServer::is_ip_in_subnet(IpAddress ip, const DhcpSubnet& subnet) {
    // Check if IP is within the subnet range
    return ntohl(ip) >= ntohl(subnet.range_start) && ntohl(ip) <= ntohl(subnet.range_end);
}

} // namespace simple_dhcpd
