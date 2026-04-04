# Test configuration for Simple DHCP Daemon
cmake_minimum_required(VERSION 3.16)

include(FetchContent)

# Prefer system Google Test; fetch from upstream when missing (e.g. FreeBSD pkg not installed).
find_package(GTest QUIET CONFIG)
if(NOT GTest_FOUND)
    find_package(GTest QUIET MODULE)
endif()

set(_simple_dhcpd_gtest_ready FALSE)
if(TARGET GTest::gtest_main AND TARGET GTest::gtest)
    set(_simple_dhcpd_gtest_ready TRUE)
elseif(TARGET GTest::Main AND TARGET GTest::GTest)
    set(_simple_dhcpd_gtest_ready TRUE)
endif()

if(NOT _simple_dhcpd_gtest_ready)
    message(STATUS "Google Test not found via find_package; using FetchContent (set OFFLINE or install googletest to skip)")
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

if(TARGET GTest::gtest_main AND TARGET GTest::gtest)
    set(SIMPLE_DHCPD_GTEST_MAIN GTest::gtest_main)
    set(SIMPLE_DHCPD_GTEST_LIB GTest::gtest)
elseif(TARGET GTest::Main AND TARGET GTest::GTest)
    set(SIMPLE_DHCPD_GTEST_MAIN GTest::Main)
    set(SIMPLE_DHCPD_GTEST_LIB GTest::GTest)
else()
    message(FATAL_ERROR "Google Test targets missing after find_package / FetchContent")
endif()

include(GoogleTest)

add_executable(simple_dhcpd_tests
    test_simple_dhcpd.cpp
    test_security.cpp
    test_network.cpp
    test_config.cpp
    test_integration.cpp
    test_performance.cpp
    test_load.cpp
)

target_link_libraries(simple_dhcpd_tests PRIVATE
    ${SIMPLE_DHCPD_GTEST_MAIN}
    ${SIMPLE_DHCPD_GTEST_LIB}
    Threads::Threads
    simple-dhcpd_lib
)

if(ENABLE_JSON)
    target_link_libraries(simple_dhcpd_tests PRIVATE PkgConfig::JSONCPP)
endif()

target_include_directories(simple_dhcpd_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(simple_dhcpd_tests)

message(STATUS "Google Test enabled for simple_dhcpd_tests")
