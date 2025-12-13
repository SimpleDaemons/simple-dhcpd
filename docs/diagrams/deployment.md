# Simple DHCP Daemon - Deployment Diagrams

## Deployment Architecture

```mermaid
graph TB
    subgraph "Client Network"
        Client1[Client 1]
        Client2[Client 2]
        ClientN[Client N]
        Switch[Network Switch]
    end

    subgraph "DHCP Server"
        Server[simple-dhcpd<br/>Main Process]
        Config[Configuration Files<br/>/etc/simple-dhcpd/]
        Logs[Log Files<br/>/var/log/simple-dhcpd/]
        LeaseDB[Lease Database<br/>/var/lib/simple-dhcpd/]
    end

    subgraph "System Services"
        Systemd[systemd<br/>Service Manager]
        Logrotate[logrotate<br/>Log Rotation]
        Cron[Cron Jobs<br/>Backup/Maintenance]
    end

    subgraph "Monitoring"
        Prometheus[Prometheus<br/>Metrics Collection]
        Grafana[Grafana<br/>Dashboards]
        Syslog[Syslog<br/>Centralized Logging]
    end

    Client1 --> Switch
    Client2 --> Switch
    ClientN --> Switch
    Switch --> Server

    Systemd --> Server
    Systemd --> Config
    Systemd --> Logs

    Server --> LeaseDB
    Server --> Logs
    Server --> Config

    Logrotate --> Logs
    Cron --> LeaseDB
    Cron --> Config

    Server --> Prometheus
    Server --> Syslog
    Prometheus --> Grafana
```

## High Availability Deployment

```mermaid
graph TB
    subgraph "Load Balancer"
        LB[Load Balancer<br/>DHCP Relay]
    end

    subgraph "Primary Site"
        Server1[simple-dhcpd<br/>Primary Server]
        DB1[(Lease Database<br/>Primary)]
        Config1[Configuration<br/>Primary]
    end

    subgraph "Secondary Site"
        Server2[simple-dhcpd<br/>Secondary Server]
        DB2[(Lease Database<br/>Replica)]
        Config2[Configuration<br/>Replica]
    end

    subgraph "Clients"
        Client1[Client 1]
        Client2[Client 2]
        ClientN[Client N]
    end

    Client1 --> LB
    Client2 --> LB
    ClientN --> LB

    LB --> Server1
    LB --> Server2

    Server1 --> DB1
    Server2 --> DB2

    DB1 -.->|Replication| DB2
    Config1 -.->|Sync| Config2

    Server1 -.->|Health Check| Server2
    Server2 -.->|Health Check| Server1
```

## Container Deployment

```mermaid
graph TB
    subgraph "Docker Host"
        Docker[Docker Engine]
        Container[simple-dhcpd<br/>Container]
        Volumes[Volumes<br/>Config/Data/Logs]
        HostNetwork[Host Network<br/>Port 67]
    end

    subgraph "External"
        Clients[DHCP Clients]
        ConfigMount[Configuration<br/>/host/config]
        DataMount[Lease Database<br/>/host/data]
        LogMount[Log Files<br/>/host/logs]
    end

    Clients --> HostNetwork
    HostNetwork --> Container
    Container --> Volumes

    ConfigMount --> Volumes
    DataMount --> Volumes
    LogMount --> Volumes

    Docker --> Container
```

## Kubernetes Deployment

```mermaid
graph TB
    subgraph "Kubernetes Namespace: dhcp"
        Deployment[Deployment<br/>simple-dhcpd]
        Service[Service<br/>LoadBalancer]
        ConfigMap[ConfigMap<br/>Configuration]
        Secret[Secret<br/>TLS Certificates]
        PVC[PersistentVolumeClaim<br/>Lease Database]
    end

    subgraph "Kubernetes Nodes"
        Pod1[Pod 1<br/>simple-dhcpd]
        Pod2[Pod 2<br/>simple-dhcpd]
        PodN[Pod N<br/>simple-dhcpd]
    end

    subgraph "External"
        Clients[DHCP Clients]
        Ingress[Ingress<br/>Optional HTTP API]
    end

    Clients --> Service
    Service --> Pod1
    Service --> Pod2
    Service --> PodN

    Deployment --> Pod1
    Deployment --> Pod2
    Deployment --> PodN

    Pod1 --> ConfigMap
    Pod1 --> Secret
    Pod1 --> PVC

    Pod2 --> ConfigMap
    Pod2 --> Secret
    Pod2 --> PVC

    PodN --> ConfigMap
    PodN --> Secret
    PodN --> PVC

    Ingress --> Service
```

## Multi-Subnet Deployment

```mermaid
graph TB
    subgraph "DHCP Server"
        Server[simple-dhcpd<br/>Multi-Subnet Server]
        Config[Configuration<br/>Multiple Subnets]
    end

    subgraph "Subnet 1: 192.168.1.0/24"
        Switch1[Switch 1]
        Client1A[Client 1A]
        Client1B[Client 1B]
        Router1[Router 1<br/>192.168.1.1]
    end

    subgraph "Subnet 2: 192.168.2.0/24"
        Switch2[Switch 2]
        Client2A[Client 2A]
        Client2B[Client 2B]
        Router2[Router 2<br/>192.168.2.1]
    end

    subgraph "Subnet 3: 10.0.0.0/24"
        Switch3[Switch 3]
        Client3A[Client 3A]
        Client3B[Client 3B]
        Router3[Router 3<br/>10.0.0.1]
    end

    Server --> Config

    Server --> Switch1
    Server --> Switch2
    Server --> Switch3

    Switch1 --> Client1A
    Switch1 --> Client1B
    Switch1 --> Router1

    Switch2 --> Client2A
    Switch2 --> Client2B
    Switch2 --> Router2

    Switch3 --> Client3A
    Switch3 --> Client3B
    Switch3 --> Router3
```

## Service Integration

```mermaid
graph TB
    subgraph "simple-dhcpd"
        DHCP[DHCP Server]
    end

    subgraph "DNS Integration"
        DNS[DNS Server<br/>Dynamic Updates]
    end

    subgraph "Network Management"
        NetMgr[Network Manager<br/>IPAM Integration]
    end

    subgraph "Monitoring Stack"
        Prometheus[Prometheus]
        Grafana[Grafana]
        AlertManager[Alert Manager]
    end

    subgraph "Logging Stack"
        Fluentd[Fluentd<br/>Log Aggregation]
        Elasticsearch[Elasticsearch]
        Kibana[Kibana]
    end

    subgraph "Configuration Management"
        Ansible[Ansible<br/>Config Deployment]
        Git[Git<br/>Config Version Control]
    end

    DHCP -->|Lease Events| DNS
    DHCP -->|Lease Information| NetMgr
    DHCP -->|Metrics| Prometheus
    DHCP -->|Logs| Fluentd

    Prometheus --> Grafana
    Prometheus --> AlertManager

    Fluentd --> Elasticsearch
    Elasticsearch --> Kibana

    Ansible --> DHCP
    Git --> Ansible
```
