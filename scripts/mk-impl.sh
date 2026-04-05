# Target implementations for simple-dhcpd (POSIX sh; sourced after platform-env.sh).

run_root() {
	if [ -n "run_root" ]; then
		"run_root" "$@"
	else
		"$@"
	fi
}

cmake_stale_guard() {
	here=$ROOT
	wantbuild="$here/$BUILD_DIR"
	[ -f "$BUILD_DIR/CMakeCache.txt" ] || return 0
	cachedhome=$(grep '^CMAKE_HOME_DIRECTORY:INTERNAL=' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null | head -1 | sed 's/^[^=]*=//')
	cachedbuild=$(grep '^CMAKE_CACHEFILE_DIR:INTERNAL=' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null | head -1 | sed 's/^[^=]*=//')
	stale=
	[ -n "$cachedhome" ] && [ "$cachedhome" != "$here" ] && stale=1
	[ -n "$cachedbuild" ] && [ "$cachedbuild" != "$wantbuild" ] && stale=1
	if [ -n "$stale" ]; then
		echo "CMake cache does not match this checkout (remove stale build/). Reconfiguring $BUILD_DIR from scratch."
		$RM "$BUILD_DIR" 2>/dev/null || true
		$MKDIR "$BUILD_DIR"
	fi
}

submake() {
	(cd "$BUILD_DIR" && $MAKE "$@")
}

cmd_mkdir_build_dir() {
	$MKDIR "$BUILD_DIR"
}

cmd_build() {
	if [ "$PLATFORM" = windows ]; then
		(cd "$BUILD_DIR" && cmake .. -G "Visual Studio 16 2019" -A x64 && cmake --build . --config Release)
	else
		cmake_stale_guard
		(cd "$BUILD_DIR" && cmake .. && $MAKE -j"$PARALLEL_JOBS")
	fi
}

cmd_clean() {
	if [ "$PLATFORM" = windows ]; then
		$RMDIR "$BUILD_DIR" 2>/dev/null || true
		$RMDIR "$DIST_DIR" 2>/dev/null || true
	else
		$RM "$BUILD_DIR" 2>/dev/null || true
		$RM "$DIST_DIR" 2>/dev/null || true
	fi
}

cmd_install() {
	if [ "$PLATFORM" = windows ]; then
		(cd "$BUILD_DIR" && cmake --install . --prefix "$INSTALL_PREFIX")
	else
		(cd "$BUILD_DIR" && run_root $MAKE install)
	fi
}

cmd_uninstall() {
	if [ "$PLATFORM" = windows ]; then
		$RMDIR "$INSTALL_PREFIX" 2>/dev/null || true
	else
		run_root rm -f "${INSTALL_PREFIX}/bin/${PROJECT_NAME}"
		run_root rm -f "${INSTALL_PREFIX}/lib/lib${PROJECT_NAME}.so"
		run_root rm -f "${INSTALL_PREFIX}/lib/lib${PROJECT_NAME}.dylib"
		run_root rm -rf "${INSTALL_PREFIX}/include/${PROJECT_NAME}"
		run_root rm -rf "$CONFIG_DIR"
	fi
}

cmd_test() {
	if [ "$PLATFORM" = windows ]; then
		(cd "$BUILD_DIR" && ctest --output-on-failure)
	else
		submake test
	fi
}

cmd_package() {
	$MKDIR "$DIST_DIR"
	case "$PLATFORM" in
	macos)
		echo "Building macOS packages..."
		(cd "$BUILD_DIR" && cpack -G DragNDrop)
		(cd "$BUILD_DIR" && cpack -G productbuild)
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.dmg "$DIST_DIR/" 2>/dev/null || true
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.pkg "$DIST_DIR/" 2>/dev/null || true
		echo "macOS packages created: DMG and PKG"
		;;
	linux)
		echo "Building Linux packages..."
		(cd "$BUILD_DIR" && cpack -G RPM)
		(cd "$BUILD_DIR" && cpack -G DEB)
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.rpm "$DIST_DIR/" 2>/dev/null || true
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.deb "$DIST_DIR/" 2>/dev/null || true
		echo "Linux packages created: RPM and DEB"
		;;
	freebsd)
		echo "Building FreeBSD packages..."
		(cd "$BUILD_DIR" && cpack -G TGZ)
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.tar.gz "$DIST_DIR/" 2>/dev/null || true
		(cd "$BUILD_DIR" && cpack -G FREEBSD || true)
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.pkg "$DIST_DIR/" 2>/dev/null || true
		mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.txz "$DIST_DIR/" 2>/dev/null || true
		echo "FreeBSD packages created: TGZ (and native .pkg/.txz if CPack FREEBSD is available)"
		;;
	windows)
		echo "Building Windows packages..."
		$MKDIR "$DIST_DIR"
		(cd "$BUILD_DIR" && cpack -G WIX)
		(cd "$BUILD_DIR" && cpack -G ZIP)
		$CP "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.msi "$DIST_DIR"/ 2>/dev/null || true
		$CP "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.zip "$DIST_DIR"/ 2>/dev/null || true
		echo "Windows packages created: MSI and ZIP"
		;;
	*)
		echo "Package generation not supported on this platform"
		;;
	esac
}

cmd_dev_build() {
	if [ "$PLATFORM" = windows ]; then
		(cd "$BUILD_DIR" && cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Debug && cmake --build . --config Debug)
	else
		cmake_stale_guard
		(cd "$BUILD_DIR" && cmake .. -DCMAKE_BUILD_TYPE=Debug && $MAKE -j"$PARALLEL_JOBS")
	fi
}

cmd_dev_test() {
	cmd_test
}

cmd_static_build() {
	if [ "$PLATFORM" = windows ]; then
		echo "Building static binary..."
		(cd "$BUILD_DIR" && cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release -DENABLE_STATIC_LINKING=ON && cmake --build . --config Release)
	else
		echo "Building static binary..."
		cmake_stale_guard
		(cd "$BUILD_DIR" && cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_STATIC_LINKING=ON && $MAKE -j"$PARALLEL_JOBS")
	fi
}

