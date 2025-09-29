/**
 * @file dhcp_server.cpp
 * @brief Main DHCP server implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "dhcp_server.hpp"
#include "dhcp_utils.hpp"
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
        
        // Initialize lease manager
        lease_manager_ = std::make_unique<LeaseManager>(config);
        lease_manager_->start();
        
        // Load existing leases
        if (!config.lease_file.empty()) {
            lease_manager_->load_leases(config.lease_file);
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
        
        // Reinitialize lease manager
        lease_manager_ = std::make_unique<LeaseManager>(config);
        lease_manager_->start();
        
        LOG_INFO("Configuration reloaded successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to reload configuration: " + std::string(e.what()));
        throw DhcpServerException("Configuration reload failed: " + std::string(e.what()));
    }
}

DhcpStats DhcpServer::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!lease_manager_) {
        return DhcpStats();
    }
    
    return lease_manager_->get_statistics();
}

void DhcpServer::set_signal_handler(std::function<void(int)> handler) {
    // Note: Signal handlers are set up in main.cpp
    // This method is kept for interface compatibility
}

void DhcpServer::handle_dhcp_message(const std::vector<uint8_t>& data, const std::string& client_address, uint16_t client_port) {
    try {
        // Parse DHCP message
        DhcpMessage message = DhcpParser::parse_message(data);
        
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
        // Mark IP as declined (simplified)
        LOG_INFO("Client declined IP " + ip_to_string(message.client_ip) + 
                 " for " + mac_to_string(message.client_mac));
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling DHCP Decline: " + std::string(e.what()));
    }
}

void DhcpServer::handle_inform(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        // Handle inform request (client already has IP)
        LOG_INFO("Received DHCP Inform from " + mac_to_string(message.client_mac));
        
        // Send ACK with configuration information
        // This would be implemented based on specific requirements
        
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
        
        // Build offer message
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::OFFER)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_your_ip(lease.ip_address)
               .set_server_ip(string_to_ip("192.168.1.1")) // Simplified
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::OFFER)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, 
                          string_to_ip("192.168.1.1")); // Simplified
        
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
        
        // Send offer
        socket_manager_->send_dhcp_message(offer, client_address, client_port);
        
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
        
        // Build ACK message
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::ACK)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_your_ip(lease.ip_address)
               .set_server_ip(string_to_ip("192.168.1.1")) // Simplified
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::ACK)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, 
                          string_to_ip("192.168.1.1")); // Simplified
        
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
        
        // Send ACK
        socket_manager_->send_dhcp_message(ack, client_address, client_port);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error sending DHCP ACK: " + std::string(e.what()));
    }
}

void DhcpServer::send_nak(const DhcpMessage& message, const std::string& client_address, uint16_t client_port) {
    try {
        // Build NAK message
        DhcpMessageBuilder builder;
        builder.set_message_type(DhcpMessageType::NAK)
               .set_transaction_id(message.header.xid)
               .set_client_mac(message.client_mac)
               .set_server_ip(string_to_ip("192.168.1.1")) // Simplified
               .add_option(DhcpOptionCode::DHCP_MESSAGE_TYPE, 
                          std::vector<uint8_t>{message_type_to_option_value(DhcpMessageType::NAK)})
               .add_option_ip(DhcpOptionCode::SERVER_IDENTIFIER, 
                          string_to_ip("192.168.1.1")); // Simplified
        
        DhcpMessage nak = builder.build();
        
        // Send NAK
        socket_manager_->send_dhcp_message(nak, client_address, client_port);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error sending DHCP NAK: " + std::string(e.what()));
    }
}

std::string DhcpServer::find_subnet_for_client(const DhcpMessage& message) {
    // Simplified subnet selection - use first subnet
    const auto& config = config_manager_->get_config();
    if (config.subnets.empty()) {
        throw DhcpServerException("No subnets configured");
    }
    
    return config.subnets[0].name;
}

std::vector<DhcpOption> DhcpServer::build_subnet_options(const DhcpSubnet& subnet) {
    std::vector<DhcpOption> options;
    
    // Add subnet mask
    options.push_back({DhcpOptionCode::SUBNET_MASK, {255, 255, 255, 0}});
    
    // Add router
    options.push_back({DhcpOptionCode::ROUTER, {192, 168, 1, 1}});
    
    // Add DNS servers
    options.push_back({DhcpOptionCode::DOMAIN_SERVER, {8, 8, 8, 8, 8, 8, 4, 4}});
    
    // Add domain name
    if (!subnet.domain_name.empty()) {
        options.push_back({DhcpOptionCode::DOMAIN_NAME, 
                          std::vector<uint8_t>(subnet.domain_name.begin(), subnet.domain_name.end())});
    }
    
    return options;
}

std::vector<DhcpOption> DhcpServer::build_lease_options(const DhcpLease& lease, const DhcpSubnet& subnet) {
    std::vector<DhcpOption> options;
    
    // Add lease time
    options.push_back({DhcpOptionCode::IP_ADDRESS_LEASE_TIME, 
                      {0, 0, 0, static_cast<uint8_t>(subnet.lease_time)}});
    
    // Add renewal time
    options.push_back({DhcpOptionCode::RENEWAL_TIME, 
                      {0, 0, 0, static_cast<uint8_t>(subnet.lease_time / 2)}});
    
    // Add rebinding time
    options.push_back({DhcpOptionCode::REBINDING_TIME, 
                      {0, 0, 0, static_cast<uint8_t>((subnet.lease_time * 7) / 8)}});
    
    return options;
}

void DhcpServer::log_dhcp_message(const DhcpMessage& message, const std::string& action) {
    LOG_INFO(action + " DHCP " + get_message_type_name(message.message_type) + 
             " from " + mac_to_string(message.client_mac) + 
             " (" + ip_to_string(message.client_ip) + ")");
}

void DhcpServer::update_statistics(DhcpMessageType message_type) {
    // Update statistics (simplified)
    // This would be implemented based on specific requirements
}

} // namespace simple_dhcpd
