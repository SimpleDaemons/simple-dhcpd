#!/bin/sh
# Entry point for Makefile targets (POSIX sh). Used by BusyBox make, GNU make, etc.
set -e
ROOT="${ROOT:-$(pwd)}"
case "$ROOT" in
/*) ;;
*) ROOT="$(pwd)/$ROOT" ;;
esac
cd "$ROOT" || exit 1
export MAKE="${MAKE:-make}"
export OS="${OS-}"
export SHELL="${SHELL-}"
. "$ROOT/scripts/platform-env.sh"
. "$ROOT/scripts/mk-impl.sh"
t=${1:?mk.sh: missing target}
run_target "$t"