cmd_static_test() {
	cmd_test
}

cmd_static_package() {
	$MKDIR "$DIST_DIR"
	case "$PLATFORM" in
	windows)
		echo "Creating static binary package..."
		echo "Creating Windows static binary ZIP..."
		$MKDIR "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-windows"
		cp "$BUILD_DIR/${PROJECT_NAME}.exe" "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-windows/"
		cp README.md "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-windows/"
		cp LICENSE "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-windows/"
		cp -r config "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-windows/"
		(cd "$DIST_DIR" && powershell -Command "Compress-Archive -Path '${PROJECT_NAME}-${VERSION}-static-windows' -DestinationPath '${PROJECT_NAME}-${VERSION}-static-windows.zip' -Force")
		rm -rf "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-windows"
		echo "Windows static binary package created: ${PROJECT_NAME}-${VERSION}-static-windows.zip"
		;;
	macos)
		echo "Creating static binary package..."
		echo "Creating macOS static binary TAR.GZ..."
		$MKDIR "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-macos"
		cp "$BUILD_DIR/$PROJECT_NAME" README.md LICENSE "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-macos/"
		cp -r config "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-macos/"
		(cd "$DIST_DIR" && tar -czf "${PROJECT_NAME}-${VERSION}-static-macos.tar.gz" "${PROJECT_NAME}-${VERSION}-static-macos")
		rm -rf "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-macos"
		echo "macOS static binary package created: ${PROJECT_NAME}-${VERSION}-static-macos.tar.gz"
		;;
	linux)
		echo "Creating static binary package..."
		echo "Creating Linux static binary TAR.GZ..."
		$MKDIR "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-linux"
		cp "$BUILD_DIR/$PROJECT_NAME" README.md LICENSE "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-linux/"
		cp -r config "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-linux/"
		(cd "$DIST_DIR" && tar -czf "${PROJECT_NAME}-${VERSION}-static-linux.tar.gz" "${PROJECT_NAME}-${VERSION}-static-linux")
		rm -rf "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-linux"
		echo "Linux static binary package created: ${PROJECT_NAME}-${VERSION}-static-linux.tar.gz"
		;;
	freebsd)
		echo "Creating static binary package..."
		echo "Creating FreeBSD static binary TAR.GZ..."
		$MKDIR "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-freebsd"
		cp "$BUILD_DIR/$PROJECT_NAME" README.md LICENSE "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-freebsd/"
		cp -r config "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-freebsd/"
		(cd "$DIST_DIR" && tar -czf "${PROJECT_NAME}-${VERSION}-static-freebsd.tar.gz" "${PROJECT_NAME}-${VERSION}-static-freebsd")
		rm -rf "$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-freebsd"
		echo "FreeBSD static binary package created: ${PROJECT_NAME}-${VERSION}-static-freebsd.tar.gz"
		;;
	*)
		echo "Creating static binary package..."
		echo "Static binary package generation not supported on this platform"
		;;
	esac
}

cmd_static_zip() {
	$MKDIR "$DIST_DIR"
	staged="$DIST_DIR/${PROJECT_NAME}-${VERSION}-static-$PLATFORM"
	$MKDIR "$staged"
	exesuf=
	[ "$PLATFORM" = windows ] && exesuf=.exe
	cp "$BUILD_DIR/${PROJECT_NAME}${exesuf}" README.md LICENSE "$staged/"
	cp -r config "$staged/"
	if [ "$PLATFORM" = windows ]; then
		(cd "$DIST_DIR" && powershell -Command "Compress-Archive -Path '${PROJECT_NAME}-${VERSION}-static-${PLATFORM}' -DestinationPath '${PROJECT_NAME}-${VERSION}-static-${PLATFORM}.zip' -Force")
	else
		(cd "$DIST_DIR" && zip -r "${PROJECT_NAME}-${VERSION}-static-${PLATFORM}.zip" "${PROJECT_NAME}-${VERSION}-static-${PLATFORM}")
	fi
	rm -rf "$staged"
	echo "Static binary ZIP package created: ${PROJECT_NAME}-${VERSION}-static-${PLATFORM}.zip"
}

cmd_format() {
	if [ "$PLATFORM" = windows ]; then
		echo "Code formatting on Windows requires clang-format to be installed"
		if command -v clang-format >/dev/null 2>&1; then
			find "$SRC_DIR" "$INCLUDE_DIR" \( -name "*.cpp" -o -name "*.hpp" \) -print | xargs clang-format -i
		else
			echo "clang-format not found. Install it or use the build script: scripts\\build-windows.bat --deps"
		fi
	else
		echo "Formatting source code..."
		if command -v clang-format >/dev/null 2>&1; then
			find "$SRC_DIR" "$INCLUDE_DIR" \( -name "*.cpp" -o -name "*.hpp" \) -print | xargs clang-format -i
			echo "Code formatting completed successfully"
		else
			echo "clang-format not found. Installing development tools..."
			(cd "$ROOT" && env ROOT="$ROOT" MAKE="$MAKE" PROJECT_NAME="$PROJECT_NAME" VERSION="$VERSION" BUILD_DIR="$BUILD_DIR" DIST_DIR="$DIST_DIR" PACKAGE_DIR="$PACKAGE_DIR" SRC_DIR="$SRC_DIR" INCLUDE_DIR="$INCLUDE_DIR" CONFIG_DIR_SRC="$CONFIG_DIR_SRC" SCRIPTS_DIR="$SCRIPTS_DIR" DEPLOYMENT_DIR="$DEPLOYMENT_DIR" sh "$ROOT/scripts/mk.sh" dev-deps)
			echo "Please run 'make format' again after installation"
			exit 1
		fi
	fi
}

