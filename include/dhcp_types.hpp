/**
 * @file dhcp_types.hpp
 * @brief DHCP protocol types and structures
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_DHCP_TYPES_HPP
#define SIMPLE_DHCPD_DHCP_TYPES_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <array>

namespace simple_dhcpd {

/**
 * @brief DHCP message types
 */
enum class DhcpMessageType : uint8_t {
    DISCOVER = 1,
    OFFER = 2,
    REQUEST = 3,
    DECLINE = 4,
    ACK = 5,
    NAK = 6,
    RELEASE = 7,
    INFORM = 8
};

/**
 * @brief DHCP option codes
 */
enum class DhcpOptionCode : uint8_t {
    PAD = 0,
    SUBNET_MASK = 1,
    TIME_OFFSET = 2,
    ROUTER = 3,
    TIME_SERVER = 4,
    NAME_SERVER = 5,
    DOMAIN_SERVER = 6,
    LOG_SERVER = 7,
    COOKIE_SERVER = 8,
    LPR_SERVER = 9,
    IMPRESS_SERVER = 10,
    RESOURCE_LOCATION_SERVER = 11,
    HOST_NAME = 12,
    BOOT_FILE_SIZE = 13,
    MERIT_DUMP_FILE = 14,
    DOMAIN_NAME = 15,
    SWAP_SERVER = 16,
    ROOT_PATH = 17,
    EXTENSIONS_PATH = 18,
    IP_FORWARDING = 19,
    NON_LOCAL_SOURCE_ROUTING = 20,
    POLICY_FILTER = 21,
    MAX_DGRAM_REASSEMBLY = 22,
    DEFAULT_IP_TTL = 23,
    PATH_MTU_AGING_TIMEOUT = 24,
    PATH_MTU_PLATEAU_TABLE = 25,
    INTERFACE_MTU = 26,
    ALL_SUBNETS_LOCAL = 27,
    BROADCAST_ADDRESS = 28,
    PERFORM_MASK_DISCOVERY = 29,
    MASK_SUPPLIER = 30,
    ROUTER_DISCOVERY = 31,
    ROUTER_SOLICITATION_ADDRESS = 32,
    STATIC_ROUTE = 33,
    TRAILER_ENCAPSULATION = 34,
    ARP_CACHE_TIMEOUT = 35,
    ETHERNET_ENCAPSULATION = 36,
    TCP_DEFAULT_TTL = 37,
    TCP_KEEPALIVE_INTERVAL = 38,
    TCP_KEEPALIVE_GARBAGE = 39,
    NIS_DOMAIN = 40,
    NIS_SERVERS = 41,
    NTP_SERVERS = 42,
    VENDOR_SPECIFIC = 43,
    NETBIOS_NAME_SERVERS = 44,
    NETBIOS_DISTRIBUTION = 45,
    NETBIOS_NODE_TYPE = 46,
    NETBIOS_SCOPE = 47,
    FONT_SERVERS = 48,
    X_DISPLAY_MANAGER = 49,
    REQUESTED_IP_ADDRESS = 50,
    IP_ADDRESS_LEASE_TIME = 51,
    OPTION_OVERLOAD = 52,
    DHCP_MESSAGE_TYPE = 53,
    SERVER_IDENTIFIER = 54,
    PARAMETER_REQUEST_LIST = 55,
    MESSAGE = 56,
    MAX_DHCP_MESSAGE_SIZE = 57,
    RENEWAL_TIME = 58,
    REBINDING_TIME = 59,
    VENDOR_CLASS_IDENTIFIER = 60,
    CLIENT_IDENTIFIER = 61,
    NETWARE_IP_DOMAIN = 62,
    NETWARE_IP_OPTION = 63,
    NIS_PLUS_DOMAIN = 64,
    NIS_PLUS_SERVERS = 65,
    TFTP_SERVER_NAME = 66,
    BOOTFILE_NAME = 67,
    MOBILE_IP_HOME_AGENT = 68,
    SMTP_SERVER = 69,
    POP3_SERVER = 70,
    NNTP_SERVER = 71,
    WWW_SERVER = 72,
    FINGER_SERVER = 73,
    IRC_SERVER = 74,
    STREETTALK_SERVER = 75,
    STDA_SERVER = 76,
    USER_CLASS = 77,
    DIRECTORY_AGENT = 78,
    SERVICE_SCOPE = 79,
    RAPID_COMMIT = 80,
    CLIENT_FQDN = 81,
    RELAY_AGENT_INFORMATION = 82,
    ISCSI_INITIATOR_NAME = 83,
    NDS_SERVERS = 85,
    NDS_TREE_NAME = 86,
    NDS_CONTEXT = 87,
    BCMCS_CONTROLLER_DOMAIN_NAME_LIST = 88,
    BCMCS_CONTROLLER_IPV4_ADDRESS = 89,
    AUTHENTICATION = 90,
    LAST_TRANSACTION_TIME = 91,
    ASSOCIATED_IP = 92,
    CLIENT_SYSTEM = 93,
    CLIENT_NDI = 94,
    LDAP = 95,
    UUID_GUID = 97,
    USER_AUTH = 98,
    GEOCONF_CIVIC = 99,
    PCODE = 100,
    TCODE = 101,
    NETINFO_ADDRESS = 112,
    NETINFO_TAG = 113,
    URL = 114,
    AUTO_CONFIG = 116,
    NAME_SERVICE_SEARCH = 117,
    SUBNET_SELECTION = 118,
    DOMAIN_SEARCH = 119,
    SIP_SERVERS = 120,
    CLASSLESS_STATIC_ROUTE = 121,
    CCC = 122,
    GEOCONF = 123,
    V_I_VENDOR_CLASS = 124,
    V_I_VENDOR_SPECIFIC = 125,
    PXE_LEGACY = 128,
    PXE_CLIENT = 128,
    PXE_UNDI = 128,
    PXE_BC = 128,
    PXE_BS = 128,
    PXE_BD = 128,
    PXE_BE = 128,
    PXE_BF = 128,
    PXE_BG = 128,
    PXE_BH = 128,
    PXE_BI = 128,
    PXE_BJ = 128,
    PXE_BK = 128,
    PXE_BL = 128,
    PXE_BM = 128,
    PXE_BN = 128,
    PXE_BO = 128,
    PXE_BP = 128,
    PXE_BQ = 128,
    PXE_BR = 128,
    PXE_BT = 128,
    PXE_BU = 128,
    PXE_BV = 128,
    PXE_BW = 128,
    PXE_BX = 128,
    PXE_BY = 128,
    PXE_BZ = 128,
    PXE_CA = 128,
    PXE_CB = 128,
    PXE_CC = 128,
    PXE_CD = 128,
    PXE_CE = 128,
    PXE_CF = 128,
    PXE_CG = 128,
    PXE_CH = 128,
    PXE_CI = 128,
    PXE_CJ = 128,
    PXE_CK = 128,
    PXE_CL = 128,
    PXE_CM = 128,
    PXE_CN = 128,
    PXE_CO = 128,
    PXE_CP = 128,
    PXE_CQ = 128,
    PXE_CR = 128,
    PXE_CS = 128,
    PXE_CT = 128,
    PXE_CU = 128,
    PXE_CV = 128,
    PXE_CW = 128,
    PXE_CX = 128,
    PXE_CY = 128,
    PXE_CZ = 128,
    PXE_DA = 128,
    PXE_DB = 128,
    PXE_DC = 128,
    PXE_DD = 128,
    PXE_DE = 128,
    PXE_DF = 128,
    PXE_DG = 128,
    PXE_DH = 128,
    PXE_DI = 128,
    PXE_DJ = 128,
    PXE_DK = 128,
    PXE_DL = 128,
    PXE_DM = 128,
    PXE_DN = 128,
    PXE_DO = 128,
    PXE_DP = 128,
    PXE_DQ = 128,
    PXE_DR = 128,
    PXE_DS = 128,
    PXE_DT = 128,
    PXE_DU = 128,
    PXE_DV = 128,
    PXE_DW = 128,
    PXE_DX = 128,
    PXE_DY = 128,
    PXE_DZ = 128,
    PXE_EA = 128,
    PXE_EB = 128,
    PXE_EC = 128,
    PXE_ED = 128,
    PXE_EE = 128,
    PXE_EF = 128,
    PXE_EG = 128,
    PXE_EH = 128,
    PXE_EI = 128,
    PXE_EJ = 128,
    PXE_EK = 128,
    PXE_EL = 128,
    PXE_EM = 128,
    PXE_EN = 128,
    PXE_EO = 128,
    PXE_EP = 128,
    PXE_EQ = 128,
    PXE_ER = 128,
    PXE_ES = 128,
    PXE_ET = 128,
    PXE_EU = 128,
    PXE_EV = 128,
    PXE_EW = 128,
    PXE_EX = 128,
    PXE_EY = 128,
    PXE_EZ = 128,
    PXE_FA = 128,
    PXE_FB = 128,
    PXE_FC = 128,
    PXE_FD = 128,
    PXE_FE = 128,
    PXE_FF = 128,
    END = 255
};

