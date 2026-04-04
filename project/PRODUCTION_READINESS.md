# Production readiness (honest checklist)

**Last updated:** April 2026  
**Authoritative narrative:** [PROGRESS_REPORT.md](PROGRESS_REPORT.md)

---

## Verdict

**Not calling it v1.0.0 yet** — the codebase is **much closer** than prior audits: the **main `simple-dhcpd` binary builds**, the **default Google Test binary builds and passes** (`ctest`, **60/60**). Remaining work is **operational proof** (CI on real runners, field DHCP validation, coverage artifacts, packaging smoke tests).

---

## Checklist (current truth)

### Build & core runtime
- [x] Production CMake target compiles (core + production sources).
- [x] **Default automated tests compile and pass** (`simple_dhcpd_tests` / `ctest`, 60 tests).
- [ ] Windows / Linux CI parity verified on **real** runners (not assumed from one machine).

### Integration vs documentation
- [x] `DhcpServer` uses **`LeaseManager`** by default; **`AdvancedLeaseManager`** when `advanced_lease_database` is set.
- [x] **`DhcpSecurityManager`** used from `DhcpServer` when **`enable_security`** (optional policy file).
- [x] **`AdvancedLeaseManager::load_database()`** — dynamic **`LEASE:`** rows loaded into active leases (text format, not SQLite).
- [x] **Config-driven** server identifier, subnet mask, gateway, DNS, lease timers in offer/ack paths (see server implementation).
- [x] **`update_statistics()`** — packet counters merged into **`get_statistics()`**.
- [x] **DHCP Decline** — release + **`add_declined_ip`** with **`decline_hold_seconds`**.

### Testing
- [x] Tests aligned with static **`DhcpParser::parse_message`**, **`allocate_lease(mac, requested_ip, subnet)`**, **`DhcpMessage` / `header.xid`**.
- [ ] Add or wire **`test_dhcp_parser.cpp`**, **`test_lease_manager.cpp`** into `tests/CMakeLists.txt` if they should ship (optional).
- [ ] Remove or replace **placeholder** assertions in **`test_basic.cpp`** if that target is ever enabled.
- [ ] Publish a **real coverage number** (tool-generated).

### Security (claims vs datapath)
- [x] Security checks **before** handling packets when security is enabled (`security_allow_message`).
- [ ] Review **`get_security_events()`** filtering if exposed as a product API.

### Packaging & ops
- [ ] Confirm CPack/Docker/systemd paths on target distros (not marked done from docs alone).

---

## When to call it 1.0

Minimum bar:

1. Green **`ctest`** on **at least one Linux and one macOS** CI job (same commit).  
2. **Field** or lab validation of a **representative** deployment (clients, renew, decline, optional relay).  
3. **Coverage** report or explicit “no coverage claim” in release notes.  
4. **Packaging** smoke-tested where you claim support.

---

*Earlier “100% ready / GO FOR RELEASE” content was inaccurate; this checklist is reconciled to the repository as of April 2026.*