cmd_check_style() {
	if [ "$PLATFORM" = windows ]; then
		echo "Code style checking on Windows requires clang-format to be installed"
		if command -v clang-format >/dev/null 2>&1; then
			find "$SRC_DIR" "$INCLUDE_DIR" \( -name "*.cpp" -o -name "*.hpp" \) -print | xargs clang-format --dry-run --Werror
		else
			echo "clang-format not found. Install it or use the build script: scripts\\build-windows.bat --deps"
		fi
	else
		echo "Checking code style..."
		if command -v clang-format >/dev/null 2>&1; then
			find "$SRC_DIR" "$INCLUDE_DIR" \( -name "*.cpp" -o -name "*.hpp" \) -print | xargs clang-format --dry-run --Werror
			echo "Code style check completed successfully"
		else
			echo "clang-format not found. Installing development tools..."
			(cd "$ROOT" && env ROOT="$ROOT" MAKE="$MAKE" PROJECT_NAME="$PROJECT_NAME" VERSION="$VERSION" BUILD_DIR="$BUILD_DIR" DIST_DIR="$DIST_DIR" PACKAGE_DIR="$PACKAGE_DIR" SRC_DIR="$SRC_DIR" INCLUDE_DIR="$INCLUDE_DIR" CONFIG_DIR_SRC="$CONFIG_DIR_SRC" SCRIPTS_DIR="$SCRIPTS_DIR" DEPLOYMENT_DIR="$DEPLOYMENT_DIR" sh "$ROOT/scripts/mk.sh" dev-deps)
			echo "Please run 'make check-style' again after installation"
			exit 1
		fi
	fi
}

cmd_lint() {
	if [ "$PLATFORM" = windows ]; then
		echo "Code linting on Windows requires cppcheck to be installed"
		if command -v cppcheck >/dev/null 2>&1; then
			cppcheck --enable=all --std=c++17 "$SRC_DIR" "$INCLUDE_DIR"
		else
			echo "cppcheck not found. Install it or use the build script: scripts\\build-windows.bat --deps"
		fi
	else
		echo "Running static analysis with cppcheck..."
		if command -v cppcheck >/dev/null 2>&1; then
			cppcheck --enable=all --std=c++17 "$SRC_DIR" "$INCLUDE_DIR"
			echo "Static analysis completed successfully"
		else
			echo "cppcheck not found. Installing development tools..."
			(cd "$ROOT" && env ROOT="$ROOT" MAKE="$MAKE" PROJECT_NAME="$PROJECT_NAME" VERSION="$VERSION" BUILD_DIR="$BUILD_DIR" DIST_DIR="$DIST_DIR" PACKAGE_DIR="$PACKAGE_DIR" SRC_DIR="$SRC_DIR" INCLUDE_DIR="$INCLUDE_DIR" CONFIG_DIR_SRC="$CONFIG_DIR_SRC" SCRIPTS_DIR="$SCRIPTS_DIR" DEPLOYMENT_DIR="$DEPLOYMENT_DIR" sh "$ROOT/scripts/mk.sh" dev-deps)
			echo "Please run 'make lint' again after installation"
			exit 1
		fi
	fi
}

cmd_security_scan() {
	if [ "$PLATFORM" = windows ]; then
		echo "Security scanning tools not yet implemented for Windows"
		echo "Consider using the build script: scripts\\build-windows.bat --deps"
		return 0
	fi
	echo "Running security scan..."
	if command -v bandit >/dev/null 2>&1; then
		echo "Running bandit security scan..."
		bandit -r "$SRC_DIR"
		if command -v semgrep >/dev/null 2>&1; then
			echo "Running semgrep security scan..."
			semgrep --config=auto "$SRC_DIR"
		else
			echo "Note: semgrep not found. Skipping semgrep scan."
			echo "To install semgrep: pip3 install semgrep"
		fi
		echo "Security scan completed successfully"
	else
		echo "Security scanning tools not found. Installing development tools..."
		(cd "$ROOT" && env ROOT="$ROOT" MAKE="$MAKE" PROJECT_NAME="$PROJECT_NAME" VERSION="$VERSION" BUILD_DIR="$BUILD_DIR" DIST_DIR="$DIST_DIR" PACKAGE_DIR="$PACKAGE_DIR" SRC_DIR="$SRC_DIR" INCLUDE_DIR="$INCLUDE_DIR" CONFIG_DIR_SRC="$CONFIG_DIR_SRC" SCRIPTS_DIR="$SCRIPTS_DIR" DEPLOYMENT_DIR="$DEPLOYMENT_DIR" sh "$ROOT/scripts/mk.sh" dev-deps)
		echo "Please run 'make security-scan' again after installation"
		exit 1
	fi
}

cmd_deps() {
	case "$PLATFORM" in
	macos)
		echo "Installing dependencies on macOS..."
		run_root port install openssl jsoncpp cmake
		;;
	linux)
		echo "Installing dependencies on Linux..."
		run_root apt-get update
		run_root apt-get install -y build-essential cmake libssl-dev libjsoncpp-dev
		;;
	freebsd)
		echo "Installing dependencies on FreeBSD..."
		run_root pkg update
		run_root pkg install -y cmake jsoncpp openssl googletest
		;;
	windows)
		echo "Installing dependencies on Windows..."
		echo "Please run: scripts\\build-windows.bat --deps"
		echo "This will install vcpkg and required dependencies"
		;;
	*)
		echo "Unknown platform for deps"
		;;
	esac
}

