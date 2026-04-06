# CMakeLists.txt template for SimpleDaemons projects
# A lightweight and secure DHCP server daemon
# Copyright 2024 SimpleDaemons

cmake_minimum_required(VERSION 3.16)
project(simple-dhcpd VERSION 0.6.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Version selection
set(BUILD_VERSION "production" CACHE STRING "Version to build: production, enterprise, datacenter")
set_property(CACHE BUILD_VERSION PROPERTY STRINGS production enterprise datacenter)

# Platform detection (for status messages / packaging labels)
if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(PLATFORM_NAME "FreeBSD")
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    set(PLATFORM_NAME "OpenBSD")
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
endif()

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_PACKAGING "Enable package generation" ON)
option(ENABLE_SSL "Enable SSL/TLS support" ON)
option(ENABLE_JSON "Enable JSON support" ON)
option(ENABLE_STATIC_LINKING "Enable static linking for self-contained binaries" OFF)

# Find required packages
find_package(Threads REQUIRED)

if(ENABLE_SSL)
    find_package(OpenSSL REQUIRED)
endif()

if(ENABLE_JSON)
    # Prefer pkg-config when present (clean -L/-I on e.g. FreeBSD /usr/local). Otherwise fall back so
    # minimal images without pkgconf still configure (find_path/find_library).
    set(SIMPLE_DHCPD_JSONCPP_TARGET "")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(JSONCPP QUIET IMPORTED_TARGET jsoncpp)
        if(JSONCPP_FOUND)
            set(SIMPLE_DHCPD_JSONCPP_TARGET PkgConfig::JSONCPP)
        endif()
    endif()
    if(NOT SIMPLE_DHCPD_JSONCPP_TARGET)
        find_package(JsonCpp CONFIG QUIET)
        if(NOT JsonCpp_FOUND)
            find_package(jsoncpp CONFIG QUIET)
        endif()
        if(TARGET JsonCpp::JsonCpp)
            set(SIMPLE_DHCPD_JSONCPP_TARGET JsonCpp::JsonCpp)
        elseif(TARGET jsoncpp_lib)
            set(SIMPLE_DHCPD_JSONCPP_TARGET jsoncpp_lib)
        elseif(TARGET jsoncpp_static)
            set(SIMPLE_DHCPD_JSONCPP_TARGET jsoncpp_static)
        endif()
    endif()
    if(NOT SIMPLE_DHCPD_JSONCPP_TARGET)
        find_path(JSONCPP_INCLUDE_DIR NAMES json/json.h PATH_SUFFIXES jsoncpp)
        find_library(JSONCPP_LIBRARY NAMES jsoncpp libjsoncpp)
        if(JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY)
            add_library(simple_dhcpd_jsoncpp UNKNOWN IMPORTED)
            set_target_properties(simple_dhcpd_jsoncpp PROPERTIES
                IMPORTED_LOCATION "${JSONCPP_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_INCLUDE_DIR}")
            set(SIMPLE_DHCPD_JSONCPP_TARGET simple_dhcpd_jsoncpp)
        endif()
    endif()
    if(NOT SIMPLE_DHCPD_JSONCPP_TARGET)
        message(FATAL_ERROR
            "ENABLE_JSON is ON but jsoncpp was not found. "
            "Install jsoncpp (and optionally pkgconf for pkg-config), e.g. FreeBSD: pkg install jsoncpp pkgconf; "
            "Alpine: apk add jsoncpp-dev pkgconf; Debian: apt install libjsoncpp-dev pkg-config. "
            "Or configure with -DENABLE_JSON=OFF.")
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core sources (always included - shared by all versions)
set(CORE_SOURCES
    src/core/dhcp/parser.cpp
    src/core/dhcp/server.cpp
    src/core/lease/manager.cpp
    src/core/network/udp_socket.cpp
    src/core/config/manager.cpp
    src/core/options/manager.cpp
    src/core/utils/logger.cpp
)

# Core headers
file(GLOB_RECURSE CORE_HEADERS "include/simple-dhcpd/core/*.hpp")

# Version-specific sources and main
if(BUILD_VERSION STREQUAL "production")
    set(VERSION_SOURCES
        ${CORE_SOURCES}
        src/production/security/manager.cpp
        src/production/features/advanced_manager.cpp
    )
    file(GLOB_RECURSE VERSION_HEADERS
        "include/simple-dhcpd/core/*.hpp"
        "include/simple-dhcpd/production/*.hpp"
    )
    set(VERSION_MAIN main/production.cpp)
    set(PACKAGE_SUFFIX "production")
    set(PROJECT_DESCRIPTION "Simple DHCP Daemon - Production Version")

elseif(BUILD_VERSION STREQUAL "enterprise")
    set(VERSION_SOURCES
        ${CORE_SOURCES}
        src/production/security/manager.cpp
        src/production/features/advanced_manager.cpp
        # Enterprise sources will be added here
        # src/enterprise/ha/failover.cpp
        # src/enterprise/management/web_ui.cpp
        # src/enterprise/integrations/dns.cpp
    )
    file(GLOB_RECURSE VERSION_HEADERS
        "include/simple-dhcpd/core/*.hpp"
        "include/simple-dhcpd/production/*.hpp"
        "include/simple-dhcpd/enterprise/*.hpp"
    )
    set(VERSION_MAIN main/enterprise.cpp)
    set(PACKAGE_SUFFIX "enterprise")
    set(PROJECT_DESCRIPTION "Simple DHCP Daemon - Enterprise Version")

