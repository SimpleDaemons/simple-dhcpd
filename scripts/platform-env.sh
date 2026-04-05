# Platform and tool variables for simple-dhcpd (POSIX sh; sourced by mk.sh).
# No GNU Make directives — safe for BusyBox make calling into this shell.

: "${ROOT:=$(pwd)}"
: "${PROJECT_NAME:=simple-dhcpd}"
: "${VERSION:=0.1.0}"
: "${BUILD_DIR:=build}"
: "${DIST_DIR:=dist}"
: "${PACKAGE_DIR:=packaging}"
: "${SRC_DIR:=src}"
: "${INCLUDE_DIR:=include}"
: "${CONFIG_DIR_SRC:=config}"
: "${SCRIPTS_DIR:=scripts}"
: "${DEPLOYMENT_DIR:=deployment}"
: "${MAKE:=make}"

UNAME_S=$(uname -s 2>/dev/null || echo Unknown)
: "${OS:=}"

# --- PLATFORM + toolchain (mirrors former Makefile ifeq tree) ---
if [ "$OS" = Windows_NT ]; then
	PLATFORM=windows
	CXX=cl
	CXXFLAGS='/std:c++17 /W3 /O2 /DNDEBUG /EHsc /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS'
	LDFLAGS='ws2_32.lib crypt32.lib'
	PARALLEL_JOBS=${PARALLEL_JOBS:-${NUMBER_OF_PROCESSORS:-4}}
	INSTALL_PREFIX=${INSTALL_PREFIX:-"C:/Program Files/${PROJECT_NAME}"}
	CONFIG_DIR="${INSTALL_PREFIX}/etc"
	EXE_EXT=.exe
	LIB_EXT=.lib
	DLL_EXT=.dll
	RM="del /Q"
	RMDIR="rmdir /S /Q"
	MKDIR=mkdir
	CP=copy
	if [ "${SHELL:-}" = /usr/bin/bash ]; then
		RM="rm -rf"
		RMDIR="rm -rf"
		MKDIR="mkdir -p"
		CP="cp -r"
		INSTALL_PREFIX="/c/Program Files/${PROJECT_NAME}"
		CONFIG_DIR="/c/Program Files/${PROJECT_NAME}/etc"
	fi
elif [ "$UNAME_S" = Darwin ]; then
	PLATFORM=macos
	CXX=clang++
	CXXFLAGS='-std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG -target x86_64-apple-macos12.0 -target arm64-apple-macos12.0'
	LDFLAGS='-lssl -lcrypto'
	PARALLEL_JOBS=${PARALLEL_JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}
	INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
	CONFIG_DIR="${INSTALL_PREFIX}/etc/${PROJECT_NAME}"
	EXE_EXT=
	LIB_EXT=.dylib
	DLL_EXT=.dylib
	RM="rm -rf"
	RMDIR="rm -rf"
	MKDIR="mkdir -p"
	CP="cp -r"
elif [ "$UNAME_S" = FreeBSD ]; then
	PLATFORM=freebsd
	CXX=c++
	CXXFLAGS='-std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG'
	LDFLAGS='-lssl -lcrypto -lpthread'
	PARALLEL_JOBS=${PARALLEL_JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}
	INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
	CONFIG_DIR="${INSTALL_PREFIX}/etc/${PROJECT_NAME}"
	EXE_EXT=
	LIB_EXT=.so
	DLL_EXT=.so
	RM="rm -rf"
	RMDIR="rm -rf"
	MKDIR="mkdir -p"
	CP="cp -r"
else
	# Linux and other Unix-like (Alpine, Arch, Debian, RHEL, etc.)
	PLATFORM=linux
	CXX=g++
	CXXFLAGS='-std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG'
	LDFLAGS='-lssl -lcrypto -lpthread'
	PARALLEL_JOBS=${PARALLEL_JOBS:-$(nproc 2>/dev/null || echo 4)}
	INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
	CONFIG_DIR="/etc/${PROJECT_NAME}"
	EXE_EXT=
	LIB_EXT=.so
	DLL_EXT=.so
	RM="rm -rf"
	RMDIR="rm -rf"
	MKDIR="mkdir -p"
	CP="cp -r"
fi

# Privileged command prefix (empty if root, else sudo/doas when available)
if [ "$PLATFORM" = windows ] && [ "${SHELL:-}" != /usr/bin/bash ]; then
	ROOT_CMD=
elif id -u >/dev/null 2>&1 && [ "$(id -u)" -eq 0 ]; then
	ROOT_CMD=
elif command -v sudo >/dev/null 2>&1; then
	ROOT_CMD=sudo
elif command -v doas >/dev/null 2>&1; then
	ROOT_CMD=doas
else
	ROOT_CMD=
fi

INSTALL_DIR=$INSTALL_PREFIX
LOG_DIR=/var/log
DATA_DIR=/var/lib/${PROJECT_NAME}

export PLATFORM PROJECT_NAME VERSION BUILD_DIR DIST_DIR PACKAGE_DIR
export SRC_DIR INCLUDE_DIR CONFIG_DIR_SRC SCRIPTS_DIR DEPLOYMENT_DIR
export CXX CXXFLAGS LDFLAGS PARALLEL_JOBS INSTALL_PREFIX CONFIG_DIR
export EXE_EXT LIB_EXT DLL_EXT RM RMDIR MKDIR CP ROOT_CMD
export INSTALL_DIR LOG_DIR DATA_DIR MAKE UNAME_S OS