cmd_dev_deps() {
	case "$PLATFORM" in
	macos)
		echo "Installing development tools on macOS..."
		echo "Installing available packages from MacPorts..."
		run_root port install cppcheck bandit
		echo "Note: semgrep is optional and may fail to install from MacPorts."
		echo "You can install it manually with: pip3 install semgrep"
		echo "Note: clang-format is not available in MacPorts."
		echo "To install clang-format, you can:"
		echo "  1. Use Homebrew: brew install clang-format"
		echo "  2. Install Xcode Command Line Tools: xcode-select --install"
		echo "  3. Install manually from LLVM releases"
		;;
	linux)
		echo "Installing development tools on Linux..."
		run_root apt-get update
		run_root apt-get install -y clang-format cppcheck python3-pip
		pip3 install bandit semgrep
		;;
	freebsd)
		echo "Installing development tools on FreeBSD..."
		run_root pkg update
		run_root pkg install -y cppcheck python3
		PYPIP=$(python3 -c 'import sys; print("py%d%d-pip" % sys.version_info[:2])' 2>/dev/null || true)
		[ -n "$PYPIP" ] && run_root pkg install -y "$PYPIP" || true
		if ! python3 -m pip --version >/dev/null 2>&1; then
			python3 -m ensurepip --user 2>/dev/null || {
				echo "Install pip for your Python (e.g. pkg install py311-pip as root, or sudo/doas pkg install py311-pip)."
				exit 1
			}
		fi
		for L in llvm20 llvm19 llvm18 llvm17 llvm16 llvm15; do
			run_root pkg install -y "$L" && break
		done || echo "Warning: could not pkg install any of llvm20..llvm15; clang-format may be missing."
		if ! command -v clang-format >/dev/null 2>&1; then
			for d in /usr/local/llvm*/bin/clang-format; do
				if [ -f "$d" ]; then
					echo "Linking $d -> /usr/local/bin/clang-format"
					run_root ln -sf "$d" /usr/local/bin/clang-format
					break
				fi
			done
		fi
		python3 -m pip install --user bandit
		python3 -m pip install --user semgrep || echo "Note: semgrep was not installed via pip (FreeBSD: maturin/Rust wheels often fail)."
		;;
	windows)
		echo "Installing development tools on Windows..."
		echo "Please run: scripts\\build-windows.bat --dev-deps"
		echo "This will install clang-format, cppcheck, and other development tools"
		;;
	*)
		echo "Unknown platform for dev-deps"
		;;
	esac
}

cmd_service_install() {
	case "$PLATFORM" in
	macos)
		echo "Installing service on macOS..."
		if [ -f "$DEPLOYMENT_DIR/launchd/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root cp "$DEPLOYMENT_DIR/launchd/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" /Library/LaunchDaemons/
			run_root launchctl load "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
			echo "Service installed and started successfully"
		else
			echo "Service file not found at $DEPLOYMENT_DIR/launchd/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
			echo "Please create the service file first"
			exit 1
		fi
		;;
	linux)
		echo "Installing service on Linux..."
		if [ -f "$DEPLOYMENT_DIR/systemd/${PROJECT_NAME}.service" ]; then
			run_root cp "$DEPLOYMENT_DIR/systemd/${PROJECT_NAME}.service" /etc/systemd/system/
			run_root systemctl daemon-reload
			run_root systemctl enable "$PROJECT_NAME"
			run_root systemctl start "$PROJECT_NAME"
			echo "Service installed and started successfully"
		else
			echo "Service file not found at $DEPLOYMENT_DIR/systemd/${PROJECT_NAME}.service"
			echo "Please create the service file first"
			exit 1
		fi
		;;
	freebsd)
		echo "Installing service on FreeBSD..."
		if [ -f "$DEPLOYMENT_DIR/freebsd/${PROJECT_NAME}" ]; then
			run_root cp "$DEPLOYMENT_DIR/freebsd/${PROJECT_NAME}" "/usr/local/etc/rc.d/${PROJECT_NAME}"
			run_root chmod 0555 "/usr/local/etc/rc.d/${PROJECT_NAME}"
			run_root sysrc simple_dhcpd_enable=YES
			run_root service "$PROJECT_NAME" start
			echo "Service installed and started successfully"
		else
			echo "Service file not found at $DEPLOYMENT_DIR/freebsd/${PROJECT_NAME}"
			echo "Please create the service file first"
			exit 1
		fi
		;;
	windows)
		echo "Installing service on Windows..."
		if [ -f scripts/build-windows.bat ]; then
			scripts/build-windows.bat --service
		else
			echo "Windows build script not found. Please install service manually:"
			echo "  sc create simple-dhcpd binPath= \"${INSTALL_PREFIX}\\bin\\${PROJECT_NAME}.exe\""
			echo "  sc start simple-dhcpd"
		fi
		;;
	*)
		echo "Service install not supported on this platform"
		;;
	esac
}

cmd_service_status() {
	case "$PLATFORM" in
	macos)
		echo "Checking service status on macOS..."
		if launchctl list | grep -q "$PROJECT_NAME"; then
			echo "Service is running:"
			launchctl list | grep "$PROJECT_NAME"
		else
			echo "Service is not running"
		fi
		;;
	linux)
		echo "Checking service status on Linux..."
		if systemctl is-active --quiet "$PROJECT_NAME"; then
			echo "Service is running:"
			run_root systemctl status "$PROJECT_NAME" --no-pager -l
		else
			echo "Service is not running"
			if systemctl is-enabled --quiet "$PROJECT_NAME" 2>/dev/null; then
				echo "Service is enabled but not running"
			else
				echo "Service is not enabled"
			fi
		fi
		;;
	freebsd)
		echo "Checking service status on FreeBSD..."
		service "$PROJECT_NAME" status || true
		;;
	windows)
		echo "Checking service status on Windows..."
		sc query simple-dhcpd
		;;
	*)
		echo "Service status not implemented for this platform"
		;;
	esac
}

