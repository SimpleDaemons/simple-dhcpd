# Simple DHCP Daemon - Technical Debt

**Date:** April 2026 (major correction)
**Current Version:** CMake `0.3.0`, production target
**Purpose:** Track technical debt, known issues, and areas requiring improvement

---

## 🎯 Overview

This document tracks technical debt. **Earlier revisions overstated shipping readiness** (e.g. tests “complete” while broken). The **default test binary now passes** — see [PROGRESS_REPORT.md](PROGRESS_REPORT.md) for the current bar.

**Priority theme:** CI parity, coverage metrics, relay/subnet edge cases, YAML depth, dead test files.

---

## 🟢 Recently closed (April 2026)

### Default Google Test binary (`simple_dhcpd_tests`)
**Status:** ✅ **Resolved** — target **builds**; **`ctest`** reports **60/60** passing (parser, lease, config, security, network, integration-style, performance, load).

**Follow-ups (lower urgency):**
- [ ] Add or remove **`test_dhcp_parser.cpp`** / **`test_lease_manager.cpp`** if they should be part of the default binary.
- [ ] Add **black-box** UDP:67 DHCP tests (optional; current suite is mostly in-process).

---

## 🔴 High Priority (Critical)

### 1. Declined IP Address Tracking
**Status:** ✅ **Implemented** (verify behavior under load)

**Location:** `LeaseManager::add_declined_ip`, `DhcpServer::handle_decline`, config **`decline_hold_seconds`**.

**Remaining:**
- [ ] Dedicated unit/integration tests that assert a declined IP is not re-offered inside the hold window.

---

### 2. YAML Configuration Parser
**Status:** ⚠️ **Partial Implementation**
**Priority:** 🔴 **HIGH**
**Estimated Effort:** 12-16 hours

**Location:** `src/core/config/manager.cpp` (`parse_yaml_config`)

**Current State:**
```cpp
// Note: For production, replace with yaml-cpp.
// Very light YAML support without external deps
```

**Issue:**
- Current YAML parser is a minimal implementation
- May not handle complex YAML structures correctly
- Not production-ready for enterprise use

**Impact:**
- Configuration parsing errors
- Limited YAML feature support
- Potential security issues with custom parser

**Solution:**
- Integrate yaml-cpp library
- Replace custom parser with robust library
- Add comprehensive YAML test cases

**Action Items:**
- [ ] Add yaml-cpp as dependency
- [ ] Replace custom YAML parser
- [ ] Add YAML parsing tests
- [ ] Update documentation

**Target:** v0.3.0 release

---

### 3. Lease persistence (advanced backend)
**Status:** ⚠️ **Text `LEASE:`/`STATIC:` file — needs hardening**
**Priority:** 🔴 **HIGH**

**Current state:** `AdvancedLeaseManager` persists to a **line-oriented file**, not SQLite. **`load_database()`** loads dynamic `LEASE:` rows into the active set (April 2026).

**Action items:**
- [ ] Fuzz / corruption tests for lease files
- [ ] Backup/restore and crash-recovery procedures documented and tested
- [ ] Performance with very large files

---

## 🟡 Medium Priority (Important)

### 4. DHCP Options Configuration Persistence
**Status:** ❌ **Not Implemented**
**Priority:** 🟡 **MEDIUM**
**Estimated Effort:** 10-14 hours

**Location:** `src/dhcp_options_manager.cpp:391-400`

**Current State:**
```cpp
// TODO: Implement configuration loading
// TODO: Implement configuration saving
```

**Issue:**
- Options configuration cannot be saved/loaded
- Changes are lost on restart
- No way to persist custom options

**Impact:**
- Configuration management difficulty
- Manual reconfiguration required
- No audit trail of changes

**Solution:**
- Implement configuration save/load
- Add versioning for configuration
- Support import/export

**Action Items:**
- [ ] Implement `load_configuration()`
- [ ] Implement `save_configuration()`
- [ ] Add configuration versioning
- [ ] Add import/export functionality
- [ ] Add tests

**Target:** v0.4.0

---

### 5. Memory Management Review
**Status:** ⚠️ **Needs Review**
**Priority:** 🟡 **MEDIUM**
**Estimated Effort:** 8-12 hours

**Current State:**
- No known memory leaks
- No comprehensive memory analysis performed
- Smart pointers used but not everywhere

**Issues:**
- Potential memory leaks in edge cases
- No memory profiling
- Possible use-after-free scenarios
- No memory usage monitoring

**Impact:**
- Potential crashes in long-running deployments
- Memory growth over time
- Performance degradation

