# Simple DHCP Daemon — Honest Progress Report

**Last verified:** April 2026 (code + CMake build reviewed)  
**Product version (CMake):** 0.3.0 (`production` build target)  
**This file:** Canonical “what is actually true” status for the Production line. Use it when other docs disagree.

---

## Executive summary

The **production binary (`simple-dhcpd`) builds** from the core DHCP server, lease manager, parser, UDP layer, config manager, and options code. There is **real logic** for Discover/Offer/Request/ACK-style handling, lease allocation, and multi-format config loading.

The project is **not** “100% production-complete” in the sense of *verified* quality: the **Google Test target does not compile** against the current C++ API (parser is static; lease API changed; message shape differs from what tests assume). Several **large subsystems exist as compiled code but are not wired into the running server**. Documentation and older audits **overstate** integration, SQLite usage, full DORA end-to-end testing, and security in the live datapath.

**Honest Production line status:** **alpha / pre–1.0** — promising core, **must fix tests and integration gaps** before claiming production readiness.

---

## What was verified (April 2026)

| Check | Result |
|--------|--------|
| `cmake .. && cmake --build .` (production, tests ON) | **`simple-dhcpd` executable builds** |
| `simple_dhcpd_tests` | **Fails to compile** — API mismatch vs `test_integration.cpp`, `test_performance.cpp`, `test_load.cpp` |
| `DhcpServer` (`src/core/dhcp/server.cpp`) | Uses **`LeaseManager` only** — not `AdvancedLeaseManager` |
| `DhcpSecurityManager` | **Not referenced** from `DhcpServer`; exercised only from **`test_security.cpp`** |
| `AdvancedLeaseManager::load_database()` | Reads `LEASE:` lines but **does not insert deserialized dynamic leases** into the active lease set (incomplete load path) |

---

## Stubs, shortcuts, and incomplete wiring

### Runtime server (`src/core/dhcp/server.cpp`)

- **`update_statistics()`** — explicitly a no-op / “simplified” placeholder.
- **Server identifier and many options** — **hardcoded** `192.168.1.1` and fixed subnet mask/router/DNS in `build_subnet_options()` rather than fully driving from config.
- **DHCP Decline** — **TODO** in source: no persistent “declined IP” quarantine (risk of immediate re-offer of a bad address).
- **Relay / multi-subnet** — `find_subnet_for_client()` falls back to the **first subnet** when giaddr/client IP do not match; fine for lab, risky for real multi-subnet/relay setups without review.

### “Production” library extras (`src/production/`)

- **`DhcpSecurityManager`**: Comment block `// Placeholder implementations for remaining methods` is **misleading** — much of the file is real logic; still **not called** from `DhcpServer`, so it does not protect the live daemon.
- **`AdvancedLeaseManager`**: Substantial code, but **not used** by `DhcpServer`. Persistence is **line-oriented text** (`LEASE:` / `STATIC:`), **not SQLite** (despite older docs).
- **`get_security_events()`** filtering logic is suspicious (event type filter looks inconsistent); needs review if used in production.

### Tests

- **`tests/test_basic.cpp`**: Hand-rolled tests **return `true` without asserting real behavior** (“placeholder”).
- **`tests/test_dhcp_parser.cpp`**, **`tests/test_lease_manager.cpp`**: Present in tree but **not listed in `tests/CMakeLists.txt`** — not part of the default test binary.
- **“Integration” / “load” / “performance”** tests: Mostly **in-process** parser + `LeaseManager` (where they match API); they are **not** the same as a black-box DHCP test against UDP/67, and **currently do not build**.

---

## What actually works (high level)

- **DHCP message parse/generate path** exists (static `DhcpParser::parse_message`, builders used by server).
- **Core lease lifecycle** in **`LeaseManager`**: allocation, renewal, release paths used by the server.
- **UDP + socket manager** initialized from config; message handler dispatches by type.
- **Config**: JSON via jsoncpp; YAML/INI paths exist in config manager (with varying depth — see code for exact coverage).

---

## Gap list to reach a defensible “Production 1.0”

1. **Fix the test suite** — align tests with `DhcpParser` static API, `DhcpMessage` layout (`header.xid`, etc.), and `allocate_lease(mac, requested_ip, subnet)`; restore CI meaning.
2. **Wire or drop** — Either integrate **`DhcpSecurityManager`** and **`AdvancedLeaseManager`** into `DhcpServer` behind config flags, or stop claiming they are part of the production datapath.
3. **Complete `AdvancedLeaseManager::load_database()`** for dynamic leases, or document file format as “static only” until fixed.
4. **Replace hardcoded** server ID / options with config-driven values; implement **declined-IP** behavior or document as non-compliant.
5. **Measure coverage** with `gcov`/`llvm-cov` — do not claim “~70%” without a current report.
6. **Windows / packaging** — still need real validation on each claimed platform.

---

## Enterprise / datacenter lines

Unchanged: **roadmap-only**; same core + stubs for future mains. Not the focus of this report.

---

## Other project docs

Older files (`PROJECT_STATUS.md`, `PRODUCTION_READINESS.md`, `FEATURE_AUDIT.md`, `IMPLEMENTATION_SUMMARY.md`, `TECHNICAL_DEBT.md`, `ROADMAP_CHECKLIST.md`) may still contain **February 2025 “100% complete” language**. Treat **this file** as authoritative until those are reconciled.

---

*Next update: after tests compile and pass, and security/advanced lease paths are either integrated or scoped out of “production” claims.*