elseif(BUILD_VERSION STREQUAL "datacenter")
    set(VERSION_SOURCES
        ${CORE_SOURCES}
        src/production/security/manager.cpp
        src/production/features/advanced_manager.cpp
        # Enterprise sources (when implemented)
        # Datacenter sources will be added here
        # src/datacenter/cluster/cluster_manager.cpp
        # src/datacenter/multi-site/sync.cpp
    )
    file(GLOB_RECURSE VERSION_HEADERS
        "include/simple-dhcpd/core/*.hpp"
        "include/simple-dhcpd/production/*.hpp"
        "include/simple-dhcpd/enterprise/*.hpp"
        "include/simple-dhcpd/datacenter/*.hpp"
    )
    set(VERSION_MAIN main/datacenter.cpp)
    set(PACKAGE_SUFFIX "datacenter")
    set(PROJECT_DESCRIPTION "Simple DHCP Daemon - Datacenter Version")

else()
    message(FATAL_ERROR "Unknown BUILD_VERSION: ${BUILD_VERSION}. Must be: production, enterprise, or datacenter")
endif()

# Create library
add_library(${PROJECT_NAME}_lib ${VERSION_SOURCES} ${VERSION_HEADERS})

# Create executable
add_executable(${PROJECT_NAME} ${VERSION_MAIN})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib Threads::Threads)
target_link_libraries(${PROJECT_NAME}_lib PRIVATE Threads::Threads)

if(ENABLE_SSL)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(${PROJECT_NAME}_lib PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(ENABLE_JSON)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SIMPLE_DHCPD_JSONCPP_TARGET})
    target_link_libraries(${PROJECT_NAME}_lib PRIVATE ${SIMPLE_DHCPD_JSONCPP_TARGET})
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
    target_compile_options(${PROJECT_NAME}_lib PRIVATE /W4 /WX-)
else()
    # -Wno-unused-private-field is Clang-only; GCC (e.g. CentOS) rejects it.
    set(_SIMPLE_DHCPD_CLANG_PRIV_FIELD "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-unused-private-field>")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter ${_SIMPLE_DHCPD_CLANG_PRIV_FIELD})
    target_compile_options(${PROJECT_NAME}_lib PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter ${_SIMPLE_DHCPD_CLANG_PRIV_FIELD})
    if(ENABLE_STATIC_LINKING)
        # Only apply static linking flags for GCC, not clang
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
            target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
        endif()
    endif()
endif()

# Static linking configuration
if(ENABLE_STATIC_LINKING)
    set(BUILD_SHARED_LIBS OFF)
    if(ENABLE_SSL)
        # Force static linking for OpenSSL
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    endif()
    if(ENABLE_JSON)
        # Force static linking for jsoncpp
        set(JSONCPP_USE_STATIC_LIBS TRUE)
    endif()
endif()

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install configuration files
install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.example"
)

# Install service files
if(UNIX AND NOT APPLE)
    install(FILES deployment/systemd/${PROJECT_NAME}.service
        DESTINATION lib/systemd/system
    )
elseif(APPLE)
    # Relative to CMAKE_INSTALL_PREFIX so CPack/DMG and non-root installs work.
    # Copy into /Library/LaunchDaemons with: make service-install (or sudo cp from share path).
    install(FILES deployment/launchd/com.${PROJECT_NAME}.${PROJECT_NAME}.plist
        DESTINATION share/${PROJECT_NAME}/launchd
    )
endif()

# Tests
if(ENABLE_TESTS)
    enable_testing()
    # Only add tests subdirectory if it exists and has CMakeLists.txt
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(STATUS "Tests directory found but no CMakeLists.txt - skipping tests")
    endif()
endif()

# Package generation
if(ENABLE_PACKAGING)
    include(CPack)

    # Package information
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}")
    set(CPACK_PACKAGE_VENDOR "SimpleDaemons")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

    # Package file names
    if(APPLE)
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_ID}")
        set(CPACK_DMG_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_ID}")
        set(CPACK_PRODUCTBUILD_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_ID}")
    elseif(UNIX)
        # Detect distribution
        if(EXISTS "/etc/os-release")
            file(READ "/etc/os-release" os_release)
            string(REGEX MATCH "ID=([^\n]+)" _ ${os_release})
            set(PLATFORM_ID ${CMAKE_MATCH_1})
        else()
            set(PLATFORM_ID "linux")
        endif()

        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
        # We want: {name}-{version}-{platform}-{arch}.deb (architecture already in CPACK_PACKAGE_FILE_NAME)
        set(CPACK_DEBIAN_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
        set(CPACK_RPM_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
    else()
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-${PLATFORM_NAME}-${PACKAGE_ARCH}")
    endif()

    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PACKAGE_SUFFIX}-${PROJECT_VERSION}-src")

    # Include packaging configuration
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CMakeLists.cpack")
        include("${CMAKE_CURRENT_SOURCE_DIR}/packaging/CMakeLists.cpack")
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== Simple DHCP Daemon Configuration ===")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Build Version: ${BUILD_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  SSL Support: ${ENABLE_SSL}")
message(STATUS "  JSON Support: ${ENABLE_JSON}")
message(STATUS "  Tests: ${ENABLE_TESTS}")
message(STATUS "  Packaging: ${ENABLE_PACKAGING}")
message(STATUS "==========================================")
message(STATUS "")
