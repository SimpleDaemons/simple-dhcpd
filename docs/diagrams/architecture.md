# Simple DHCP Daemon - Architecture Diagrams

## System Architecture

```mermaid
graph TB
    subgraph "Application Layer"
        Main[main.cpp]
        Server[DhcpServer]
    end

    subgraph "Core Layer"
        Parser[DhcpParser<br/>Message Parsing/Generation]
        Types[DhcpTypes<br/>Data Structures]
    end

    subgraph "Network Layer"
        Socket[UdpSocket<br/>UDP Communication]
    end

    subgraph "Lease Management Layer"
        LeaseMgr[LeaseManager<br/>Basic Lease Operations]
        AdvLeaseMgr[AdvancedLeaseManager<br/>Conflict Resolution]
    end

    subgraph "Options Layer"
        OptionsMgr[DhcpOptionsManager<br/>Options Handling]
    end

    subgraph "Security Layer"
        SecurityMgr[DhcpSecurityManager<br/>Snooping/Filtering/Rate Limiting]
    end

    subgraph "Configuration Layer"
        ConfigMgr[ConfigManager<br/>Multi-format Config]
    end

    subgraph "Utilities Layer"
        Logger[Logger<br/>Logging System]
    end

    Main --> Server
    Server --> Parser
    Server --> Socket
    Server --> LeaseMgr
    Server --> OptionsMgr
    Server --> SecurityMgr
    Server --> ConfigMgr
    Server --> Logger

    LeaseMgr --> AdvLeaseMgr
    Parser --> Types
    Socket --> Types
    LeaseMgr --> Types
    OptionsMgr --> Types
```

## DORA Process Flow

```mermaid
sequenceDiagram
    participant Client
    participant Server
    participant LeaseMgr
    participant SecurityMgr
    participant OptionsMgr

    Client->>Server: DHCP Discover (Broadcast)
    Server->>SecurityMgr: Validate Request
    SecurityMgr-->>Server: Allowed/Denied
    Server->>LeaseMgr: Allocate IP Address
    LeaseMgr-->>Server: Lease Information
    Server->>OptionsMgr: Build Options
    OptionsMgr-->>Server: DHCP Options
    Server->>Client: DHCP Offer (Unicast/Broadcast)

    Client->>Server: DHCP Request (Broadcast)
    Server->>SecurityMgr: Validate Request
    Server->>LeaseMgr: Confirm Lease
    LeaseMgr-->>Server: Lease Confirmed
    Server->>OptionsMgr: Build Options
    Server->>Client: DHCP ACK (Unicast/Broadcast)

    Note over Client,Server: Client now has IP configuration

    Client->>Server: DHCP Release (Optional)
    Server->>LeaseMgr: Release Lease
    LeaseMgr-->>Server: Lease Released
```

## Component Interaction

```mermaid
graph LR
    subgraph "Message Handling"
        A[UDP Socket] -->|Raw Data| B[DhcpParser]
        B -->|Parsed Message| C[DhcpServer]
        C -->|Response| B
        B -->|Serialized| A
    end

    subgraph "Lease Management"
        C -->|Allocate| D[LeaseManager]
        C -->|Renew| D
        C -->|Release| D
        D -->|Conflict Check| E[AdvancedLeaseManager]
    end

    subgraph "Security"
        C -->|Validate| F[SecurityManager]
        F -->|Snooping| G[Trusted Interfaces]
        F -->|Filtering| H[MAC/IP Filters]
        F -->|Rate Limit| I[Rate Limiter]
    end

    subgraph "Configuration"
        C -->|Load| J[ConfigManager]
        J -->|Subnets| C
        J -->|Options| K[OptionsManager]
    end
```

## State Machine

```mermaid
stateDiagram-v2
    [*] --> Initializing: Server Start
    Initializing --> Running: Initialization Complete
    Running --> Processing: Receive Message
    Processing --> Validating: Parse Message
    Validating --> Allocating: Security Check Pass
    Validating --> Rejecting: Security Check Fail
    Allocating --> Responding: Lease Allocated
    Responding --> Running: Response Sent
    Rejecting --> Running: Rejection Sent
    Running --> Reloading: SIGHUP Received
    Reloading --> Running: Config Reloaded
    Running --> Stopping: SIGTERM/SIGINT
    Stopping --> [*]: Shutdown Complete
```

## Layered Architecture Detail

