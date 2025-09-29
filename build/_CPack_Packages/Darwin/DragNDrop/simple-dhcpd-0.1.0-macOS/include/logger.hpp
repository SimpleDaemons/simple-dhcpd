/**
 * @file logger.hpp
 * @brief Logging infrastructure
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_DHCPD_LOGGER_HPP
#define SIMPLE_DHCPD_LOGGER_HPP

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace simple_dhcpd {

/**
 * @brief Log levels
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

/**
 * @brief Logger class
 */
class Logger {
public:
    /**
     * @brief Constructor
     * @param log_file Path to log file
     * @param level Minimum log level
     */
    explicit Logger(const std::string& log_file = "", LogLevel level = LogLevel::INFO);
    
    /**
     * @brief Destructor
     */
    ~Logger();
    
    /**
     * @brief Set log level
     * @param level New log level
     */
    void set_level(LogLevel level);
    
    /**
     * @brief Get current log level
     * @return Current log level
     */
    LogLevel get_level() const;
    
    /**
     * @brief Log a message
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Log debug message
     * @param message Message to log
     */
    void debug(const std::string& message);
    
    /**
     * @brief Log info message
     * @param message Message to log
     */
    void info(const std::string& message);
    
    /**
     * @brief Log warning message
     * @param message Message to log
     */
    void warn(const std::string& message);
    
    /**
     * @brief Log error message
     * @param message Message to log
     */
    void error(const std::string& message);
    
    /**
     * @brief Log fatal message
     * @param message Message to log
     */
    void fatal(const std::string& message);
    
    /**
     * @brief Enable/disable console output
     * @param enable True to enable console output
     */
    void set_console_output(bool enable);
    
    /**
     * @brief Enable/disable file output
     * @param enable True to enable file output
     */
    void set_file_output(bool enable);
    
    /**
     * @brief Flush log output
     */
    void flush();

private:
    std::string log_file_;
    LogLevel level_;
    bool console_output_;
    bool file_output_;
    std::unique_ptr<std::ofstream> file_stream_;
    mutable std::mutex mutex_;
    
    /**
     * @brief Get current timestamp string
     * @return Timestamp string
     */
    std::string get_timestamp() const;
    
    /**
     * @brief Get log level string
     * @param level Log level
     * @return Log level string
     */
    std::string get_level_string(LogLevel level) const;
    
    /**
     * @brief Write log message
     * @param level Log level
     * @param message Message to write
     */
    void write_log(LogLevel level, const std::string& message);
};

/**
 * @brief Global logger instance
 */
extern std::unique_ptr<Logger> g_logger;

/**
 * @brief Initialize global logger
 * @param log_file Path to log file
 * @param level Log level
 */
void init_logger(const std::string& log_file = "", LogLevel level = LogLevel::INFO);

/**
 * @brief Get global logger instance
 * @return Logger instance
 */
Logger& get_logger();

/**
 * @brief Log macros for convenience
 */
#define LOG_DEBUG(msg) do { \
    if (simple_dhcpd::g_logger && simple_dhcpd::g_logger->get_level() <= simple_dhcpd::LogLevel::DEBUG) { \
        std::ostringstream oss; \
        oss << msg; \
        simple_dhcpd::g_logger->debug(oss.str()); \
    } \
} while(0)

#define LOG_INFO(msg) do { \
    if (simple_dhcpd::g_logger && simple_dhcpd::g_logger->get_level() <= simple_dhcpd::LogLevel::INFO) { \
        std::ostringstream oss; \
        oss << msg; \
        simple_dhcpd::g_logger->info(oss.str()); \
    } \
} while(0)

#define LOG_WARN(msg) do { \
    if (simple_dhcpd::g_logger && simple_dhcpd::g_logger->get_level() <= simple_dhcpd::LogLevel::WARN) { \
        std::ostringstream oss; \
        oss << msg; \
        simple_dhcpd::g_logger->warn(oss.str()); \
    } \
} while(0)

#define LOG_ERROR(msg) do { \
    if (simple_dhcpd::g_logger && simple_dhcpd::g_logger->get_level() <= simple_dhcpd::LogLevel::ERROR) { \
        std::ostringstream oss; \
        oss << msg; \
        simple_dhcpd::g_logger->error(oss.str()); \
    } \
} while(0)

#define LOG_FATAL(msg) do { \
    if (simple_dhcpd::g_logger && simple_dhcpd::g_logger->get_level() <= simple_dhcpd::LogLevel::FATAL) { \
        std::ostringstream oss; \
        oss << msg; \
        simple_dhcpd::g_logger->fatal(oss.str()); \
    } \
} while(0)

} // namespace simple_dhcpd

#endif // SIMPLE_DHCPD_LOGGER_HPP
