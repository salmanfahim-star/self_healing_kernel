#pragma once
#include <string>
#include <vector>
#include <ctime>

// ===== System Configuration Constants =====
const int TOTAL_MEMORY      = 1024;   // simulated memory in bytes
const int MAX_PROCESSES     = 12;
const int HEARTBEAT_TIMEOUT = 6;      // seconds before marking process unresponsive
const int LEAK_THRESHOLD    = 20;     // seconds before flagging memory as leaked
const int MAX_RESTART_COUNT = 3;      // max auto-restarts for a critical process

// ===== Process States =====
enum class ProcessState { NEW, READY, RUNNING, WAITING, TERMINATED, CRASHED };

inline std::string stateToStr(ProcessState s) {
    switch (s) {
        case ProcessState::NEW:        return "NEW";
        case ProcessState::READY:      return "READY";
        case ProcessState::RUNNING:    return "RUNNING";
        case ProcessState::WAITING:    return "WAITING";
        case ProcessState::TERMINATED: return "TERMINATED";
        case ProcessState::CRASHED:    return "CRASHED";
        default:                       return "UNKNOWN";
    }
}

// ===== Log Levels =====
enum class LogLevel { INFO, WARNING, ERR, CRITICAL, HEAL };

// ===== Process Control Block =====
struct PCB {
    int         pid;
    std::string name;
    ProcessState state;
    int         priority;       // 1 (low) to 10 (high)
    int         memory_used;    // bytes allocated
    bool        is_critical;    // if true, auto-restart on crash
    int         restart_count;
    time_t      created_at;
    time_t      last_heartbeat;
    int         health_score;   // 0-100
};

// ===== Memory Block =====
struct MemoryBlock {
    int    block_id;
    int    size;
    int    owner_pid;       // -1 if free
    bool   is_free;
    time_t allocated_at;
    bool   flagged_as_leak;
};

// ===== Resource (for deadlock detection) =====
struct Resource {
    int         id;
    std::string name;
    int         total_instances;
    int         available;
};

// ===== Log Entry =====
struct LogEntry {
    time_t      timestamp;
    LogLevel    level;
    std::string message;
};
