/**
 * @file main.cpp
 * @brief Main entry point for Simple DHCP Daemon
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-dhcpd/core/server.hpp"
#include "simple-dhcpd/core/utils/logger.hpp"
#include "simple-dhcpd/core/config/manager.hpp"
#include <iostream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

namespace {
    std::unique_ptr<simple_dhcpd::DhcpServer> g_server = nullptr;
    std::atomic<bool> g_running{true};
}

/**
 * @brief Signal handler
 * @param signal Signal number
 */
void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
            g_running = false;
            if (g_server) {
                g_server->stop();
            }
            break;
            
        case SIGHUP:
            std::cout << "Received SIGHUP, reloading configuration..." << std::endl;
            if (g_server) {
                try {
                    g_server->reload_config();
                    std::cout << "Configuration reloaded successfully" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Failed to reload configuration: " << e.what() << std::endl;
                }
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief Print usage information
 * @param program_name Program name
 */
void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Options:\n"
              << "  -c, --config FILE    Configuration file path\n"
              << "  -d, --daemon         Run as daemon\n"
              << "  -p, --pid-file FILE  PID file path\n"
              << "  -l, --log-file FILE  Log file path\n"
              << "  -v, --verbose        Verbose logging\n"
              << "  -h, --help           Show this help message\n"
              << "  -V, --version        Show version information\n"
              << std::endl;
}

/**
 * @brief Print version information
 */
void print_version() {
    std::cout << "Simple DHCP Daemon v0.3.0\n"
              << "Copyright 2024 SimpleDaemons\n"
              << "License: Apache-2.0\n"
              << std::endl;
}

/**
 * @brief Create daemon process
 */
void create_daemon() {
    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Failed to fork daemon process" << std::endl;
        exit(1);
    }
    
    if (pid > 0) {
        // Parent process
        exit(0);
    }
    
    // Child process
    setsid();
    
    // Fork again to ensure we're not a session leader
    pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork daemon process" << std::endl;
        exit(1);
    }
    
    if (pid > 0) {
        // Parent process
        exit(0);
    }
    
    // Change to root directory
    chdir("/");
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect to /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
}

/**
 * @brief Write PID file
 * @param pid_file PID file path
 */
void write_pid_file(const std::string& pid_file) {
    std::ofstream file(pid_file);
    if (file.is_open()) {
        file << getpid() << std::endl;
        file.close();
    }
}

/**
 * @brief Remove PID file
 * @param pid_file PID file path
 */
void remove_pid_file(const std::string& pid_file) {
    unlink(pid_file.c_str());
}

/**
 * @brief Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    std::string config_file = "/etc/simple-dhcpd/simple-dhcpd.conf";
    std::string pid_file = "/var/run/simple-dhcpd.pid";
    std::string log_file = "/var/log/simple-dhcpd.log";
    bool daemon_mode = false;
    bool verbose = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: --config requires a file path" << std::endl;
                return 1;
            }
        } else if (arg == "-d" || arg == "--daemon") {
            daemon_mode = true;
        } else if (arg == "-p" || arg == "--pid-file") {
            if (i + 1 < argc) {
                pid_file = argv[++i];
            } else {
                std::cerr << "Error: --pid-file requires a file path" << std::endl;
                return 1;
            }
        } else if (arg == "-l" || arg == "--log-file") {
            if (i + 1 < argc) {
                log_file = argv[++i];
            } else {
                std::cerr << "Error: --log-file requires a file path" << std::endl;
                return 1;
            }
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-V" || arg == "--version") {
            print_version();
            return 0;
        } else {
            std::cerr << "Error: Unknown option " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    try {
        // Initialize logger
        simple_dhcpd::init_logger(log_file, verbose ? simple_dhcpd::LogLevel::DEBUG : simple_dhcpd::LogLevel::INFO);
        
        // Create daemon if requested
        if (daemon_mode) {
            create_daemon();
        }
        
        // Write PID file
        if (daemon_mode) {
            write_pid_file(pid_file);
        }
        
        // Set up signal handlers
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGHUP, signal_handler);
        
        // Create and initialize server
        g_server = std::make_unique<simple_dhcpd::DhcpServer>(config_file);
        g_server->initialize();
        
        // Start server
        g_server->start();
        
        // Main loop
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Cleanup
        if (g_server) {
            g_server->stop();
        }
        
        if (daemon_mode) {
            remove_pid_file(pid_file);
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        
        if (daemon_mode) {
            remove_pid_file(pid_file);
        }
        
        return 1;
    }
}
