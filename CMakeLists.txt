# CMakeLists.txt template for SimpleDaemons projects
# A lightweight and secure DHCP server daemon
# Copyright 2024 SimpleDaemons

cmake_minimum_required(VERSION 3.16)
project(simple-dhcpd VERSION 0.3.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Version selection
set(BUILD_VERSION "production" CACHE STRING "Version to build: production, enterprise, datacenter")
set_property(CACHE BUILD_VERSION PROPERTY STRINGS production enterprise datacenter)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
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
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
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
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib Threads::Threads)
target_link_libraries(${PROJECT_NAME}_lib Threads::Threads)

if(ENABLE_SSL)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(${PROJECT_NAME}_lib OpenSSL::SSL OpenSSL::Crypto)
endif()

if(ENABLE_JSON)
    target_link_libraries(${PROJECT_NAME} ${JSONCPP_LIBRARIES})
    target_link_libraries(${PROJECT_NAME}_lib ${JSONCPP_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${JSONCPP_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME}_lib PRIVATE ${JSONCPP_INCLUDE_DIRS})
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
    target_compile_options(${PROJECT_NAME}_lib PRIVATE /W4 /WX-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-private-field)
    target_compile_options(${PROJECT_NAME}_lib PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-private-field)
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
    install(FILES deployment/launchd/com.${PROJECT_NAME}.${PROJECT_NAME}.plist
        DESTINATION /Library/LaunchDaemons
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
