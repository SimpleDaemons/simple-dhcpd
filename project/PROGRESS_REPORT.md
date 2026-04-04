# Simple DHCP Daemon — Honest Progress Report

**Last verified:** April 2026 (`cmake --build` + `ctest`, 60/60)  
**Product version (CMake):** 0.6.0 (`production` build target)  
**This file:** Canonical “what is actually true” status for the Production line. Use it when other docs disagree.

---

## Executive summary

The **production binary (`simple-dhcpd`) builds** from the core DHCP server, lease manager, parser, UDP layer, config manager, and options code. The **default test target `simple_dhcpd_tests` compiles and passes** (60 Google Tests), including parser, lease manager, config (JSON validation), security, network, integration-style, performance, and load/stress cases.

**`DhcpServer`** selects **`LeaseManager`** or **`AdvancedLeaseManager`** from config, optionally enables **`DhcpSecurityManager`** and policy file loading, tracks packet statistics, applies config-driven server identifier and common options, and handles **declined IPs** with a configurable hold time. **UDP `stop_receiving()`** uses a receive timeout so the receive thread can join without hanging indefinitely.

**Honest Production line status:** **late alpha / approaching beta** — core paths and tests are in much better shape than prior audits; **field deployment, relay-heavy sites, coverage numbers, and Windows/packaging** still need explicit validation before a “1.0” claim.

---

## What was verified (April 2026)

| Check | Result |
|--------|--------|
| `cmake .. && cmake --build .` (production, tests ON) | **`simple-dhcpd` and `simple_dhcpd_tests` build** |
| `ctest` (60 tests) | **All pass** |
| `DhcpServer` | **`LeaseManager` or `AdvancedLeaseManager`** per `advanced_lease_database`; **`DhcpSecurityManager`** when `enable_security`; stats updated on handled messages |
| `AdvancedLeaseManager::load_database()` | **`LEASE:`** lines feed **`add_lease`** (active set); format remains line-oriented text, not SQLite |
| JSON config | **`dhcp.listen` + `dhcp.subnets` required**; subnet **`network`** accepts **CIDR** or **IP + `prefix_length`**; **`range_start` / `range_end`** supported alongside **`range`** |

---

## Remaining gaps (before a defensible “Production 1.0”)

1. **Black-box DHCP** — Tests are mostly in-process (parser, lease manager, UDP loopback). End-to-end against real clients or a DHCP test harness on port 67 is still valuable.
2. **Relay / multi-subnet** — `find_subnet_for_client()` still falls back to the **first subnet** when giaddr/client IP do not match; review for production relay topologies.
3. **Coverage** — No current **gcov/llvm-cov** report committed; do not claim a percentage without one.
4. **Windows / packaging** — CI and manual validation on each claimed artifact (DEB/RPM/DMG/MSI) as needed.
5. **YAML / INI config** — Less strict than JSON (e.g. no universal “must include listen + subnets” rule); document or align if users rely on those paths.

---

## Enterprise / datacenter lines

Unchanged: **roadmap-only**; same core + stubs for future mains. Not the focus of this report.

---

## Other project docs

Older audits may **overstate** SQLite, full DORA UDP integration tests, or “100% complete” language. Prefer **this file** and the **current `ctest` output** when deciding what is shipped vs planned.
