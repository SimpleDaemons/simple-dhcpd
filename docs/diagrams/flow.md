# Simple DHCP Daemon - Flow Diagrams

## DHCP Message Processing Flow

```mermaid
flowchart TD
    Start([UDP Message Received]) --> Parse[Parse DHCP Message]
    Parse --> Validate{Valid Message?}
    Validate -->|No| LogError[Log Error]
    Validate -->|Yes| CheckType{Message Type?}

    CheckType -->|DISCOVER| DiscoverFlow[Discover Handler]
    CheckType -->|REQUEST| RequestFlow[Request Handler]
    CheckType -->|RELEASE| ReleaseFlow[Release Handler]
    CheckType -->|DECLINE| DeclineFlow[Decline Handler]
    CheckType -->|INFORM| InformFlow[Inform Handler]

    DiscoverFlow --> SecurityCheck[Security Validation]
    SecurityCheck -->|Pass| FindSubnet[Find Subnet for Client]
    SecurityCheck -->|Fail| Reject[Reject Request]
    FindSubnet --> AllocateIP[Allocate IP Address]
    AllocateIP --> BuildOffer[Build DHCP Offer]
    BuildOffer --> SendOffer[Send Offer Message]

    RequestFlow --> SecurityCheck2[Security Validation]
    SecurityCheck2 -->|Pass| ConfirmLease[Confirm Lease]
    SecurityCheck2 -->|Fail| SendNAK[Send NAK]
    ConfirmLease -->|Success| BuildACK[Build DHCP ACK]
    ConfirmLease -->|Fail| SendNAK
    BuildACK --> SendACK[Send ACK Message]

    ReleaseFlow --> ReleaseLease[Release Lease]
    ReleaseLease --> LogRelease[Log Release]

    DeclineFlow --> MarkDeclined[Mark IP as Declined]
    MarkDeclined --> LogDecline[Log Decline]

    InformFlow --> BuildInfo[Build Information Response]
    BuildInfo --> SendInfo[Send Information]

    SendOffer --> UpdateStats[Update Statistics]
    SendACK --> UpdateStats
    SendNAK --> UpdateStats
    LogError --> End([End])
    Reject --> End
    UpdateStats --> End
    LogRelease --> End
    LogDecline --> End
    SendInfo --> End
```

## Lease Allocation Flow

```mermaid
flowchart TD
    Start([Lease Request]) --> CheckStatic{Static Lease?}
    CheckStatic -->|Yes| GetStatic[Get MAC-to-IP Binding]
    CheckStatic -->|No| CheckRange[Check Subnet Range]

    GetStatic --> ValidateStatic{IP Available?}
    ValidateStatic -->|Yes| AllocateStatic[Allocate Static Lease]
    ValidateStatic -->|No| CheckRange

    CheckRange --> SelectIP[Select IP from Range]
    SelectIP --> ConflictCheck[Check for Conflicts]
    ConflictCheck -->|Conflict| NextIP[Try Next IP]
    NextIP --> ConflictCheck
    ConflictCheck -->|No Conflict| CheckDeclined{Previously Declined?}

    CheckDeclined -->|Yes| WaitPeriod{Wait Period Expired?}
    WaitPeriod -->|No| NextIP
    WaitPeriod -->|Yes| AllocateDynamic[Allocate Dynamic Lease]
    CheckDeclined -->|No| AllocateDynamic

    AllocateStatic --> SetLease[Set Lease Parameters]
    AllocateDynamic --> SetLease
    SetLease --> StoreLease[Store Lease]
    StoreLease --> ReturnLease([Return Lease])
```

## Security Processing Flow

```mermaid
flowchart TD
    Start([Incoming Request]) --> SnoopingCheck{DHCP Snooping Enabled?}
    SnoopingCheck -->|Yes| CheckTrusted{Trusted Interface?}
    SnoopingCheck -->|No| MACCheck

    CheckTrusted -->|No| RejectSnoop[Reject - Untrusted Interface]
    CheckTrusted -->|Yes| MACCheck[MAC Address Filter]

    MACCheck --> MACAllowed{MAC Allowed?}
    MACAllowed -->|No| RejectMAC[Reject - MAC Filtered]
    MACAllowed -->|Yes| IPCheck[IP Address Filter]

    IPCheck --> IPAllowed{IP Allowed?}
    IPAllowed -->|No| RejectIP[Reject - IP Filtered]
    IPAllowed -->|Yes| RateLimit[Rate Limiting Check]

    RateLimit --> RateOK{Within Limits?}
    RateOK -->|No| RejectRate[Reject - Rate Limited]
    RateOK -->|Yes| AuthCheck[Authentication Check]

    AuthCheck --> AuthOK{Authenticated?}
    AuthOK -->|No| RejectAuth[Reject - Auth Failed]
    AuthOK -->|Yes| Allow[Allow Request]

    RejectSnoop --> LogSecurity[Log Security Event]
    RejectMAC --> LogSecurity
    RejectIP --> LogSecurity
    RejectRate --> LogSecurity
    RejectAuth --> LogSecurity
    Allow --> Process([Process Request])
    LogSecurity --> End([End])
```

## Configuration Reload Flow