cmd_service_start() {
	case "$PLATFORM" in
	macos)
		echo "Starting service on macOS..."
		if [ -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root launchctl start "com.${PROJECT_NAME}.${PROJECT_NAME}"
			echo "Service started successfully"
		else
			echo "Service not installed. Run 'make service-install' first"
		fi
		;;
	linux)
		echo "Starting service on Linux..."
		if [ -f "/etc/systemd/system/${PROJECT_NAME}.service" ]; then
			run_root systemctl start "$PROJECT_NAME"
			echo "Service started successfully"
		else
			echo "Service not installed. Run 'make service-install' first"
		fi
		;;
	freebsd)
		echo "Starting service on FreeBSD..."
		run_root service "$PROJECT_NAME" start || run_root service "$PROJECT_NAME" onestart
		;;
	windows)
		echo "Starting service on Windows..."
		sc start simple-dhcpd
		;;
	*)
		echo "Service start not implemented for this platform"
		;;
	esac
}

cmd_service_stop() {
	case "$PLATFORM" in
	macos)
		echo "Stopping service on macOS..."
		if [ -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root launchctl stop "com.${PROJECT_NAME}.${PROJECT_NAME}"
			echo "Service stopped successfully"
		else
			echo "Service not installed"
		fi
		;;
	linux)
		echo "Stopping service on Linux..."
		if [ -f "/etc/systemd/system/${PROJECT_NAME}.service" ]; then
			run_root systemctl stop "$PROJECT_NAME"
			echo "Service stopped successfully"
		else
			echo "Service not installed"
		fi
		;;
	freebsd)
		echo "Stopping service on FreeBSD..."
		run_root service "$PROJECT_NAME" stop || true
		;;
	windows)
		echo "Stopping service on Windows..."
		sc stop simple-dhcpd
		;;
	*)
		echo "Service stop not implemented for this platform"
		;;
	esac
}

cmd_service_restart() {
	case "$PLATFORM" in
	macos)
		echo "Restarting service on macOS..."
		if [ -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root launchctl stop "com.${PROJECT_NAME}.${PROJECT_NAME}"
			sleep 2
			run_root launchctl start "com.${PROJECT_NAME}.${PROJECT_NAME}"
			echo "Service restarted successfully"
		else
			echo "Service not installed. Run 'make service-install' first"
		fi
		;;
	linux)
		echo "Restarting service on Linux..."
		if [ -f "/etc/systemd/system/${PROJECT_NAME}.service" ]; then
			run_root systemctl restart "$PROJECT_NAME"
			echo "Service restarted successfully"
		else
			echo "Service not installed. Run 'make service-install' first"
		fi
		;;
	freebsd)
		echo "Restarting service on FreeBSD..."
		run_root service "$PROJECT_NAME" restart || {
			run_root service "$PROJECT_NAME" stop
			sleep 2
			run_root service "$PROJECT_NAME" start
		}
		;;
	windows)
		echo "Restarting service on Windows..."
		sc stop simple-dhcpd
		sleep 2 2>/dev/null || true
		sc start simple-dhcpd
		;;
	*)
		echo "Service restart not implemented for this platform"
		;;
	esac
}

cmd_service_enable() {
	case "$PLATFORM" in
	macos)
		echo "Enabling service on macOS..."
		if [ -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root launchctl load -w "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
			echo "Service enabled successfully"
		else
			echo "Service not installed. Run 'make service-install' first"
		fi
		;;
	linux)
		echo "Enabling service on Linux..."
		if [ -f "/etc/systemd/system/${PROJECT_NAME}.service" ]; then
			run_root systemctl enable "$PROJECT_NAME"
			echo "Service enabled successfully"
		else
			echo "Service not installed. Run 'make service-install' first"
		fi
		;;
	freebsd)
		echo "Enabling service on FreeBSD..."
		run_root sysrc simple_dhcpd_enable=YES
		;;
	windows)
		echo "Enabling service on Windows..."
		sc config simple-dhcpd start= auto
		;;
	*)
		echo "Service enable not implemented for this platform"
		;;
	esac
}

cmd_service_disable() {
	case "$PLATFORM" in
	macos)
		echo "Disabling service on macOS..."
		if [ -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root launchctl unload "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
			echo "Service disabled successfully"
		else
			echo "Service not installed"
		fi
		;;
	linux)
		echo "Disabling service on Linux..."
		if [ -f "/etc/systemd/system/${PROJECT_NAME}.service" ]; then
			run_root systemctl disable "$PROJECT_NAME"
			echo "Service disabled successfully"
		else
			echo "Service not installed"
		fi
		;;
	freebsd)
		echo "Disabling service on FreeBSD..."
		run_root service "$PROJECT_NAME" stop 2>/dev/null || true
		run_root sysrc -x simple_dhcpd_enable 2>/dev/null || run_root sysrc simple_dhcpd_enable=NO
		;;
	windows)
		echo "Disabling service on Windows..."
		sc config simple-dhcpd start= disabled
		;;
	*)
		echo "Service disable not implemented for this platform"
		;;
	esac
}

cmd_service_uninstall() {
	case "$PLATFORM" in
	macos)
		echo "Uninstalling service on macOS..."
		if [ -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist" ]; then
			run_root launchctl unload "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
			run_root rm -f "/Library/LaunchDaemons/com.${PROJECT_NAME}.${PROJECT_NAME}.plist"
			echo "Service uninstalled successfully"
		else
			echo "Service not found"
		fi
		;;
	linux)
		echo "Uninstalling service on Linux..."
		if [ -f "/etc/systemd/system/${PROJECT_NAME}.service" ]; then
			run_root systemctl stop "$PROJECT_NAME"
			run_root systemctl disable "$PROJECT_NAME"
			run_root rm -f "/etc/systemd/system/${PROJECT_NAME}.service"
			run_root systemctl daemon-reload
			echo "Service uninstalled successfully"
		else
			echo "Service not found"
		fi
		;;
	freebsd)
		echo "Uninstalling service on FreeBSD..."
		if [ -f "/usr/local/etc/rc.d/${PROJECT_NAME}" ]; then
			run_root service "$PROJECT_NAME" stop 2>/dev/null || true
			run_root sysrc -x simple_dhcpd_enable 2>/dev/null || run_root sysrc simple_dhcpd_enable=NO
			run_root rm -f "/usr/local/etc/rc.d/${PROJECT_NAME}"
			echo "Service uninstalled successfully"
		else
			echo "Service not found"
		fi
		;;
	windows)
		echo "Uninstalling service on Windows..."
		if sc query simple-dhcpd >/dev/null 2>&1; then
			sc stop simple-dhcpd
			sc delete simple-dhcpd
			echo "Service uninstalled successfully"
		else
			echo "Service not found"
		fi
		;;
	*)
		echo "Service uninstall not implemented for this platform"
		;;
	esac
}