**Action Items:**
- [ ] Run Valgrind/AddressSanitizer analysis
- [ ] Review all pointer usage
- [ ] Add memory leak detection to CI
- [ ] Implement memory usage monitoring
- [ ] Add memory profiling tools

**Target:** v0.3.0 release

---

### 6. Error Handling Improvements
**Status:** ⚠️ **Partial**
**Priority:** 🟡 **MEDIUM**
**Estimated Effort:** 12-16 hours

**Current State:**
- Basic error handling in place
- Some error cases not handled
- Error messages could be more descriptive
- No error recovery strategies

**Issues:**
- Some exceptions not caught
- Generic error messages
- No retry logic for transient failures
- Limited error context

**Impact:**
- Difficult debugging
- Poor user experience
- Potential crashes

**Action Items:**
- [ ] Review all error handling paths
- [ ] Add specific error types
- [ ] Improve error messages
- [ ] Add error recovery strategies
- [ ] Add error logging context

**Target:** v0.4.0

---

### 7. Performance Optimization
**Status:** ⚠️ **Basic Optimization Only**
**Priority:** 🟡 **MEDIUM**
**Estimated Effort:** 20-30 hours

**Current State:**
- Basic optimizations in place
- No performance profiling
- No load testing
- Unknown performance characteristics

**Issues:**
- No benchmarks for current performance
- No profiling data
- Unknown bottlenecks
- No performance regression testing

**Impact:**
- May not meet performance targets (50K+ RPS)
- Unknown scalability limits
- Potential performance regressions

**Action Items:**
- [ ] Create performance benchmarks
- [ ] Profile code for bottlenecks
- [ ] Optimize hot paths
- [ ] Add performance regression tests
- [ ] Document performance characteristics

**Target:** v0.4.0

---

### 8. Logging Improvements
**Status:** ⚠️ **Functional but Could Be Better**
**Priority:** 🟡 **MEDIUM**
**Estimated Effort:** 8-12 hours

**Current State:**
- Basic logging works
- Multiple log formats supported
- No structured logging
- Limited log rotation

**Issues:**
- No structured logging (key-value pairs)
- Limited log rotation options
- No log aggregation support
- No log level filtering per component

**Impact:**
- Difficult log analysis
- Large log files
- Limited observability

**Action Items:**
- [ ] Add structured logging support
- [ ] Improve log rotation
- [ ] Add log aggregation hooks
- [ ] Implement component-level log filtering
- [ ] Add log sampling for high-volume events

**Target:** v0.4.0

---

### 9. Configuration Validation Enhancement
**Status:** ⚠️ **Basic Validation Only**
**Priority:** 🟡 **MEDIUM**
**Estimated Effort:** 10-14 hours

**Current State:**
- Basic validation in place
- Some edge cases not validated
- Error messages could be clearer
- No configuration schema documentation

**Issues:**
- Missing validation for some fields
- Unclear error messages
- No schema documentation
- No validation for option values

**Impact:**
- Runtime configuration errors
- Poor user experience
- Difficult troubleshooting

**Action Items:**
- [ ] Add comprehensive validation
- [ ] Improve error messages
- [ ] Create configuration schema docs
- [ ] Add option value validation
- [ ] Add validation tests

**Target:** v0.4.0

---

## 🟢 Low Priority (Nice to Have)

### 10. Code Documentation
**Status:** ⚠️ **Partial**
**Priority:** 🟢 **LOW**
**Estimated Effort:** 16-20 hours

**Current State:**
- Header files have basic documentation
- Some functions lack documentation
- No architecture diagrams
- Limited inline comments

**Issues:**
- Missing function documentation
- No design decision documentation
- No architecture overview
- Limited code examples

**Action Items:**
- [ ] Document all public APIs
- [ ] Add architecture diagrams
- [ ] Document design decisions
- [ ] Add code examples
- [ ] Generate API documentation

**Target:** v0.4.0

---

### 11. Refactoring Opportunities
**Status:** ⚠️ **Code Works but Could Be Better**
**Priority:** 🟢 **LOW**
**Estimated Effort:** 20-30 hours

**Areas for Refactoring:**

1. **DHCP Server Class**
   - Large class with many responsibilities
   - Could be split into smaller components
   - Message handling could be extracted

2. **Configuration Parsing**
   - Duplicate code across formats
   - Could use strategy pattern
   - Better abstraction needed

3. **Lease Management**
   - Some code duplication
   - Could improve abstraction
   - Better separation of concerns

**Action Items:**
- [ ] Refactor DHCP server into smaller components
- [ ] Extract message handling logic
- [ ] Improve configuration parsing abstraction
- [ ] Reduce code duplication
- [ ] Improve separation of concerns

