# Feature audit — implementation vs claims

**Last audited:** April 2026 (static review + CMake build)  
**Canonical status:** [PROGRESS_REPORT.md](PROGRESS_REPORT.md)

This document replaces the February 2025 audit that described many features as “fully implemented” without checking server wiring or test compilation.

---

## Summary

| Area | In codebase | In live `DhcpServer` path | Tests verify |
|------|-------------|---------------------------|--------------|
| DHCP parse / generate | Yes | Yes | **Suite broken** (compile errors) |
| Lease allocate / renew / release (`LeaseManager`) | Yes | Yes | Partial intent only; **does not build** |
| UDP / sockets | Yes | Yes | Some network tests (when build works) |
| Multi-format config | Yes (JSON strong) | Yes | Config tests (when build works) |
| `DhcpSecurityManager` | Yes (large .cpp) | **No** | `test_security.cpp` only |
| `AdvancedLeaseManager` | Yes | **No** | Not in default test binary |
| SQLite lease DB | **No** | — | Text `LEASE:`/`STATIC:` file in advanced manager |
| Full DORA end-to-end over UDP:67 | **Partial** | **Not black-box tested** | Integration test is **Discover + manual offer**, not full server |

---

## Core DHCP (`src/core/dhcp/server.cpp`)

- **Implemented:** Dispatch by message type; Discover → allocate + Offer; Request → renew or allocate + ACK; Release; Decline (release + TODO); Inform (ACK with **hardcoded** server IP).
- **Gaps:** Hardcoded server identifier and many options; `update_statistics` empty; decline quarantine missing; subnet selection can default to first subnet.

## Parser (`src/core/dhcp/parser.cpp` + header)

- **Implemented:** Static `parse_message` returning `DhcpMessage`.
- **Gap:** Tests written for an older instance-based API — **must be updated**.

## Lease manager (`src/core/lease/manager.cpp`)

- **Implemented:** Core allocation API requires **requested IP** + subnet name (three-parameter `allocate_lease`).
- **Gap:** Several tests still use the old two-argument style — **broken build**.

## Advanced lease (`src/production/features/advanced_manager.cpp`)

- **Implemented:** Static lease map, conflict helpers, text save, workers, serialization.
- **Gaps:** **`load_database()`** parses dynamic lease lines but does not load them into active leases; **not used** by `DhcpServer`.

## Security (`src/production/security/manager.cpp`)

- **Implemented:** Snooping, filters, rate limits, Option 82 helpers, HMAC helpers, config load (JSON + simplified YAML/INI).
- **Gap:** **Not called** from `DhcpServer` — security is **not** applied to live DHCP traffic unless another binary wires it.

## Options manager (`src/core/options/manager.cpp`)

- Exists as a subsystem; **server** builds many options inline — full options manager integration should be verified separately.

## Documentation drift

README and older `project/*.md` files may still claim “production ready,” “70% coverage,” “SQLite,” or “full DORA integration tests.” Treat those as **historical** unless they cite a current CI run and commit.

---

*Update this file when: tests pass, security/advanced lease are wired, or scope is explicitly narrowed in release notes.*
