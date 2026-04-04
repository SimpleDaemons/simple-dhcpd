# Feature audit — implementation vs claims

**Last audited:** April 2026 (code review + **`ctest` 60/60**)  
**Canonical status:** [PROGRESS_REPORT.md](PROGRESS_REPORT.md)

---

## Summary

| Area | In codebase | In live `DhcpServer` path | Tests verify |
|------|-------------|---------------------------|--------------|
| DHCP parse / generate | Yes | Yes | **Yes** (parser + integration-style tests) |
| Lease allocate / renew / release | Yes | Yes (`LeaseManager` or `AdvancedLeaseManager`) | **Yes** |
| UDP / sockets | Yes | Yes | **Yes** (incl. loopback send/receive) |
| Multi-format config | Yes (JSON strongest) | Yes | **Yes** (JSON); YAML/INI partial |
| `DhcpSecurityManager` | Yes | **Yes** when `enable_security` | **Yes** (`test_security.cpp`) |
| `AdvancedLeaseManager` | Yes | **Yes** when `advanced_lease_database` set | Indirectly via lease tests + server build |
| SQLite lease DB | **No** | — | Text **`LEASE:`/`STATIC:`** file |
| Full DORA end-to-end over UDP:67 | **Partial** | Yes (daemon path) | **Not black-box** — in-process / synthetic |

---

## Core DHCP (`src/core/dhcp/server.cpp`)

- **Implemented:** Message dispatch; Discover/Offer/Request/ACK; Release; Decline with **declined-IP hold**; Inform; optional **security** gate; **statistics**; config-driven **server ID** and common options.
- **Gaps:** Subnet selection can still **fall back to first subnet** for ambiguous relay/client addressing; **black-box** UDP:67 testing is thin.

## Parser

- **Implemented:** Static **`DhcpParser::parse_message`** returning **`DhcpMessage`**.
- **Tests:** Updated to current API.

## Lease manager (`src/core/lease/manager.cpp`)

- **Implemented:** **`allocate_lease(mac, requested_ip, subnet_name)`**, renewal, release, expiry cleanup, **declined IP** tracking.
- **Tests:** Default suite exercises allocation paths.

## Advanced lease (`src/production/features/advanced_manager.cpp`)

- **Implemented:** Static/dynamic maps, text persistence, **`load_database()`** loads **`LEASE:`** into active state when used as server backend.

## Security (`src/production/security/manager.cpp`)

- **Implemented:** Filters, rate limits, Option 82, etc.
- **Wired:** **`DhcpServer`** consults **`DhcpSecurityManager`** when security is enabled.

## Documentation drift

README marketing bullets and some **`docs/`** pages may still imply SQLite, “complete RFC,” or **v1.0.0 shipped**. Prefer **`PROGRESS_REPORT.md`** and this file.

---

*Update this file when CI matrix, black-box DHCP tests, or release scope changes.*