**Target:** v0.5.0

---

### 12. Platform-Specific Code
**Status:** ⚠️ **Basic Support**
**Priority:** 🟢 **LOW**
**Estimated Effort:** 12-16 hours

**Current State:**
- Basic cross-platform support
- Some platform-specific code
- Windows support not fully tested

**Issues:**
- Windows support needs testing
- Some platform-specific optimizations missing
- No platform-specific documentation

**Action Items:**
- [ ] Test Windows support thoroughly
- [ ] Add platform-specific optimizations
- [ ] Document platform differences
- [ ] Add platform-specific tests

**Target:** v0.4.0

---

### 13. Dependency Management
**Status:** ⚠️ **Basic**
**Priority:** 🟢 **LOW**
**Estimated Effort:** 8-12 hours

**Current State:**
- Dependencies managed via CMake
- No dependency version pinning
- No security scanning

**Issues:**
- No dependency version management
- No security vulnerability scanning
- No dependency update strategy

**Action Items:**
- [ ] Pin dependency versions
- [ ] Add dependency security scanning
- [ ] Create dependency update process
- [ ] Document dependency management

**Target:** v0.4.0

---

### 14. Build System Improvements
**Status:** ⚠️ **Functional**
**Priority:** 🟢 **LOW**
**Estimated Effort:** 8-12 hours

**Current State:**
- CMake build system works
- Some build optimizations missing
- No build caching
- Limited build documentation

**Issues:**
- No build caching (ccache)
- No incremental build optimizations
- Limited build documentation
- No build performance metrics

**Action Items:**
- [ ] Add build caching
- [ ] Optimize build times
- [ ] Improve build documentation
- [ ] Add build performance tracking

**Target:** v0.4.0

---

## 📊 Technical Debt Summary

### By Priority
- **High Priority:** 4 items (~76-108 hours)
- **Medium Priority:** 6 items (~66-94 hours)
- **Low Priority:** 5 items (~64-90 hours)

### By Category
- **Testing:** follow-ups (black-box DHCP, optional extra sources) — default suite **green**
- **Code Quality:** 6 items (Mixed)
- **Features:** 3 items (Mixed)
- **Infrastructure:** 5 items (Low-Medium)

### Estimated Total Effort
- **Minimum:** ~150 hours (reduced due to completed testing)
- **Maximum:** ~250 hours
- **Realistic:** ~200 hours

---

## 🎯 Debt Reduction Strategy

### Phase 1: Critical path (pre–1.0)
**Status:** **In progress** — default **`ctest` green**; declined-IP hold **implemented**; YAML still minimal; lease-file persistence needs hardening.

1. ✅ Default automated tests build and pass (`simple_dhcpd_tests`)
2. ✅ In-process integration / performance / load tests in CI locally
3. ✅ Declined IP hold (`decline_hold_seconds` + `add_declined_ip`)
4. ⚠️ YAML: replace or deepen (`yaml-cpp` / stricter schema)
5. ⚠️ Advanced lease file: corruption, backup, scale testing

### Phase 2: Important Improvements (Enterprise Version 1.0.0)
**Timeline:** 3-4 months
**Focus:** Medium priority items

1. Options configuration persistence
2. Memory management review
3. Error handling improvements
4. Performance optimization
5. Logging improvements
6. Configuration validation

### Phase 3: Polish and Refactoring (Enterprise Version 2.0.0)
**Timeline:** 2-3 months
**Focus:** Low priority items

1. Code documentation
2. Refactoring opportunities
3. Platform-specific improvements
4. Dependency management
5. Build system improvements

---

## 📝 Tracking

### How to Use This Document

1. **When adding new debt:**
   - Add item with priority, effort estimate, and impact
   - Include location in codebase
   - Add action items

2. **When addressing debt:**
   - Mark items as complete
   - Update status
   - Note actual effort vs. estimate

3. **Regular review:**
   - Review monthly
   - Update priorities as needed
   - Track progress

### Status Legend
- ❌ **Not Started** - No work done
- ⚠️ **In Progress** - Work started but not complete
- ✅ **Complete** - Work finished and verified

---

## 🔗 Related Documents

- [PROJECT_STATUS.md](PROJECT_STATUS.md) - Overall project status
- [FEATURE_AUDIT.md](FEATURE_AUDIT.md) - Feature implementation status
- [PROGRESS_REPORT.md](PROGRESS_REPORT.md) - Progress tracking
- [ROADMAP.md](../ROADMAP.md) - Future roadmap

---

*Last updated: April 2026 — see [PROGRESS_REPORT.md](PROGRESS_REPORT.md)*
