#!/usr/bin/env sh
# Verify that release tags configure and build with a clean CMake tree.
# Usage (from repo root): ./scripts/verify-release-build.sh [tag...]
# Default tags: v0.2.1 v0.3.0 v0.4.0 v0.5.0 v0.6.0
#
# Note: If v0.4.0–v0.6.0 point at commits before "fix: track src/core sources",
# CMake will fail (missing src/core/dhcp/*.cpp). Use an updated v0.6.0 tag or
# see RELEASING.md.

set -eu

ROOT=$(CDPATH= cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

VERIFY_ROOT=${VERIFY_ROOT:-"$ROOT/.release-verify"}
CMAKE_EXTRA=${CMAKE_EXTRA:-""}

git worktree prune >/dev/null 2>&1 || true

if [ "$#" -gt 0 ]; then
  TAGS=$*
else
  TAGS="v0.1.0 v0.2.0 v0.2.1 v0.3.0 v0.4.0 v0.5.0 v0.6.0"
fi

fail=0
mkdir -p "$VERIFY_ROOT"
for tag in $TAGS; do
  printf '%s\n' "========== $tag =========="
  wt="$VERIFY_ROOT/$tag"
  git worktree remove --force "$wt" >/dev/null 2>&1 || true
  rm -rf "$wt"
  git worktree add "$wt" "$tag" >/dev/null
  rm -rf "$wt/build"
  if cmake -S "$wt" -B "$wt/build" -DCMAKE_BUILD_TYPE=Release $CMAKE_EXTRA; then
    if cmake --build "$wt/build" --parallel; then
      printf '%s\n' "OK $tag"
    else
      printf '%s\n' "FAIL $tag (build)" >&2
      fail=1
    fi
  else
    printf '%s\n' "FAIL $tag (configure)" >&2
    if printf '%s\n' "$tag" | grep -qE '^v0\.(4|5|6)\.0$'; then
      printf '%s\n' "  Hint: tags v0.4.0–v0.6.0 historically pointed at commits where" >&2
      printf '%s\n' "  CMake listed src/core/dhcp/*.cpp before those files existed." >&2
      printf '%s\n' "  First buildable commit in that era: ab8050a; current main is preferred." >&2
    fi
    fail=1
  fi
done

git worktree prune >/dev/null 2>&1 || true
exit "$fail"
