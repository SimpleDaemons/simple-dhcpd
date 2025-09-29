/**
 * @file lease_manager.cpp
 * @brief DHCP lease management implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "lease_manager.hpp"
#include "logger.hpp"
#include "dhcp_utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace simple_dhcpd {

LeaseManager::LeaseManager(const DhcpConfig& config) 
    : config_(config), running_(false) {
    LOG_DEBUG("Lease manager initialized");
}

LeaseManager::~LeaseManager() {
    stop();
}

void LeaseManager::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    cleanup_thread_ = std::thread(&LeaseManager::cleanup_expired_leases, this);
    
    LOG_INFO("Lease manager started");
}

void LeaseManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    
    LOG_INFO("Lease manager stopped");
}

DhcpLease LeaseManager::allocate_lease(const MacAddress& mac_address, IpAddress requested_ip, const std::string& subnet_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if client already has a lease
    auto existing_lease = leases_by_mac_.find(mac_address);
    if (existing_lease != leases_by_mac_.end() && existing_lease->second->is_active) {
        // Client already has an active lease, return it
        return *existing_lease->second;
    }
    
    // Get subnet configuration
    const DhcpSubnet& subnet = get_subnet_by_name(subnet_name);
    
    // Determine IP address to allocate
    IpAddress ip_to_allocate = requested_ip;
    
    if (ip_to_allocate == 0) {
        // No specific IP requested, find an available one
        ip_to_allocate = find_available_ip(subnet);
    } else {
        // Check if requested IP is available
        if (!is_ip_available(ip_to_allocate, subnet_name)) {
            throw LeaseManagerException("Requested IP address not available: " + ip_to_string(ip_to_allocate));
        }
    }
    
    // Create new lease
    auto lease = std::make_shared<DhcpLease>();
    lease->mac_address = mac_address;
    lease->ip_address = ip_to_allocate;
    lease->hostname = "";
    lease->lease_start = get_current_time();
    lease->lease_end = calculate_lease_end(lease->lease_start, subnet.lease_time);
    lease->renewal_time = calculate_renewal_time(lease->lease_start, subnet.lease_time);
    lease->rebinding_time = calculate_rebinding_time(lease->lease_start, subnet.lease_time);
    lease->is_static = false;
    lease->is_active = true;
    
    // Add lease to internal structures
    add_lease(lease);
    
    LOG_INFO("Allocated lease: " + mac_to_string(mac_address) + " -> " + ip_to_string(ip_to_allocate));
    
    return *lease;
}

DhcpLease LeaseManager::renew_lease(const MacAddress& mac_address, IpAddress ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find existing lease
    auto lease_it = leases_by_mac_.find(mac_address);
    if (lease_it == leases_by_mac_.end()) {
        throw LeaseManagerException("No lease found for MAC address: " + mac_to_string(mac_address));
    }
    
    auto lease = lease_it->second;
    if (!lease->is_active) {
        throw LeaseManagerException("Lease is not active for MAC address: " + mac_to_string(mac_address));
    }
    
    if (lease->ip_address != ip_address) {
        throw LeaseManagerException("IP address mismatch for lease renewal");
    }
    
    // Get subnet configuration (simplified - use first subnet)
    const DhcpSubnet& subnet = config_.subnets[0];
    
    // Renew lease
    lease->lease_start = get_current_time();
    lease->lease_end = calculate_lease_end(lease->lease_start, subnet.lease_time);
    lease->renewal_time = calculate_renewal_time(lease->lease_start, subnet.lease_time);
    lease->rebinding_time = calculate_rebinding_time(lease->lease_start, subnet.lease_time);
    
    LOG_INFO("Renewed lease: " + mac_to_string(mac_address) + " -> " + ip_to_string(ip_address));
    
    return *lease;
}

bool LeaseManager::release_lease(const MacAddress& mac_address, IpAddress ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find lease by MAC address
    auto lease_it = leases_by_mac_.find(mac_address);
    if (lease_it == leases_by_mac_.end()) {
        return false;
    }
    
    auto lease = lease_it->second;
    if (lease->ip_address != ip_address) {
        return false;
    }
    
    // Release lease
    lease->is_active = false;
    remove_lease(lease);
    
    LOG_INFO("Released lease: " + mac_to_string(mac_address) + " -> " + ip_to_string(ip_address));
    
    return true;
}

std::shared_ptr<DhcpLease> LeaseManager::get_lease_by_mac(const MacAddress& mac_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = leases_by_mac_.find(mac_address);
    if (it != leases_by_mac_.end() && it->second->is_active) {
        return it->second;
    }
    
    return nullptr;
}

std::shared_ptr<DhcpLease> LeaseManager::get_lease_by_ip(IpAddress ip_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = leases_by_ip_.find(ip_address);
    if (it != leases_by_ip_.end() && it->second->is_active) {
        return it->second;
    }
    
    return nullptr;
}

bool LeaseManager::is_ip_available(IpAddress ip_address, const std::string& subnet_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if IP is already leased
    auto it = leases_by_ip_.find(ip_address);
    if (it != leases_by_ip_.end() && it->second->is_active) {
        return false;
    }
    
    // Get subnet configuration
    const DhcpSubnet& subnet = get_subnet_by_name(subnet_name);
    
    // Check if IP is in range
    if (!is_ip_in_range(ip_address, subnet)) {
        return false;
    }
    
    // Check if IP is excluded
    if (is_ip_excluded(ip_address, subnet)) {
        return false;
    }
    
    return true;
}

std::vector<std::shared_ptr<DhcpLease>> LeaseManager::get_active_leases() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::shared_ptr<DhcpLease>> active_leases;
    for (const auto& pair : leases_by_mac_) {
        if (pair.second->is_active) {
            active_leases.push_back(pair.second);
        }
    }
    
    return active_leases;
}

std::vector<std::shared_ptr<DhcpLease>> LeaseManager::get_leases_for_subnet(const std::string& subnet_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::shared_ptr<DhcpLease>> subnet_leases;
    for (const auto& pair : leases_by_mac_) {
        if (pair.second->is_active) {
            // Check if lease belongs to subnet (simplified)
            subnet_leases.push_back(pair.second);
        }
    }
    
    return subnet_leases;
}

DhcpStats LeaseManager::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    DhcpStats stats;
    stats.active_leases = 0;
    
    for (const auto& pair : leases_by_mac_) {
        if (pair.second->is_active) {
            stats.active_leases++;
        }
    }
    
    return stats;
}

void LeaseManager::set_lease_expiration_callback(std::function<void(const DhcpLease&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    expiration_callback_ = callback;
}

void LeaseManager::load_leases(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARN("Cannot open lease file: " + filename);
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse lease line (simplified format)
        std::istringstream iss(line);
        std::string mac_str, ip_str, hostname, start_str, end_str;
        
        if (iss >> mac_str >> ip_str >> hostname >> start_str >> end_str) {
            try {
                auto lease = std::make_shared<DhcpLease>();
                lease->mac_address = string_to_mac(mac_str);
                lease->ip_address = string_to_ip(ip_str);
                lease->hostname = hostname;
                lease->lease_start = std::chrono::system_clock::from_time_t(std::stoll(start_str));
                lease->lease_end = std::chrono::system_clock::from_time_t(std::stoll(end_str));
                lease->is_static = false;
                lease->is_active = true;
                
                add_lease(lease);
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to parse lease line: " + line + " - " + e.what());
            }
        }
    }
    
    file.close();
    LOG_INFO("Loaded leases from: " + filename);
}

void LeaseManager::save_leases(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw LeaseManagerException("Cannot open lease file for writing: " + filename);
    }
    
    file << "# DHCP Lease Database\n";
    file << "# Format: MAC IP HOSTNAME START_TIME END_TIME\n";
    
    for (const auto& pair : leases_by_mac_) {
        const auto& lease = pair.second;
        if (lease->is_active) {
            auto start_time = std::chrono::system_clock::to_time_t(lease->lease_start);
            auto end_time = std::chrono::system_clock::to_time_t(lease->lease_end);
            
            file << mac_to_string(lease->mac_address) << " "
                 << ip_to_string(lease->ip_address) << " "
                 << lease->hostname << " "
                 << start_time << " "
                 << end_time << "\n";
        }
    }
    
    file.close();
    LOG_INFO("Saved leases to: " + filename);
}

void LeaseManager::cleanup_expired_leases() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(60)); // Check every minute
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = get_current_time();
        std::vector<std::shared_ptr<DhcpLease>> expired_leases;
        
        for (const auto& pair : leases_by_mac_) {
            const auto& lease = pair.second;
            if (lease->is_active && now > lease->lease_end) {
                expired_leases.push_back(lease);
            }
        }
        
        for (const auto& lease : expired_leases) {
            lease->is_active = false;
            remove_lease(lease);
            
            if (expiration_callback_) {
                expiration_callback_(*lease);
            }
            
            LOG_INFO("Expired lease: " + mac_to_string(lease->mac_address) + " -> " + ip_to_string(lease->ip_address));
        }
    }
}

IpAddress LeaseManager::find_available_ip(const DhcpSubnet& subnet) {
    for (uint32_t ip = ntohl(subnet.range_start); ip <= ntohl(subnet.range_end); ++ip) {
        IpAddress network_ip = htonl(ip);
        
        // Check if IP is already leased
        if (leases_by_ip_.find(network_ip) != leases_by_ip_.end()) {
            continue;
        }
        
        // Check if IP is excluded
        if (is_ip_excluded(network_ip, subnet)) {
            continue;
        }
        
        return network_ip;
    }
    
    throw LeaseManagerException("No available IP addresses in subnet: " + subnet.name);
}

bool LeaseManager::is_ip_in_range(IpAddress ip, const DhcpSubnet& subnet) {
    return ntohl(ip) >= ntohl(subnet.range_start) && ntohl(ip) <= ntohl(subnet.range_end);
}

bool LeaseManager::is_ip_excluded(IpAddress ip, const DhcpSubnet& subnet) {
    for (const auto& exclusion : subnet.exclusions) {
        if (ntohl(ip) >= ntohl(exclusion.first) && ntohl(ip) <= ntohl(exclusion.second)) {
            return true;
        }
    }
    return false;
}

const DhcpSubnet& LeaseManager::get_subnet_by_name(const std::string& name) {
    for (const auto& subnet : config_.subnets) {
        if (subnet.name == name) {
            return subnet;
        }
    }
    throw LeaseManagerException("Subnet not found: " + name);
}

void LeaseManager::add_lease(std::shared_ptr<DhcpLease> lease) {
    leases_by_mac_[lease->mac_address] = lease;
    leases_by_ip_[lease->ip_address] = lease;
    update_statistics(*lease);
}

void LeaseManager::remove_lease(std::shared_ptr<DhcpLease> lease) {
    leases_by_mac_.erase(lease->mac_address);
    leases_by_ip_.erase(lease->ip_address);
}

void LeaseManager::update_statistics(const DhcpLease& lease) {
    // Update statistics (simplified)
    // This would be implemented based on specific requirements
}

} // namespace simple_dhcpd
