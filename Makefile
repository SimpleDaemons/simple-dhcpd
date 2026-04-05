# Makefile for simple-dhcpd
# Portable across GNU make, BSD make, and BusyBox make: no ifeq/define/$(shell).
# Platform logic lives in scripts/platform-env.sh + scripts/mk-impl.sh (POSIX sh).
#
# Copyright 2024 SimpleDaemons <info@simpledaemons.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.

PROJECT_NAME = simple-dhcpd
VERSION = 0.1.0
BUILD_DIR = build
DIST_DIR = dist
PACKAGE_DIR = packaging
SRC_DIR = src
INCLUDE_DIR = include
CONFIG_DIR_SRC = config
SCRIPTS_DIR = scripts
DEPLOYMENT_DIR = deployment

# Optional overrides (passed into scripts/mk.sh)
PARALLEL_JOBS ?=
INSTALL_PREFIX ?=

MAKE ?= make
# Relative path: do not use $(CURDIR) — empty on some BSD/BusyBox makes → "/scripts/mk.sh".
MK_SH = ./scripts/mk.sh

# ROOT from the shell cwd when the recipe runs (correct with make -C dir).
# Environment passed to every mk.sh invocation (quote paths for spaces)
E = ROOT="$$(pwd)" MAKE="$(MAKE)" OS="$(OS)" SHELL="$(SHELL)" \
	PROJECT_NAME="$(PROJECT_NAME)" VERSION="$(VERSION)" \
	BUILD_DIR="$(BUILD_DIR)" DIST_DIR="$(DIST_DIR)" PACKAGE_DIR="$(PACKAGE_DIR)" \
	SRC_DIR="$(SRC_DIR)" INCLUDE_DIR="$(INCLUDE_DIR)" CONFIG_DIR_SRC="$(CONFIG_DIR_SRC)" \
	SCRIPTS_DIR="$(SCRIPTS_DIR)" DEPLOYMENT_DIR="$(DEPLOYMENT_DIR)" \
	PARALLEL_JOBS="$(PARALLEL_JOBS)" INSTALL_PREFIX="$(INSTALL_PREFIX)"

.DEFAULT_GOAL := all

all: build

$(BUILD_DIR)-dir:
	@env $(E) sh $(MK_SH) mkdir-build-dir

build: $(BUILD_DIR)-dir
	@env $(E) sh $(MK_SH) build

clean:
	@env $(E) sh $(MK_SH) clean

install: build
	@env $(E) sh $(MK_SH) install

uninstall:
	@env $(E) sh $(MK_SH) uninstall

test: build
	@env $(E) sh $(MK_SH) test

package: build
	@env $(E) sh $(MK_SH) package

dev-build: $(BUILD_DIR)-dir
	@env $(E) sh $(MK_SH) dev-build

dev-test: dev-build
	@env $(E) sh $(MK_SH) dev-test

static-build: $(BUILD_DIR)-dir
	@env $(E) sh $(MK_SH) static-build

static-test: static-build
	@env $(E) sh $(MK_SH) static-test

static-package: static-build
	@env $(E) sh $(MK_SH) static-package

static-zip: static-build
	@env $(E) sh $(MK_SH) static-zip

static-all: static-package static-zip
	@echo "All static binary packages created successfully"
	@echo "Static binary packages:"
	@ls -la "$(DIST_DIR)"/$(PROJECT_NAME)-$(VERSION)-static-* 2>/dev/null || echo "No static binary packages found"

format:
	@env $(E) sh $(MK_SH) format

check-style:
	@env $(E) sh $(MK_SH) check-style

lint: check-style
	@env $(E) sh $(MK_SH) lint

security-scan:
	@env $(E) sh $(MK_SH) security-scan

deps:
	@env $(E) sh $(MK_SH) deps

dev-deps:
	@env $(E) sh $(MK_SH) dev-deps

docker-build:
	docker build -t $(PROJECT_NAME):$(VERSION) .

docker-run:
	docker run -d --name $(PROJECT_NAME)-$(VERSION) -p 67:67 $(PROJECT_NAME):$(VERSION)

docker-stop:
	docker stop $(PROJECT_NAME)-$(VERSION)
	docker rm $(PROJECT_NAME)-$(VERSION)

service-install: install
	@env $(E) sh $(MK_SH) service-install

service-status:
	@env $(E) sh $(MK_SH) service-status