```mermaid
flowchart TD
    Start([SIGHUP Signal Received]) --> ValidateSignal{Signal Valid?}
    ValidateSignal -->|No| LogError[Log Error]
    ValidateSignal -->|Yes| LockConfig[Lock Configuration]

    LockConfig --> LoadFile[Load Config File]
    LoadFile --> ParseConfig[Parse Configuration]
    ParseConfig --> ValidateConfig{Config Valid?}

    ValidateConfig -->|No| Rollback[Rollback to Previous Config]
    ValidateConfig -->|Yes| DiffConfig[Compare with Current]

    DiffConfig --> HasChanges{Changes Detected?}
    HasChanges -->|No| NoOp[No Operation Needed]
    HasChanges -->|Yes| UpdateComponents[Update Components]

    UpdateComponents --> UpdateServer[Update Server Config]
    UpdateComponents --> UpdateSubnets[Update Subnet Config]
    UpdateComponents --> UpdateSecurity[Update Security Config]
    UpdateComponents --> UpdateOptions[Update Options Config]

    UpdateServer --> NotifyComponents[Notify Components]
    UpdateSubnets --> NotifyComponents
    UpdateSecurity --> NotifyComponents
    UpdateOptions --> NotifyComponents

    NotifyComponents --> UnlockConfig[Unlock Configuration]
    Rollback --> UnlockConfig
    NoOp --> UnlockConfig
    UnlockConfig --> LogReload[Log Reload Event]
    LogError --> End([End])
    LogReload --> End
```

## Lease Renewal Flow

```mermaid
flowchart TD
    Start([DHCP Request - Renewal]) --> ExtractInfo[Extract Client Info<br/>MAC, Current IP]
    ExtractInfo --> FindLease[Find Existing Lease]

    FindLease --> LeaseFound{Lease Found?}
    LeaseFound -->|No| SendNAK[Send NAK]
    LeaseFound -->|Yes| CheckExpired{Lease Expired?}

    CheckExpired -->|Yes| CheckGrace{In Grace Period?}
    CheckGrace -->|No| ReleaseLease[Release Lease]
    CheckGrace -->|Yes| ExtendLease[Extend Lease]

    CheckExpired -->|No| ValidateIP{IP Still Valid?}
    ValidateIP -->|No| SendNAK
    ValidateIP -->|Yes| CheckSubnet{Subnet Still Valid?}

    CheckSubnet -->|No| SendNAK
    CheckSubnet -->|Yes| UpdateLease[Update Lease Timestamp]

    ExtendLease --> UpdateLease
    UpdateLease --> BuildACK[Build DHCP ACK]
    BuildACK --> SendACK[Send ACK Message]

    ReleaseLease --> LogRelease[Log Lease Release]
    SendNAK --> LogNAK[Log NAK Event]
    SendACK --> LogACK[Log ACK Event]

    LogRelease --> End([End])
    LogNAK --> End
    LogACK --> End
```

## Error Handling Flow

```mermaid
flowchart TD
    Start([Operation Starts]) --> TryOperation[Try Operation]
    TryOperation --> Success{Success?}

    Success -->|Yes| ReturnSuccess[Return Success]
    Success -->|No| ClassifyError[Classify Error Type]

    ClassifyError --> ErrorType{Error Type?}

    ErrorType -->|Network Error| HandleNetwork[Handle Network Error<br/>Retry/Timeout]
    ErrorType -->|Parse Error| HandleParse[Handle Parse Error<br/>Log & Reject]
    ErrorType -->|Config Error| HandleConfig[Handle Config Error<br/>Validate & Rollback]
    ErrorType -->|Security Error| HandleSecurity[Handle Security Error<br/>Log & Block]
    ErrorType -->|Lease Error| HandleLease[Handle Lease Error<br/>Conflict Resolution]
    ErrorType -->|Unknown Error| HandleUnknown[Handle Unknown Error<br/>Log & Report]

    HandleNetwork --> Retry{Retry Possible?}
    Retry -->|Yes| TryOperation
    Retry -->|No| LogError[Log Error]

    HandleParse --> LogError
    HandleConfig --> LogError
    HandleSecurity --> LogError
    HandleLease --> LogError
    HandleUnknown --> LogError

    LogError --> NotifyMonitoring[Notify Monitoring System]
    NotifyMonitoring --> ReturnError[Return Error]

    ReturnSuccess --> End([End])
    ReturnError --> End
```

## Performance Optimization Flow

```mermaid
flowchart TD
    Start([Message Received]) --> CheckCache{Cache Hit?}
    CheckCache -->|Yes| ReturnCached[Return Cached Response]
    CheckCache -->|No| ProcessMessage[Process Message]

    ProcessMessage --> CheckPool{Thread Pool Available?}
    CheckPool -->|Yes| AssignThread[Assign to Worker Thread]
    CheckPool -->|No| QueueMessage[Queue Message]

    AssignThread --> ProcessInThread[Process in Thread]
    QueueMessage --> WaitForThread[Wait for Available Thread]
    WaitForThread --> AssignThread

    ProcessInThread --> BatchCheck{Batch Processing?}
    BatchCheck -->|Yes| BatchProcess[Process Batch]
    BatchCheck -->|No| SingleProcess[Process Single]

    BatchProcess --> UpdateCache[Update Cache]
    SingleProcess --> UpdateCache

    UpdateCache --> UpdateStats[Update Statistics]
    UpdateStats --> CheckMetrics{Metrics Threshold?}

    CheckMetrics -->|Exceeded| AdjustPerformance[Adjust Performance<br/>Scale Threads/Increase Cache]
    CheckMetrics -->|Normal| Continue[Continue Normal Operation]

    AdjustPerformance --> Continue
    ReturnCached --> End([End])
    Continue --> End
```
