#include "../include/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

std::string Logger::timeStr(time_t t) {
    struct tm* tm_info = localtime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
    return std::string(buf);
}

std::string Logger::levelStr(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "[INFO]    ";
        case LogLevel::WARNING:  return "[WARNING] ";
        case LogLevel::ERR:      return "[ERROR]   ";
        case LogLevel::CRITICAL: return "[CRITICAL]";
        case LogLevel::HEAL:     return "[HEAL]    ";
        default:                 return "[LOG]     ";
    }
}

std::string Logger::levelColor(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "\033[36m";   // cyan
        case LogLevel::WARNING:  return "\033[33m";   // yellow
        case LogLevel::ERR:      return "\033[31m";   // red
        case LogLevel::CRITICAL: return "\033[35m";   // magenta
        case LogLevel::HEAL:     return "\033[32m";   // green
        default:                 return "\033[0m";
    }
}

void Logger::log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    LogEntry entry;
    entry.timestamp = time(nullptr);
    entry.level     = level;
    entry.message   = msg;
    entries.push_back(entry);

    std::cout << levelColor(level)
              << timeStr(entry.timestamp) << "  "
              << levelStr(level) << "  "
              << msg
              << "\033[0m\n";
}

void Logger::showLogs(int last_n) {
    std::lock_guard<std::mutex> lock(mtx);
    int start = std::max(0, (int)entries.size() - last_n);
    std::cout << "\n\033[1m====== SYSTEM LOG (last " << last_n << " entries) ======\033[0m\n";
    for (int i = start; i < (int)entries.size(); i++) {
        std::cout << levelColor(entries[i].level)
                  << timeStr(entries[i].timestamp) << "  "
                  << levelStr(entries[i].level) << "  "
                  << entries[i].message
                  << "\033[0m\n";
    }
    std::cout << std::string(48, '=') << "\n\n";
}
