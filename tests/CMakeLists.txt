# Test configuration for Simple DHCP Daemon
cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(GTest_FOUND)
    # Include Google Test
    include(GoogleTest)

    # Create test executable using Google Test
    add_executable(simple_dhcpd_tests
        test_simple_dhcpd.cpp
        test_security.cpp
        test_network.cpp
        test_config.cpp
        test_integration.cpp
        test_performance.cpp
        test_load.cpp
    )

    # Link libraries
    target_link_libraries(simple_dhcpd_tests
        GTest::GTest
        GTest::Main
        Threads::Threads
        simple-dhcpd_lib
    )

    # Add JSON linking if enabled
    if(ENABLE_JSON)
        if(ENABLE_STATIC_LINKING)
            find_library(JSONCPP_STATIC_LIB jsoncpp PATHS /opt/local/lib /usr/local/lib /usr/lib)
            if(JSONCPP_STATIC_LIB)
                target_link_libraries(simple_dhcpd_tests ${JSONCPP_STATIC_LIB})
            else()
                target_link_libraries(simple_dhcpd_tests ${JSONCPP_LIBRARIES})
            endif()
        else()
            target_link_libraries(simple_dhcpd_tests ${JSONCPP_LIBRARIES})
        endif()
        target_include_directories(simple_dhcpd_tests PRIVATE ${JSONCPP_INCLUDE_DIRS})
        target_compile_options(simple_dhcpd_tests PRIVATE ${JSONCPP_CFLAGS_OTHER})
        target_link_directories(simple_dhcpd_tests PRIVATE ${JSONCPP_LIBRARY_DIRS})
    endif()

    # Include directories
    target_include_directories(simple_dhcpd_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )

    # Discover tests
    gtest_discover_tests(simple_dhcpd_tests)

    message(STATUS "Google Test found - tests enabled")

else()
    # Fallback to basic test framework
    message(STATUS "Google Test not found - using basic test framework")

    # Create basic test executable
    add_executable(simple_dhcpd_tests
        test_simple_dhcpd.cpp
        test_network.cpp
        test_config.cpp
        test_integration.cpp
        test_performance.cpp
        test_load.cpp
    )

    # Link with required libraries
    target_link_libraries(simple_dhcpd_tests
        Threads::Threads
        simple-dhcpd_lib
    )

    # Add JSON linking if enabled
    if(ENABLE_JSON)
        if(ENABLE_STATIC_LINKING)
            find_library(JSONCPP_STATIC_LIB jsoncpp PATHS /opt/local/lib /usr/local/lib /usr/lib)
            if(JSONCPP_STATIC_LIB)
                target_link_libraries(simple_dhcpd_tests ${JSONCPP_STATIC_LIB})
            else()
                target_link_libraries(simple_dhcpd_tests ${JSONCPP_LIBRARIES})
            endif()
        else()
            target_link_libraries(simple_dhcpd_tests ${JSONCPP_LIBRARIES})
        endif()
        target_include_directories(simple_dhcpd_tests PRIVATE ${JSONCPP_INCLUDE_DIRS})
        target_compile_options(simple_dhcpd_tests PRIVATE ${JSONCPP_CFLAGS_OTHER})
        target_link_directories(simple_dhcpd_tests PRIVATE ${JSONCPP_LIBRARY_DIRS})
    endif()

    # Include directories
    target_include_directories(simple_dhcpd_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )

    # Add basic test
    add_test(NAME basic_tests COMMAND simple_dhcpd_tests)
endif()