```mermaid
graph TB
    subgraph "Client Layer"
        Client1[DHCP Client 1]
        Client2[DHCP Client 2]
        ClientN[DHCP Client N]
    end

    subgraph "Network Interface Layer"
        Eth0[eth0<br/>Primary Interface]
        Eth1[eth1<br/>Secondary Interface]
        Vlan[VLAN Interfaces]
    end

    subgraph "Network Stack"
        UDP[UDP Socket<br/>Port 67]
        IPv4[IPv4 Stack]
        IPv6[IPv6 Stack]
    end

    subgraph "Application Core"
        Server[DhcpServer<br/>Main Server Loop]
        Parser[DhcpParser<br/>Message Parsing]
        Handler[MessageHandler<br/>Request Routing]
    end

    subgraph "Business Logic Layer"
        LeaseMgr[LeaseManager<br/>IP Allocation]
        OptionsMgr[OptionsManager<br/>DHCP Options]
        SecurityMgr[SecurityManager<br/>Access Control]
    end

    subgraph "Data Layer"
        LeaseDB[(Lease Database<br/>SQLite/File)]
        ConfigDB[(Configuration<br/>JSON/YAML/INI)]
        StatsDB[(Statistics<br/>Metrics)]
    end

    subgraph "Infrastructure Layer"
        Logger[Logger<br/>Structured Logging]
        Metrics[Metrics<br/>Prometheus/Stats]
        Signals[SignalHandler<br/>SIGHUP/SIGTERM]
    end

    Client1 --> Eth0
    Client2 --> Eth1
    ClientN --> Vlan

    Eth0 --> UDP
    Eth1 --> UDP
    Vlan --> UDP

    UDP --> IPv4
    UDP --> IPv6
    IPv4 --> Server
    IPv6 --> Server

    Server --> Parser
    Parser --> Handler
    Handler --> LeaseMgr
    Handler --> OptionsMgr
    Handler --> SecurityMgr

    LeaseMgr --> LeaseDB
    OptionsMgr --> ConfigDB
    SecurityMgr --> ConfigDB
    Server --> StatsDB

    Server --> Logger
    Server --> Metrics
    Server --> Signals
```

## Module Dependencies

```mermaid
graph TD
    Main[main.cpp] --> Server[DhcpServer]

    Server --> Parser[DhcpParser]
    Server --> Socket[UdpSocket]
    Server --> LeaseMgr[LeaseManager]
    Server --> OptionsMgr[DhcpOptionsManager]
    Server --> SecurityMgr[DhcpSecurityManager]
    Server --> ConfigMgr[ConfigManager]
    Server --> Logger[Logger]

    Parser --> Types[DhcpTypes]
    Socket --> Types
    LeaseMgr --> Types
    OptionsMgr --> Types

    LeaseMgr --> AdvLeaseMgr[AdvancedLeaseManager]
    AdvLeaseMgr --> Types

    SecurityMgr --> Types
    SecurityMgr --> Logger

    ConfigMgr --> Logger
    OptionsMgr --> ConfigMgr

    LeaseMgr --> ConfigMgr
    Server --> ConfigMgr
```

## Threading Model

```mermaid
graph TB
    subgraph "Main Thread"
        MainThread[Main Thread<br/>Signal Handling<br/>Configuration Management]
    end

    subgraph "Network I/O Threads"
        IOThread1[I/O Thread 1<br/>UDP Socket 1]
        IOThread2[I/O Thread 2<br/>UDP Socket 2]
        IOThreadN[I/O Thread N<br/>UDP Socket N]
    end

    subgraph "Worker Thread Pool"
        Worker1[Worker Thread 1<br/>Message Processing]
        Worker2[Worker Thread 2<br/>Message Processing]
        WorkerN[Worker Thread N<br/>Message Processing]
    end

    subgraph "Background Threads"
        LeaseCleanup[Lease Cleanup Thread<br/>Expired Lease Removal]
        StatsThread[Statistics Thread<br/>Metrics Collection]
        ConfigWatcher[Config Watcher Thread<br/>File System Monitoring]
    end

    MainThread --> IOThread1
    MainThread --> IOThread2
    MainThread --> IOThreadN

    IOThread1 --> Worker1
    IOThread1 --> Worker2
    IOThread2 --> Worker1
    IOThread2 --> Worker2
    IOThreadN --> WorkerN

    MainThread --> LeaseCleanup
    MainThread --> StatsThread
    MainThread --> ConfigWatcher

    Worker1 --> LeaseCleanup
    Worker2 --> LeaseCleanup
    WorkerN --> LeaseCleanup
```
