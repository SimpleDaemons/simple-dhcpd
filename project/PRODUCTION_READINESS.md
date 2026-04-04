# Production readiness (honest checklist)

**Last updated:** April 2026  
**Authoritative narrative:** [PROGRESS_REPORT.md](PROGRESS_REPORT.md)

---

## Verdict

**Not ready for a v1.0.0 production release** until the items below are closed. The **main `simple-dhcpd` binary builds** (macOS verified); the **default Google Test binary does not compile** due to API drift between tests and core types.

---

## Checklist (current truth)

### Build & core runtime
- [x] Production CMake target compiles (core + production sources).
- [ ] **All automated tests compile and pass** (`simple_dhcpd_tests`).
- [ ] Windows / Linux CI parity verified on real runners (not assumed).

### Integration vs documentation
- [x] `DhcpServer` uses **`LeaseManager`** for the live datapath.
- [ ] **`DhcpSecurityManager`** invoked from `DhcpServer` when security is enabled — **not done** (manager exists; server does not call it).
- [ ] **`AdvancedLeaseManager`** selectable from server / same process — **not done**; `load_database()` does not restore dynamic `LEASE:` rows into active state (incomplete).
- [ ] Replace **hardcoded** `192.168.1.1` and fixed options in `build_subnet_options()` / Inform path with **config-driven** values.
- [ ] **`update_statistics()`** — implement or remove claims about metrics.
- [ ] **DHCP Decline** — declined-IP quarantine or documented RFC gap (`src/core/dhcp/server.cpp`).

### Testing
- [ ] Fix: static `DhcpParser::parse_message` vs tests expecting instance + output parameter.
- [ ] Fix: `allocate_lease(mac, requested_ip, subnet)` vs tests passing two arguments.
- [ ] Fix: `DhcpMessage` fields (`header.xid`, etc.) vs tests using `xid` / `yiaddr` at top level.
- [ ] Add or wire **`test_dhcp_parser.cpp`**, **`test_lease_manager.cpp`** into `tests/CMakeLists.txt` if they should ship.
- [ ] Remove or replace **placeholder** assertions in `test_basic.cpp`.
- [ ] Publish a **real coverage number** (tool-generated), not a hand-waved percentage.

### Security (claims vs datapath)
- [ ] Security **enforced on received packets** in `handle_dhcp_message` path, or docs must say “library only.”
- [ ] Review **`get_security_events()`** filtering logic if productized.

### Packaging & ops
- [ ] Confirm CPack/Docker/systemd paths on target distros (not marked done from docs alone).

---

## When to call it 1.0

Minimum bar:

1. Green `ctest` (or equivalent) on at least one Linux and macOS CI job.  
2. Security/advanced lease either **integrated** or **explicitly out of scope** for 1.0 in user-facing docs.  
3. No known **stub** in the hot path (statistics, decline policy, or documented exceptions).

---

*Previous “100% ready / GO FOR RELEASE” content was inaccurate relative to the repository state; retained only in git history.*
