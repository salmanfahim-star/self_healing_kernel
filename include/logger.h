#pragma once
#include "types.h"
#include <vector>
#include <mutex>
#include <string>

class Logger {
public:
    static Logger& getInstance();

    void log(LogLevel level, const std::string& msg);
    void showLogs(int last_n = 20);

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::vector<LogEntry> entries;
    std::mutex            mtx;

    std::string levelStr(LogLevel level);
    std::string levelColor(LogLevel level);
    std::string timeStr(time_t t);
};