/**
 * @brief DHCP message header structure
 */
struct DhcpMessageHeader {
    uint8_t op;         // Message op code / message type
    uint8_t htype;      // Hardware address type
    uint8_t hlen;       // Hardware address length
    uint8_t hops;       // Client sets to zero, optionally used by relay agents
    uint32_t xid;       // Transaction ID
    uint16_t secs;      // Seconds elapsed since client began address acquisition
    uint16_t flags;     // Flags
    uint32_t ciaddr;    // Client IP address
    uint32_t yiaddr;    // 'your' (client) IP address
    uint32_t siaddr;    // IP address of next server to use in bootstrap
    uint32_t giaddr;    // Relay agent IP address
    uint8_t chaddr[16]; // Client hardware address
    uint8_t sname[64];  // Optional server host name
    uint8_t file[128];  // Boot file name
    uint8_t options[312]; // Optional parameters field
} __attribute__((packed));

/**
 * @brief DHCP option structure
 */
struct DhcpOption {
    DhcpOptionCode code;
    uint8_t length;
    std::vector<uint8_t> data;
    
    DhcpOption() : code(DhcpOptionCode::PAD), length(0) {}
    DhcpOption(DhcpOptionCode c, const std::vector<uint8_t>& d) 
        : code(c), length(static_cast<uint8_t>(d.size())), data(d) {}
};

