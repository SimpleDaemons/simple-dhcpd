/**
 * @file lease/advanced_lease_manager.hpp
 * @brief Advanced DHCP lease management with static leases and conflict resolution
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_ADVANCED_LEASE_MANAGER_HPP
#define SIMPLE_DHCPD_ADVANCED_LEASE_MANAGER_HPP

#include "simple-dhcpd/lease/manager.hpp"
#include "simple-dhcpd/core/types.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <queue>
#include <set>

namespace simple_dhcpd {

/**
 * @brief Lease conflict resolution strategy
 */
enum class ConflictResolutionStrategy {
    REJECT,           // Reject the new lease request
    REPLACE,          // Replace the existing lease
    EXTEND,           // Extend the existing lease
    NEGOTIATE         // Negotiate with the client
};

/**
 * @brief Static lease configuration
 */
struct StaticLease {
    MacAddress mac_address;
    IpAddress ip_address;
    std::string hostname;
    std::string description;
    std::chrono::seconds lease_time;
    std::map<DhcpOptionCode, std::vector<uint8_t>> options;
    bool enabled;
    std::string vendor_class;
    
    StaticLease() = default;
    StaticLease(const MacAddress& mac, const IpAddress& ip, 
                const std::string& host = "", const std::string& desc = "",
                std::chrono::seconds lease = std::chrono::seconds(86400))
        : mac_address(mac), ip_address(ip), hostname(host), description(desc),
          lease_time(lease), enabled(true) {}
};

/**
 * @brief Lease conflict information
 */
struct LeaseConflict {
    MacAddress existing_mac;
    MacAddress conflicting_mac;
    IpAddress ip_address;
    std::chrono::system_clock::time_point conflict_time;
    ConflictResolutionStrategy resolution;
    std::string reason;
    
    LeaseConflict(const MacAddress& existing, const MacAddress& conflicting,
                  const IpAddress& ip, const std::string& reason = "")
        : existing_mac(existing), conflicting_mac(conflicting), ip_address(ip),
          conflict_time(std::chrono::system_clock::now()), 
          resolution(ConflictResolutionStrategy::REJECT), reason(reason) {}
};

/**
 * @brief Lease database statistics
 */
struct LeaseDatabaseStats {
    size_t total_leases;
    size_t active_leases;
    size_t expired_leases;
    size_t static_leases;
    size_t dynamic_leases;
    size_t conflicts_resolved;
    size_t database_size_bytes;
    std::chrono::system_clock::time_point last_cleanup;
    std::chrono::system_clock::time_point last_backup;
};

/**
 * @brief Advanced lease manager with static leases and conflict resolution
 */
class AdvancedLeaseManager : public LeaseManager {
public:
    /**
     * @brief Constructor
     * @param config DHCP configuration
     * @param database_path Path to lease database
     */
    explicit AdvancedLeaseManager(const DhcpConfig& config, const std::string& database_path = "");
    
    /**
     * @brief Destructor
     */
    ~AdvancedLeaseManager();
    
    // Static lease management
    /**
     * @brief Add static lease
     * @param static_lease Static lease configuration
     * @return true if added successfully
     */
    bool add_static_lease(const StaticLease& static_lease);
    
    /**
     * @brief Remove static lease
     * @param mac_address MAC address of static lease
     * @return true if removed successfully
     */
    bool remove_static_lease(const MacAddress& mac_address);
    
    /**
     * @brief Get static lease by MAC address
     * @param mac_address MAC address
     * @return Static lease if found, nullptr otherwise
     */
    std::shared_ptr<StaticLease> get_static_lease(const MacAddress& mac_address);
    
    /**
     * @brief Get all static leases
     * @return Vector of static leases
     */
    std::vector<std::shared_ptr<StaticLease>> get_all_static_leases();
    
    /**
     * @brief Update static lease
     * @param mac_address MAC address
     * @param static_lease Updated static lease
     * @return true if updated successfully
     */
    bool update_static_lease(const MacAddress& mac_address, const StaticLease& static_lease);
    
    // Conflict resolution
    /**
     * @brief Set conflict resolution strategy
     * @param strategy Conflict resolution strategy
     */
    void set_conflict_resolution_strategy(ConflictResolutionStrategy strategy);
    
    /**
     * @brief Get conflict resolution strategy
     * @return Current conflict resolution strategy
     */
    ConflictResolutionStrategy get_conflict_resolution_strategy() const;
    
    /**
     * @brief Resolve lease conflict
     * @param conflict Lease conflict to resolve
     * @return true if conflict was resolved
     */
    bool resolve_lease_conflict(const LeaseConflict& conflict);
    
    /**
     * @brief Get pending conflicts
     * @return Vector of pending conflicts
     */
    std::vector<LeaseConflict> get_pending_conflicts();
    
    /**
     * @brief Set conflict resolution callback
     * @param callback Callback function for conflict resolution
     */
    void set_conflict_callback(std::function<void(const LeaseConflict&)> callback);
    
    // Enhanced lease allocation
    /**
     * @brief Allocate lease with conflict resolution
     * @param mac_address Client MAC address
     * @param requested_ip Requested IP address (0 for any)
     * @param subnet_name Subnet name
     * @param client_id Client identifier
     * @return Allocated lease
     * @throws LeaseManagerException if allocation fails
     */
    DhcpLease allocate_lease_advanced(const MacAddress& mac_address, IpAddress requested_ip, 
                                     const std::string& subnet_name, const std::string& client_id = "");
    
