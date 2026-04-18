# Releasing simple-dhcpd

## Verify a tag builds (clean tree)

From the repository root, with OpenSSL and jsoncpp available (and GoogleTest if tests are enabled):

```sh
chmod +x scripts/verify-release-build.sh
./scripts/verify-release-build.sh v0.2.1 v0.3.0 v0.6.0
```

The script uses a fresh `git worktree` per tag and removes any `build/` directory checked out with the tag (some older tags accidentally carried a stale `CMakeCache.txt`).

Override CMake options if needed:

```sh
CMAKE_EXTRA="-DBUILD_VERSION=production -DENABLE_TESTS=OFF" ./scripts/verify-release-build.sh v0.3.0
```

## Tags v0.4.0, v0.5.0, and v0.6.0 (repaired)

Originally, **v0.4.0**–**v0.6.0** pointed at commits where `CMakeLists.txt` listed `src/core/dhcp/parser.cpp` (and related files) before those paths existed. Clean checkouts could not configure.

**Current tags (intended for releases):**

| Tag | Repair tip (full SHA in `git show`) |
|-----|-------------------------------------|
| **v0.4.0** | **`b703957`** — based on original v0.4.0 (`b631542`) + *track src/core sources* + *test alignment*; full build including tests. |
| **v0.5.0** | **`f2919ba`** — based on original v0.5.0 (`6dbe857`) + *track src/core sources*; same tree as `b703957`, different parent order. |
| **v0.6.0** | **Current `main`** at release time (CMake **0.6.0**, portability fixes, verify script). |

The first fix on `main` was **`ab8050a`** (*fix: track src/core sources*). Tags above use that change (or later `main`) so GitHub source archives build.

Releases for **v0.2.1** and **v0.3.0** use the older single-target layout and verify cleanly.

## GitHub releases

After a tag points at a buildable commit:

1. `git push origin vX.Y.Z` (use `git push --force origin vX.Y.Z` only if you moved an existing tag intentionally).
2. Create or edit the GitHub Release for that tag; paste the corresponding section from `CHANGELOG.md`.
3. Optional: attach CPack artifacts (`cpack` in your build directory) with `gh release upload`.

## Version source of truth

`CMakeLists.txt` sets `project(simple-dhcpd VERSION …)`. Keep `CHANGELOG.md`, packaging, and tags aligned with that version for the release you are publishing.