/**
 * @brief MAC address type
 */
using MacAddress = std::array<uint8_t, 6>;

/**
 * @brief IP address type
 */
using IpAddress = uint32_t;

/**
 * @brief DHCP lease information
 */
/**
 * @brief Lease type enumeration
 */
enum class LeaseType {
    DYNAMIC,
    STATIC
};

struct DhcpLease {
    MacAddress mac_address;
    IpAddress ip_address;
    std::string hostname;
    std::chrono::system_clock::time_point lease_start;
    std::chrono::system_clock::time_point lease_end;
    std::chrono::system_clock::time_point renewal_time;
    std::chrono::system_clock::time_point rebinding_time;
    std::chrono::system_clock::time_point allocated_at;
    std::chrono::system_clock::time_point expires_at;
    std::chrono::seconds lease_time;
    LeaseType lease_type;
    std::string client_id;
    std::map<DhcpOptionCode, std::vector<uint8_t>> options;
    bool is_static;
    bool is_active;
    
    DhcpLease() : ip_address(0), is_static(false), is_active(false) {}
    
    // Copy constructor
    DhcpLease(const DhcpLease& other) = default;
    
    // Copy assignment operator
    DhcpLease& operator=(const DhcpLease& other) = default;
    
    // Move constructor
    DhcpLease(DhcpLease&& other) = default;
    
    // Move assignment operator
    DhcpLease& operator=(DhcpLease&& other) = default;
};

/**
 * @brief DHCP subnet configuration
 */
struct DhcpSubnet {
    std::string name;
    IpAddress network;
    uint8_t prefix_length;
    IpAddress range_start;
    IpAddress range_end;
    IpAddress gateway;
    std::vector<IpAddress> dns_servers;
    std::string domain_name;
    uint32_t lease_time;
    uint32_t max_lease_time;
    std::vector<DhcpOption> options;
    std::map<MacAddress, DhcpLease> reservations;
    std::vector<std::pair<IpAddress, IpAddress>> exclusions;
    
    DhcpSubnet() : network(0), prefix_length(0), range_start(0), range_end(0), 
                   gateway(0), lease_time(86400), max_lease_time(172800) {}
    
    // Copy constructor
    DhcpSubnet(const DhcpSubnet& other) = default;
    
    // Copy assignment operator
    DhcpSubnet& operator=(const DhcpSubnet& other) = default;
    
    // Move constructor
    DhcpSubnet(DhcpSubnet&& other) = default;
    
    // Move assignment operator
    DhcpSubnet& operator=(DhcpSubnet&& other) = default;
};

/**
 * @brief DHCP server configuration
 */
struct DhcpConfig {
    std::vector<std::string> listen_addresses;
    std::vector<DhcpSubnet> subnets;
    std::vector<DhcpOption> global_options;
    std::string config_file;
    std::string lease_file;
    std::string log_file;
    bool enable_logging;
    bool enable_security;
    uint32_t max_leases;
    
    DhcpConfig() : enable_logging(true), enable_security(true), max_leases(10000) {}
    
    // Copy constructor
    DhcpConfig(const DhcpConfig& other) = default;
    
    // Copy assignment operator
    DhcpConfig& operator=(const DhcpConfig& other) = default;
    
    // Move constructor
    DhcpConfig(DhcpConfig&& other) = default;
    
    // Move assignment operator
    DhcpConfig& operator=(DhcpConfig&& other) = default;
};

/**
 * @brief DHCP message structure
 */
struct DhcpMessage {
    DhcpMessageHeader header;
    std::vector<DhcpOption> options;
    DhcpMessageType message_type;
    MacAddress client_mac;
    IpAddress client_ip;
    IpAddress server_ip;
    IpAddress relay_ip;
    
    DhcpMessage() : message_type(DhcpMessageType::DISCOVER), 
                    client_ip(0), server_ip(0), relay_ip(0) {}
};

/**
 * @brief DHCP statistics
 */
struct DhcpStats {
    uint64_t total_requests;
    uint64_t discover_count;
    uint64_t request_count;
    uint64_t release_count;
    uint64_t decline_count;
    uint64_t inform_count;
    uint64_t offer_count;
    uint64_t ack_count;
    uint64_t nak_count;
    uint64_t active_leases;
    uint64_t total_leases_created;
    uint64_t total_leases_expired;
    uint64_t total_errors;
    
    DhcpStats() : total_requests(0), discover_count(0), request_count(0),
                  release_count(0), decline_count(0), inform_count(0),
                  offer_count(0), ack_count(0), nak_count(0), active_leases(0),
                  total_leases_created(0), total_leases_expired(0), total_errors(0) {}
};

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_DHCP_TYPES_HPP