    /**
     * @brief Renew lease with conflict checking
     * @param mac_address Client MAC address
     * @param ip_address IP address to renew
     * @param client_id Client identifier
     * @return Renewed lease
     * @throws LeaseManagerException if renewal fails
     */
    DhcpLease renew_lease_advanced(const MacAddress& mac_address, IpAddress ip_address, 
                                  const std::string& client_id = "");
    
    // Database management
    /**
     * @brief Load leases from database
     * @throws LeaseManagerException if loading fails
     */
    void load_database();
    
    /**
     * @brief Save leases to database
     * @throws LeaseManagerException if saving fails
     */
    void save_database();
    
    /**
     * @brief Backup database
     * @param backup_path Backup file path
     * @return true if backup successful
     */
    bool backup_database(const std::string& backup_path);
    
    /**
     * @brief Restore database from backup
     * @param backup_path Backup file path
     * @return true if restore successful
     */
    bool restore_database(const std::string& backup_path);
    
    /**
     * @brief Compact database
     * @return true if compaction successful
     */
    bool compact_database();
    
    /**
     * @brief Get database statistics
     * @return Database statistics
     */
    LeaseDatabaseStats get_database_statistics() const;
    
    // Lease monitoring and analytics
    /**
     * @brief Get lease utilization by subnet
     * @return Map of subnet name to utilization percentage
     */
    std::map<std::string, double> get_subnet_utilization();
    
    /**
     * @brief Get lease history for IP address
     * @param ip_address IP address
     * @return Vector of lease history entries
     */
    std::vector<DhcpLease> get_lease_history(IpAddress ip_address);
    
    /**
     * @brief Get leases expiring soon
     * @param time_window Time window in seconds
     * @return Vector of leases expiring soon
     */
    std::vector<std::shared_ptr<DhcpLease>> get_leases_expiring_soon(std::chrono::seconds time_window);
    
    /**
     * @brief Get lease conflicts in time range
     * @param start_time Start time
     * @param end_time End time
     * @return Vector of conflicts in time range
     */
    std::vector<LeaseConflict> get_conflicts_in_range(
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time);
    
    // Configuration management
    /**
     * @brief Set database auto-save interval
     * @param interval Auto-save interval in seconds
     */
    void set_auto_save_interval(std::chrono::seconds interval);
    
    /**
     * @brief Set lease cleanup interval
     * @param interval Cleanup interval in seconds
     */
    void set_cleanup_interval(std::chrono::seconds interval);
    
    /**
     * @brief Enable/disable conflict detection
     * @param enabled Enable conflict detection
     */
    void set_conflict_detection_enabled(bool enabled);

private:
    std::string database_path_;
    std::map<MacAddress, std::shared_ptr<StaticLease>> static_leases_;
    std::queue<LeaseConflict> pending_conflicts_;
    std::vector<LeaseConflict> conflict_history_;
    std::map<IpAddress, std::vector<DhcpLease>> lease_history_;
    mutable std::mutex static_leases_mutex_;
    mutable std::mutex conflicts_mutex_;
    mutable std::mutex database_mutex_;
    
    ConflictResolutionStrategy conflict_strategy_;
    std::function<void(const LeaseConflict&)> conflict_callback_;
    std::chrono::seconds auto_save_interval_;
    std::chrono::seconds cleanup_interval_;
    std::atomic<bool> conflict_detection_enabled_;
    std::atomic<bool> auto_save_enabled_;
    
    std::thread auto_save_thread_;
    std::thread cleanup_thread_;
    std::thread conflict_resolution_thread_;
    
    /**
     * @brief Auto-save thread function
     */
    void auto_save_worker();
    
    /**
     * @brief Enhanced cleanup thread function
     */
    void enhanced_cleanup_worker();
    
    /**
     * @brief Conflict resolution thread function
     */
    void conflict_resolution_worker();
    
    /**
     * @brief Detect lease conflicts
     * @param lease Lease to check for conflicts
     * @return true if conflicts detected
     */
    bool detect_conflicts(const DhcpLease& lease);
    
    /**
     * @brief Find best available IP for allocation
     * @param subnet Subnet configuration
     * @param preferred_ip Preferred IP address
     * @return Best available IP address
     */
    IpAddress find_best_available_ip(const DhcpSubnet& subnet, IpAddress preferred_ip = 0);
    
    /**
     * @brief Calculate IP utilization for subnet
     * @param subnet Subnet configuration
     * @return Utilization percentage
     */
    double calculate_subnet_utilization(const DhcpSubnet& subnet);
    
    /**
     * @brief Add lease to history
     * @param lease Lease to add to history
     */
    void add_to_history(const DhcpLease& lease);
    
    /**
     * @brief Serialize lease to database format
     * @param lease Lease to serialize
     * @return Serialized lease data
     */
    std::string serialize_lease(const DhcpLease& lease);
    
    /**
     * @brief Deserialize lease from database format
     * @param data Serialized lease data
     * @return Deserialized lease
     */
    DhcpLease deserialize_lease(const std::string& data);
    
    /**
     * @brief Serialize static lease to database format
     * @param static_lease Static lease to serialize
     * @return Serialized static lease data
     */
    std::string serialize_static_lease(const StaticLease& static_lease);
    
    /**
     * @brief Deserialize static lease from database format
     * @param data Serialized static lease data
     * @return Deserialized static lease
     */
    StaticLease deserialize_static_lease(const std::string& data);
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_ADVANCED_LEASE_MANAGER_HPP