cmd_config_install() {
	if [ "$PLATFORM" = windows ]; then
		$MKDIR "$CONFIG_DIR"
		$CP "${CONFIG_DIR_SRC}\\*" "$CONFIG_DIR\\"
	else
		run_root mkdir -p "$CONFIG_DIR"
		run_root cp -r "${CONFIG_DIR_SRC}"/* "$CONFIG_DIR/"
		run_root chown -R root:root "$CONFIG_DIR"
		run_root chmod -R 644 "$CONFIG_DIR"
		run_root find "$CONFIG_DIR" -type d -exec chmod 755 {} \;
	fi
}

cmd_config_backup() {
	if [ "$PLATFORM" = windows ]; then
		$MKDIR "$DIST_DIR/config-backup"
		$CP "$CONFIG_DIR_SRC" "$DIST_DIR/config-backup/"
		powershell -Command "Compress-Archive -Path '${DIST_DIR}\\config-backup' -DestinationPath '${DIST_DIR}\\config-backup-${VERSION}.zip' -Force"
	else
		$MKDIR "$DIST_DIR/config-backup"
		$CP "$CONFIG_DIR_SRC" "$DIST_DIR/config-backup/"
		tar -czf "$DIST_DIR/config-backup-${VERSION}.tar.gz" -C "$DIST_DIR" config-backup
	fi
}

cmd_log_rotate() {
	case "$PLATFORM" in
	linux)
		run_root cp "$DEPLOYMENT_DIR/logrotate.d/${PROJECT_NAME}" /etc/logrotate.d/
		run_root chmod 644 "/etc/logrotate.d/${PROJECT_NAME}"
		;;
	freebsd)
		echo "Installing newsyslog fragment on FreeBSD..."
		if [ -f "$DEPLOYMENT_DIR/freebsd/${PROJECT_NAME}.newsyslog" ]; then
			run_root install -d /usr/local/etc/newsyslog.conf.d
			run_root install -m 644 "$DEPLOYMENT_DIR/freebsd/${PROJECT_NAME}.newsyslog" "/usr/local/etc/newsyslog.conf.d/${PROJECT_NAME}.conf"
			echo "Installed /usr/local/etc/newsyslog.conf.d/${PROJECT_NAME}.conf"
		else
			echo "No newsyslog template at $DEPLOYMENT_DIR/freebsd/${PROJECT_NAME}.newsyslog"
			exit 1
		fi
		;;
	windows)
		echo "Log rotation on Windows is handled by the Windows Event Log system"
		echo "Configure in Event Viewer or use PowerShell commands"
		;;
	*)
		echo "Log rotation not implemented for this platform"
		;;
	esac
}

cmd_backup() {
	if [ "$PLATFORM" = windows ]; then
		$MKDIR "$DIST_DIR/backup"
		powershell -Command "Compress-Archive -Path '${CONFIG_DIR_SRC}', '${DEPLOYMENT_DIR}', '${SRC_DIR}', '${INCLUDE_DIR}', 'CMakeLists.txt', 'Makefile', 'README.md', 'LICENSE' -DestinationPath '${DIST_DIR}\\${PROJECT_NAME}-backup-${VERSION}.zip' -Force"
	else
		$MKDIR "$DIST_DIR/backup"
		tar -czf "$DIST_DIR/${PROJECT_NAME}-backup-${VERSION}.tar.gz" \
			"$CONFIG_DIR_SRC" "$DEPLOYMENT_DIR" "$SRC_DIR" "$INCLUDE_DIR" \
			CMakeLists.txt Makefile README.md LICENSE
	fi
}

cmd_restore() {
	if [ "$PLATFORM" = windows ]; then
		echo "Restoring from backup..."
		if [ -f "$DIST_DIR\\${PROJECT_NAME}-backup-${VERSION}.zip" ]; then
			powershell -Command "Expand-Archive -Path '${DIST_DIR}\\${PROJECT_NAME}-backup-${VERSION}.zip' -DestinationPath '.' -Force"
			echo "Restore completed"
		else
			echo "No backup found at $DIST_DIR\\${PROJECT_NAME}-backup-${VERSION}.zip"
		fi
	else
		echo "Restoring from backup..."
		if [ -f "$DIST_DIR/${PROJECT_NAME}-backup-${VERSION}.tar.gz" ]; then
			tar -xzf "$DIST_DIR/${PROJECT_NAME}-backup-${VERSION}.tar.gz"
			echo "Restore completed"
		else
			echo "No backup found at $DIST_DIR/${PROJECT_NAME}-backup-${VERSION}.tar.gz"
		fi
	fi
}

cmd_distclean() {
	cmd_clean
	$RM "$DIST_DIR" 2>/dev/null || true
	$RM "$PACKAGE_DIR" 2>/dev/null || true
	if command -v find >/dev/null 2>&1; then
		find . -name "*.o" -delete
		find . -name "*.a" -delete
		find . -name "*.so" -delete
		find . -name "*.dylib" -delete
		find . -name "*.exe" -delete
		find . -name "*.dll" -delete
		find . -name "*.obj" -delete
		find . -name "*.pdb" -delete
	fi
}

cmd_package_source() {
	if [ "$PLATFORM" = windows ]; then
		echo "Creating source packages..."
		$MKDIR "$DIST_DIR"
		echo "Creating ZIP source package..."
		powershell -Command "Compress-Archive -Path '${SRC_DIR}', '${INCLUDE_DIR}', 'CMakeLists.txt', 'Makefile', 'README.md', 'LICENSE', 'deployment', 'config', 'scripts' -DestinationPath '${DIST_DIR}\\${PROJECT_NAME}-${VERSION}-src.zip' -Force"
		echo "Creating TAR.GZ source package..."
		tar -czf "$DIST_DIR/${PROJECT_NAME}-${VERSION}-src.tar.gz" \
			"$SRC_DIR" "$INCLUDE_DIR" CMakeLists.txt Makefile README.md LICENSE deployment config scripts
	else
		echo "Creating source packages..."
		$MKDIR "$DIST_DIR"
		echo "Creating TAR.GZ source package..."
		tar -czf "$DIST_DIR/${PROJECT_NAME}-${VERSION}-src.tar.gz" \
			"$SRC_DIR" "$INCLUDE_DIR" CMakeLists.txt Makefile README.md LICENSE deployment config scripts
		echo "Creating ZIP source package..."
		zip -r "$DIST_DIR/${PROJECT_NAME}-${VERSION}-src.zip" \
			"$SRC_DIR" "$INCLUDE_DIR" CMakeLists.txt Makefile README.md LICENSE deployment config scripts
	fi
	echo "Source packages created: TAR.GZ and ZIP"
}

cmd_package_deb() {
	if [ "$PLATFORM" != linux ]; then
		echo "DEB packages are only supported on Linux"
		return 0
	fi
	echo "Building DEB package..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G DEB)
	mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.deb "$DIST_DIR"/
	echo "DEB package created: $DIST_DIR/${PROJECT_NAME}-${VERSION}-*.deb"
}

cmd_package_rpm() {
	if [ "$PLATFORM" != linux ]; then
		echo "RPM packages are only supported on Linux"
		return 0
	fi
	echo "Building RPM package..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G RPM)
	mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.rpm "$DIST_DIR"/
	echo "RPM package created: $DIST_DIR/${PROJECT_NAME}-${VERSION}-*.rpm"
}

cmd_package_msi() {
	if [ "$PLATFORM" != windows ]; then
		echo "MSI packages are only supported on Windows"
		return 0
	fi
	echo "Building MSI package..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G WIX)
	$CP "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.msi "$DIST_DIR"/
	echo "MSI package created: $DIST_DIR/${PROJECT_NAME}-${VERSION}-*.msi"
}

cmd_package_exe() {
	if [ "$PLATFORM" != windows ]; then
		echo "EXE packages are only supported on Windows"
		return 0
	fi
	echo "Building EXE package..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G ZIP)
	$CP "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.zip "$DIST_DIR"/
	echo "EXE package created: $DIST_DIR/${PROJECT_NAME}-${VERSION}-*.zip"
}

cmd_package_dmg() {
	if [ "$PLATFORM" != macos ]; then
		echo "DMG packages are only supported on macOS"
		return 0
	fi
	echo "Building DMG package..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G DragNDrop)
	mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.dmg "$DIST_DIR"/
	echo "DMG package created: $DIST_DIR/${PROJECT_NAME}-${VERSION}-*.dmg"
}

cmd_package_pkg() {
	if [ "$PLATFORM" != macos ]; then
		echo "PKG packages are only supported on macOS"
		return 0
	fi
	echo "Building PKG package..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G productbuild)
	mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.pkg "$DIST_DIR"/
	echo "PKG package created: $DIST_DIR/${PROJECT_NAME}-${VERSION}-*.pkg"
}

cmd_package_tgz() {
	if [ "$PLATFORM" != freebsd ]; then
		echo "package-tgz is intended for FreeBSD (PLATFORM=freebsd)"
		return 0
	fi
	echo "Building TGZ package (CPack)..."
	$MKDIR "$DIST_DIR"
	(cd "$BUILD_DIR" && cpack -G TGZ)
	mv "$BUILD_DIR"/${PROJECT_NAME}-${VERSION}-*.tar.gz "$DIST_DIR"/ 2>/dev/null || true
	echo "TGZ package created under $DIST_DIR/"
}

cmd_help_all() {
	echo "A lightweight and secure DHCP server daemon - All Available Targets"
	echo "=============================================="
	echo ""
	echo "Essential targets:"
	echo "  all              - Build the project (default)"
	echo "  build            - Build using CMake"
	echo "  clean            - Clean build files"
	echo "  install          - Install the project"
	echo "  uninstall        - Uninstall the project"
	echo "  test             - Run tests"
	echo "  test-verbose     - Run tests with verbose output"
	echo "  rebuild          - Clean and rebuild"
	echo ""
	echo "Package targets:"
	echo "  package          - Build platform-specific packages"
	echo "  package-source   - Create source code packages (TAR.GZ + ZIP)"
	echo "  package-all      - Build all package formats"
	echo "  package-info     - Show package information"
	case "$PLATFORM" in
	macos) echo "  package-dmg      - Build DMG package (macOS only)"; echo "  package-pkg      - Build PKG package (macOS only)" ;;
	linux) echo "  package-rpm      - Build RPM package (Linux only)"; echo "  package-deb      - Build DEB package (Linux only)" ;;
	freebsd) echo "  package-tgz      - Build TGZ package (FreeBSD / CPack)" ;;
	windows) echo "  package-msi      - Build MSI package (Windows only)"; echo "  package-zip      - Build ZIP package (Windows only)" ;;
	esac
	echo ""
	echo "Development targets:"
	echo "  dev-build        - Build in debug mode"
	echo "  dev-test         - Run tests in debug mode"
	echo "  debug            - Build with debug information"
	echo "  release          - Build with optimization"
	echo "  sanitize         - Build with sanitizers"
	echo "  docs             - Build documentation"
	echo "  analyze          - Run static analysis"
	echo ""
	echo "Static binary targets:"
	echo "  static-build     - Build static binary (self-contained)"
	echo "  static-test      - Run tests on static binary"
	echo "  static-package   - Create platform-specific static binary package"
	echo "  static-zip       - Create static binary ZIP package"
	echo "  static-all       - Create all static binary formats"
	echo "  coverage         - Generate coverage report"
	echo "  format           - Format source code"
	echo "  check-style      - Check code style"
	echo "  lint             - Run linting tools"
	echo "  security-scan    - Run security scanning tools"
	echo ""
	echo "Dependency management:"
	echo "  deps             - Install dependencies"
	echo "  dev-deps         - Install development tools"
	echo "  runtime-deps     - Runtime dependencies"
	echo ""
	echo "Service management:"
	echo "  service-install  - Install system service"
	echo "  service-uninstall- Uninstall system service"
	echo "  service-status   - Check service status"
	echo "  service-start    - Start service"
	echo "  service-stop     - Stop service"
	echo "  service-restart  - Restart service"
	echo "  service-enable   - Enable service"
	echo "  service-disable  - Disable service"
	echo ""
	echo "Docker targets:"
	echo "  docker-build     - Build Docker image"
	echo "  docker-run       - Run Docker container"
	echo "  docker-stop      - Stop Docker container"
	echo ""
	echo "Configuration management:"
	echo "  config-install   - Install configuration files"
	echo "  config-backup    - Backup configuration"
	echo "  log-rotate       - Install log rotation"
	echo "  backup           - Create full backup"
	echo "  restore          - Restore from backup"
	echo ""
	echo "Cleanup targets:"
	echo "  distclean        - Clean all generated files"
	echo ""
	echo "Legacy targets:"
	echo "  start            - Start service (legacy)"
	echo "  stop             - Stop service (legacy)"
	echo "  restart          - Restart service (legacy)"
	echo "  status           - Check service status (legacy)"
}

cmd_package_info() {
	echo "Package Information for $PROJECT_NAME $VERSION"
	echo "=============================================="
	echo "Platform: $PLATFORM"
	echo "Build directory: $BUILD_DIR"
	echo "Distribution directory: $DIST_DIR"
	echo ""
	echo "Available package formats:"
	case "$PLATFORM" in
	linux)
		echo "  - DEB (Debian/Ubuntu)"
		echo "  - RPM (Red Hat/CentOS/Fedora)"
		echo "  - TAR.GZ (Source)"
		echo "  - ZIP (Source)"
		;;
	macos)
		echo "  - DMG (macOS Disk Image)"
		echo "  - PKG (macOS Installer)"
		echo "  - TAR.GZ (Source)"
		echo "  - ZIP (Source)"
		;;
	freebsd)
		echo "  - TGZ (CPack tarball binary)"
		echo "  - PKG (native CPack FREEBSD, when supported by CMake)"
		echo "  - TAR.GZ (Source)"
		echo "  - ZIP (Source)"
		;;
	windows)
		echo "  - MSI (Windows Installer)"
		echo "  - ZIP (Windows Executable + Source)"
		echo "  - TAR.GZ (Source)"
		;;
	*)
		echo "  (platform-specific formats depend on PLATFORM)"
		;;
	esac
	echo ""
	echo "To create packages:"
	echo "  make package          - Create platform-specific packages"
	echo "  make package-source   - Create source packages"
	echo "  make package-all      - Create all packages"
	echo "  make package-deb      - Create DEB package (Linux only)"
	echo "  make package-rpm      - Create RPM package (Linux only)"
	echo "  make package-msi      - Create MSI package (Windows only)"
	echo "  make package-dmg      - Create DMG package (macOS only)"
	echo "  make package-pkg      - Create PKG package (macOS only)"
	echo "  make package-tgz      - Create TGZ package (FreeBSD only)"
}

run_target() {
	t=$1
	shift
	case "$t" in
	mkdir-build-dir) cmd_mkdir_build_dir ;;
	build) cmd_build ;;
	clean) cmd_clean ;;
	install) cmd_install ;;
	uninstall) cmd_uninstall ;;
	test) cmd_test ;;
	package) cmd_package ;;
	dev-build) cmd_dev_build ;;
	dev-test) cmd_dev_test ;;
	static-build) cmd_static_build ;;
	static-test) cmd_static_test ;;
	static-package) cmd_static_package ;;
	static-zip) cmd_static_zip ;;
	format) cmd_format ;;
	check-style) cmd_check_style ;;
	lint) cmd_lint ;;
	security-scan) cmd_security_scan ;;
	deps) cmd_deps ;;
	dev-deps) cmd_dev_deps ;;
	service-install) cmd_service_install ;;
	service-status) cmd_service_status ;;
	service-start) cmd_service_start ;;
	service-stop) cmd_service_stop ;;
	service-restart) cmd_service_restart ;;
	service-enable) cmd_service_enable ;;
	service-disable) cmd_service_disable ;;
	service-uninstall) cmd_service_uninstall ;;
	config-install) cmd_config_install ;;
	config-backup) cmd_config_backup ;;
	log-rotate) cmd_log_rotate ;;
	backup) cmd_backup ;;
	restore) cmd_restore ;;
	distclean) cmd_distclean ;;
	package-source) cmd_package_source ;;
	package-deb) cmd_package_deb ;;
	package-rpm) cmd_package_rpm ;;
	package-msi) cmd_package_msi ;;
	package-exe) cmd_package_exe ;;
	package-dmg) cmd_package_dmg ;;
	package-pkg) cmd_package_pkg ;;
	package-tgz) cmd_package_tgz ;;
	package-info) cmd_package_info ;;
	help-all) cmd_help_all ;;
	*) echo "mk.sh: unknown target: $t" >&2; exit 2 ;;
	esac
}
