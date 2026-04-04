/**
 * @file lease/manager.hpp
 * @brief DHCP lease management
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_LEASE_MANAGER_HPP
#define SIMPLE_DHCPD_LEASE_MANAGER_HPP

#include "simple-dhcpd/core/types.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

namespace simple_dhcpd {

/**
 * @brief Lease manager exception
 */
class LeaseManagerException : public std::exception {
public:
    explicit LeaseManagerException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

/**
 * @brief DHCP lease manager class
 */
class LeaseManager {
public:
    /**
     * @brief Constructor
     * @param config DHCP configuration
     */
    explicit LeaseManager(const DhcpConfig& config);
    
    /**
     * @brief Destructor
     */
    ~LeaseManager();
    
    /**
     * @brief Start lease manager
     */
    void start();
    
    /**
     * @brief Stop lease manager
     */
    void stop();
    
    /**
     * @brief Allocate a new lease
     * @param mac_address Client MAC address
     * @param requested_ip Requested IP address (0 for any)
     * @param subnet_name Subnet name
     * @return Allocated lease
     * @throws LeaseManagerException if allocation fails
     */
    DhcpLease allocate_lease(const MacAddress& mac_address, IpAddress requested_ip, const std::string& subnet_name);
    
    /**
     * @brief Renew an existing lease
     * @param mac_address Client MAC address
     * @param ip_address IP address to renew
     * @return Renewed lease
     * @throws LeaseManagerException if renewal fails
     */
    DhcpLease renew_lease(const MacAddress& mac_address, IpAddress ip_address);
    
    /**
     * @brief Release a lease
     * @param mac_address Client MAC address
     * @param ip_address IP address to release
     * @return true if lease was released
     */
    bool release_lease(const MacAddress& mac_address, IpAddress ip_address);
    
    /**
     * @brief Get lease by MAC address
     * @param mac_address Client MAC address
     * @return Lease if found, nullptr otherwise
     */
    std::shared_ptr<DhcpLease> get_lease_by_mac(const MacAddress& mac_address);
    
    /**
     * @brief Get lease by IP address
     * @param ip_address IP address
     * @return Lease if found, nullptr otherwise
     */
    std::shared_ptr<DhcpLease> get_lease_by_ip(IpAddress ip_address);
    
    /**
     * @brief Check if IP address is available
     * @param ip_address IP address to check
     * @param subnet_name Subnet name
     * @return true if IP is available
     */
    bool is_ip_available(IpAddress ip_address, const std::string& subnet_name);
    
    /**
     * @brief Get all active leases
     * @return Vector of active leases
     */
    std::vector<std::shared_ptr<DhcpLease>> get_active_leases() const;
    
    /**
     * @brief Get leases for subnet
     * @param subnet_name Subnet name
     * @return Vector of leases for subnet
     */
    std::vector<std::shared_ptr<DhcpLease>> get_leases_for_subnet(const std::string& subnet_name);
    
    /**
     * @brief Get lease statistics
     * @return Lease statistics
     */
    DhcpStats get_statistics() const;
    
    /**
     * @brief Set lease expiration callback
     * @param callback Callback function for lease expiration
     */
    void set_lease_expiration_callback(std::function<void(const DhcpLease&)> callback);
    
    /**
     * @brief Load leases from file
     * @param filename Lease file path
     * @throws LeaseManagerException if loading fails
     */
    void load_leases(const std::string& filename);
    
    /**
     * @brief Save leases to file
     * @param filename Lease file path
     * @throws LeaseManagerException if saving fails
     */
    void save_leases(const std::string& filename);

protected:
    DhcpConfig config_;
    std::map<MacAddress, std::shared_ptr<DhcpLease>> leases_by_mac_;
    std::map<IpAddress, std::shared_ptr<DhcpLease>> leases_by_ip_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_;
    std::thread cleanup_thread_;
    std::function<void(const DhcpLease&)> expiration_callback_;
    
    /**
     * @brief Cleanup expired leases
     */
    void cleanup_expired_leases();
    
    /**
     * @brief Find available IP in subnet
     * @param subnet Subnet configuration
     * @return Available IP address
     * @throws LeaseManagerException if no IP available
     */
    IpAddress find_available_ip(const DhcpSubnet& subnet);
    
    /**
     * @brief Check if IP is in range
     * @param ip IP address to check
     * @param subnet Subnet configuration
     * @return true if IP is in range
     */
    bool is_ip_in_range(IpAddress ip, const DhcpSubnet& subnet);
    
    /**
     * @brief Check if IP is excluded
     * @param ip IP address to check
     * @param subnet Subnet configuration
     * @return true if IP is excluded
     */
    bool is_ip_excluded(IpAddress ip, const DhcpSubnet& subnet);
    
    /**
     * @brief Get subnet by name
     * @param name Subnet name
     * @return Subnet configuration
     * @throws LeaseManagerException if subnet not found
     */
    const DhcpSubnet& get_subnet_by_name(const std::string& name);
    
    /**
     * @brief Add lease to internal structures
     * @param lease Lease to add
     */
    void add_lease(std::shared_ptr<DhcpLease> lease);
    
    /**
     * @brief Remove lease from internal structures
     * @param lease Lease to remove
     */
    void remove_lease(std::shared_ptr<DhcpLease> lease);
    
    /**
     * @brief Update lease statistics
     * @param lease Lease that was updated
     */
    void update_statistics(const DhcpLease& lease);
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_LEASE_MANAGER_HPP
