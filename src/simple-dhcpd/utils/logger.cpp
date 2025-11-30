/**
 * @file utils/logger.cpp
 * @brief Logging infrastructure implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-dhcpd/utils/logger.hpp"
#include <iostream>
#include <ctime>
#include <cstring>

namespace simple_dhcpd {

// Global logger instance
std::unique_ptr<Logger> g_logger = nullptr;

Logger::Logger(const std::string& log_file, LogLevel level)
    : log_file_(log_file), level_(level), console_output_(true), file_output_(!log_file.empty()) {
    if (file_output_) {
        file_stream_ = std::make_unique<std::ofstream>(log_file_, std::ios::app);
        if (!file_stream_->is_open()) {
            file_output_ = false;
            console_output_ = true;
        }
    }
}

Logger::~Logger() {
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->close();
    }
}

void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

LogLevel Logger::get_level() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return level_;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < level_) {
        return;
    }
    
    write_log(level, message);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

void Logger::set_console_output(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    console_output_ = enable;
}

void Logger::set_file_output(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_output_ = enable;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->flush();
    }
    std::cout.flush();
    std::cerr.flush();
}

std::string Logger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::get_level_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::write_log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string timestamp = get_timestamp();
    std::string level_str = get_level_string(level);
    std::string log_line = "[" + timestamp + "] [" + level_str + "] " + message;
    
    if (console_output_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << log_line << std::endl;
        } else {
            std::cout << log_line << std::endl;
        }
    }
    
    if (file_output_ && file_stream_ && file_stream_->is_open()) {
        *file_stream_ << log_line << std::endl;
        file_stream_->flush();
    }
}

void init_logger(const std::string& log_file, LogLevel level) {
    g_logger = std::make_unique<Logger>(log_file, level);
}

Logger& get_logger() {
    if (!g_logger) {
        init_logger();
    }
    return *g_logger;
}

} // namespace simple_dhcpd