help:
	@echo "A lightweight and secure DHCP server daemon - Main Help"
	@echo "=================================="
	@echo ""
	@echo "Essential targets:"
	@echo "  all              - Build the project (default)"
	@echo "  build            - Build using CMake"
	@echo "  clean            - Clean build files"
	@echo "  install          - Install the project"
	@echo "  uninstall        - Uninstall the project"
	@echo "  test             - Run tests"
	@echo "  package          - Build platform-specific packages"
	@echo "  package-source   - Create source code packages (TAR.GZ + ZIP)"
	@echo "  package-all      - Create all packages (binary + source)"
	@echo "  package-info     - Show package information"
	@echo ""
	@echo "Development targets:"
	@echo "  dev-build        - Build in debug mode"
	@echo "  dev-test         - Run tests in debug mode"
	@echo "  format           - Format source code"
	@echo "  lint             - Run static analysis"
	@echo "  security-scan    - Run security scanning tools"
	@echo ""
	@echo "Static binary targets:"
	@echo "  static-build     - Build static binary (self-contained)"
	@echo "  static-test      - Run tests on static binary"
	@echo "  static-package   - Create platform-specific static binary package"
	@echo "  static-zip       - Create static binary ZIP package"
	@echo "  static-all       - Create all static binary formats"
	@echo ""
	@echo "Dependency management:"
	@echo "  deps             - Install dependencies"
	@echo "  dev-deps         - Install development tools"
	@echo ""
	@echo "Service management:"
	@echo "  service-install  - Install system service"
	@echo "  service-status   - Check service status"
	@echo "  service-start    - Start service"
	@echo "  service-stop     - Stop service"
	@echo ""
	@echo "Help categories:"
	@echo "  help-all         - Show all available targets"
	@echo "  help-build       - Build and development targets"
	@echo "  help-package     - Package creation targets"
	@echo "  help-deps        - Dependency management targets"
	@echo "  help-service     - Service management targets"
	@echo "  help-docker      - Docker targets"
	@echo "  help-config      - Configuration management targets"
	@echo "  help-platform    - Platform-specific targets"
	@echo ""
	@echo "Examples:"
	@echo "  make build       - Build the project"
	@echo "  make test        - Build and run tests"
	@echo "  make package     - Create platform-specific packages"
	@echo "  make dev-deps    - Install development tools"
	@echo "  make help-all    - Show all available targets"

help-all:
	@env $(E) sh $(MK_SH) help-all

help-build:
	@echo "Build: all, build, clean, install, uninstall, test, dev-build, dev-test,"
	@echo "static-build, static-test, static-package, static-zip, static-all, format, check-style, lint, security-scan"

help-package:
	@echo "Package: package, package-source, package-all, package-info, package-deb, package-rpm,"
	@echo "package-msi, package-exe, package-dmg, package-pkg, package-tgz"

help-deps:
	@echo "Dependencies: deps, dev-deps"

help-service:
	@echo "Service: service-install, service-uninstall, service-status, service-start, service-stop,"
	@echo "service-restart, service-enable, service-disable (plus legacy start/stop/restart/status)"

help-docker:
	@echo "Docker: docker-build, docker-run, docker-stop"

help-config:
	@echo "Config: config-install, config-backup, log-rotate, backup, restore, distclean"

help-platform:
	@echo "Platform detection runs inside scripts (Windows, macOS, Linux, FreeBSD). Override: PARALLEL_JOBS=8 make build"

package-source: build
	@env $(E) sh $(MK_SH) package-source

package-all: package package-source
	@echo "All packages created successfully"
	@echo "Binary packages:"
	@ls -la "$(DIST_DIR)"/$(PROJECT_NAME)-$(VERSION)-*.* 2>/dev/null || echo "No binary packages found"
	@echo "Source packages:"
	@ls -la "$(DIST_DIR)"/$(PROJECT_NAME)-$(VERSION)-src.* 2>/dev/null || echo "No source packages found"

package-deb: build
	@env $(E) sh $(MK_SH) package-deb

package-rpm: build
	@env $(E) sh $(MK_SH) package-rpm

package-msi: build
	@env $(E) sh $(MK_SH) package-msi

package-exe: build
	@env $(E) sh $(MK_SH) package-exe

package-dmg: build
	@env $(E) sh $(MK_SH) package-dmg

package-pkg: build
	@env $(E) sh $(MK_SH) package-pkg

package-tgz: build
	@env $(E) sh $(MK_SH) package-tgz

package-info:
	@env $(E) sh $(MK_SH) package-info

debug: dev-build
release: build
sanitize: dev-build
rebuild: clean build
test-verbose: test

start: service-start
stop: service-stop
restart: service-restart
status: service-status

service-start:
	@env $(E) sh $(MK_SH) service-start

service-stop:
	@env $(E) sh $(MK_SH) service-stop

service-restart:
	@env $(E) sh $(MK_SH) service-restart

service-enable:
	@env $(E) sh $(MK_SH) service-enable

service-disable:
	@env $(E) sh $(MK_SH) service-disable

service-uninstall:
	@env $(E) sh $(MK_SH) service-uninstall

config-install: install
	@env $(E) sh $(MK_SH) config-install

config-backup:
	@env $(E) sh $(MK_SH) config-backup

log-rotate: install
	@env $(E) sh $(MK_SH) log-rotate

backup: config-backup
	@env $(E) sh $(MK_SH) backup

restore: backup
	@env $(E) sh $(MK_SH) restore

distclean: clean
	@env $(E) sh $(MK_SH) distclean

.PHONY: all build clean install uninstall test package package-source package-all \
	package-deb package-rpm package-msi package-exe package-dmg package-pkg package-tgz package-info \
	static-build static-test static-package static-zip static-all \
	dev-build dev-test format check-style lint security-scan deps dev-deps \
	docker-build docker-run docker-stop service-install service-uninstall service-status \
	service-start service-stop service-restart service-enable service-disable \
	config-install config-backup log-rotate backup restore distclean \
	debug release sanitize rebuild test-verbose start stop restart status \
	help help-all help-build help-package help-deps help-service help-docker help-config help-platform
