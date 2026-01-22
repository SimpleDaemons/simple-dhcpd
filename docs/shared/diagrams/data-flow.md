# Simple DHCP Daemon - Data Flow Diagrams

## DHCP Message Data Flow

```mermaid
flowchart LR
    subgraph "Client"
        C1[DHCP Client]
    end

    subgraph "Network"
        N1[UDP Packet<br/>Port 67]
    end

    subgraph "Server Input"
        S1[UDP Socket<br/>Receive]
        S2[Raw Bytes]
    end

    subgraph "Parsing"
        P1[DhcpParser<br/>Deserialize]
        P2[Parsed Message<br/>DhcpMessage]
    end

    subgraph "Processing"
        PR1[Message Handler]
        PR2[Security Check]
        PR3[Lease Management]
        PR4[Options Building]
    end

    subgraph "Response Generation"
        R1[Response Builder]
        R2[Serialized Message]
    end

    subgraph "Server Output"
        O1[UDP Socket<br/>Send]
        O2[UDP Packet<br/>Port 67]
    end

    C1 -->|DHCP Discover| N1
    N1 --> S1
    S1 --> S2
    S2 --> P1
    P1 --> P2
    P2 --> PR1
    PR1 --> PR2
    PR2 --> PR3
    PR3 --> PR4
    PR4 --> R1
    R1 --> R2
    R2 --> O1
    O1 --> O2
    O2 -->|DHCP Offer| C1
```

## Lease Data Flow

```mermaid
flowchart TB
    subgraph "Lease Request"
        LR1[Client Request]
        LR2[Extract MAC/IP]
    end

    subgraph "Lease Lookup"
        LL1[Check Static Lease]
        LL2[Check Dynamic Pool]
        LL3[Check Expired Leases]
    end

    subgraph "Lease Allocation"
        LA1[Select IP Address]
        LA2[Conflict Detection]
        LA3[Create Lease Record]
    end

    subgraph "Lease Storage"
        LS1[In-Memory Cache]
        LS2[Persistent Database]
        LS3[Lease Metadata]
    end

    subgraph "Lease Response"
        LRES1[Build Lease Info]
        LRES2[Include in DHCP Response]
    end

    LR1 --> LR2
    LR2 --> LL1
    LL1 -->|Found| LA3
    LL1 -->|Not Found| LL2
    LL2 -->|Available| LA1
    LL2 -->|Full| LL3
    LL3 -->|Expired Found| LA1
    LL3 -->|None Available| LRES1

    LA1 --> LA2
    LA2 -->|No Conflict| LA3
    LA2 -->|Conflict| LL2

    LA3 --> LS1
    LA3 --> LS2
    LS1 --> LS3
    LS2 --> LS3

    LS3 --> LRES1
    LRES1 --> LRES2
```

## Configuration Data Flow

```mermaid
flowchart TB
    subgraph "Configuration Sources"
        CS1[Config File<br/>JSON/YAML/INI]
        CS2[Command Line Args]
        CS3[Environment Variables]
        CS4[Runtime API]
    end

    subgraph "Configuration Loading"
        CL1[ConfigManager<br/>Load & Parse]
        CL2[Validation]
        CL3[Merge Sources]
    end

    subgraph "Configuration Storage"
        CS5[In-Memory Config]
        CS6[Config Cache]
    end

    subgraph "Configuration Consumers"
        CC1[DhcpServer<br/>Server Settings]
        CC2[LeaseManager<br/>Subnet Config]
        CC3[OptionsManager<br/>Options Config]
        CC4[SecurityManager<br/>Security Config]
    end

    subgraph "Configuration Updates"
        CU1[Hot Reload<br/>SIGHUP]
        CU2[Config Watcher<br/>File System]
        CU3[API Update]
    end

    CS1 --> CL1
    CS2 --> CL1
    CS3 --> CL1
    CS4 --> CL1

    CL1 --> CL2
    CL2 --> CL3
    CL3 --> CS5
    CL3 --> CS6

    CS5 --> CC1
    CS5 --> CC2
    CS5 --> CC3
    CS5 --> CC4

    CU1 --> CL1
    CU2 --> CL1
    CU3 --> CL1
```

## Options Data Flow

```mermaid
flowchart TB
    subgraph "Options Sources"
        OS1[Global Options<br/>Config File]
        OS2[Subnet Options<br/>Per-Subnet]
        OS3[Pool Options<br/>Per-Pool]
        OS4[Host Options<br/>Per-Reservation]
    end

    subgraph "Options Processing"
        OP1[OptionsManager<br/>Collect Options]
        OP2[Inheritance Resolution]
        OP3[Option Validation]
        OP4[Option Merging]
    end

    subgraph "Options Building"
        OB1[Build Option List]
        OB2[Encode Options]
        OB3[Add to Message]
    end

    subgraph "Options Response"
        OR1[DHCP Message]
        OR2[Client Receives]
    end

    OS1 --> OP1
    OS2 --> OP1
    OS3 --> OP1
    OS4 --> OP1

    OP1 --> OP2
    OP2 --> OP3
    OP3 --> OP4
    OP4 --> OB1

    OB1 --> OB2
    OB2 --> OB3
    OB3 --> OR1
    OR1 --> OR2
```

## Statistics and Metrics Data Flow

```mermaid
flowchart TB
    subgraph "Event Sources"
        ES1[Message Processing]
        ES2[Lease Operations]
        ES3[Security Events]
        ES4[Error Events]
    end

    subgraph "Statistics Collection"
        SC1[Stats Collector]
        SC2[Counter Updates]
        SC3[Histogram Updates]
        SC4[Gauge Updates]
    end

    subgraph "Metrics Storage"
        MS1[In-Memory Stats]
        MS2[Metrics Cache]
    end

    subgraph "Metrics Export"
        ME1[Prometheus Endpoint]
        ME2[Stats API]
        ME3[Log Output]
    end

    ES1 --> SC1
    ES2 --> SC1
    ES3 --> SC1
    ES4 --> SC1

    SC1 --> SC2
    SC1 --> SC3
    SC1 --> SC4

    SC2 --> MS1
    SC3 --> MS1
    SC4 --> MS1

    MS1 --> MS2

    MS2 --> ME1
    MS2 --> ME2
    MS2 --> ME3
```

## Logging Data Flow

```mermaid
flowchart TB
    subgraph "Log Sources"
        LS1[Server Events]
        LS2[Message Processing]
        LS3[Lease Operations]
        LS4[Security Events]
        LS5[Configuration Changes]
        LS6[Error Conditions]
    end

    subgraph "Log Processing"
        LP1[Logger<br/>Format & Filter]
        LP2[Log Level Check]
        LP3[Structured Formatting]
    end

    subgraph "Log Outputs"
        LO1[Console Output]
        LO2[File Output<br/>Rotated Logs]
        LO3[Syslog]
        LO4[Structured JSON]
    end

    subgraph "Log Consumers"
        LC1[Log Aggregation<br/>Fluentd/Logstash]
        LC2[Log Analysis<br/>ELK Stack]
        LC3[Monitoring<br/>Alerting]
    end

    LS1 --> LP1
    LS2 --> LP1
    LS3 --> LP1
    LS4 --> LP1
    LS5 --> LP1
    LS6 --> LP1

    LP1 --> LP2
    LP2 --> LP3

    LP3 --> LO1
    LP3 --> LO2
    LP3 --> LO3
    LP3 --> LO4

    LO2 --> LC1
    LO3 --> LC1
    LO4 --> LC1

    LC1 --> LC2
    LC1 --> LC3
```
