#include "../include/process_manager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

ProcessManager::ProcessManager()
    : next_pid(1), logger(Logger::getInstance()) {}

// ===== Public API =====

int ProcessManager::createProcess(const std::string& name, int priority,
                                  bool is_critical, int memory_req) {
    std::lock_guard<std::mutex> lock(mtx);

    if ((int)process_table.size() >= MAX_PROCESSES) {
        logger.log(LogLevel::ERR,
            "Cannot create '" + name + "': max process limit reached");
        return -1;
    }

    PCB pcb;
    pcb.pid            = next_pid++;
    pcb.name           = name;
    pcb.state          = ProcessState::RUNNING;
    pcb.priority       = std::max(1, std::min(10, priority));
    pcb.memory_used    = memory_req;
    pcb.is_critical    = is_critical;
    pcb.restart_count  = 0;
    pcb.created_at     = time(nullptr);
    pcb.last_heartbeat = time(nullptr);
    pcb.health_score   = 100;

    process_table[pcb.pid] = pcb;

    std::ostringstream oss;
    oss << "Process created  PID=" << pcb.pid
        << "  Name=" << pcb.name
        << "  Priority=" << pcb.priority
        << "  Critical=" << (pcb.is_critical ? "YES" : "NO")
        << "  Memory=" << memory_req << "B";
    logger.log(LogLevel::INFO, oss.str());

    return pcb.pid;
}

bool ProcessManager::terminateProcess(int pid) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = process_table.find(pid);
    if (it == process_table.end()) return false;

    it->second.state        = ProcessState::TERMINATED;
    it->second.health_score = 0;

    logger.log(LogLevel::INFO,
        "Process terminated  PID=" + std::to_string(pid) +
        "  Name=" + it->second.name);
    return true;
}

bool ProcessManager::crashProcess(int pid) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = process_table.find(pid);
    if (it == process_table.end()) return false;
    if (it->second.state == ProcessState::TERMINATED) return false;

    it->second.state        = ProcessState::CRASHED;
    it->second.health_score = 0;

    logger.log(LogLevel::ERR,
        "FAULT INJECTED: Process CRASHED  PID=" + std::to_string(pid) +
        "  Name=" + it->second.name);
    return true;
}

void ProcessManager::sendHeartbeat(int pid) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = process_table.find(pid);
    if (it != process_table.end() &&
        it->second.state == ProcessState::RUNNING) {
        it->second.last_heartbeat = time(nullptr);
    }
}

// ===== Self-Healing Core =====

void ProcessManager::checkAndHealProcesses() {
    std::lock_guard<std::mutex> lock(mtx);
    time_t now = time(nullptr);

    for (auto& [pid, pcb] : process_table) {
        if (pcb.state == ProcessState::TERMINATED) continue;

        // Watchdog: detect unresponsive processes
        if (pcb.state == ProcessState::RUNNING) {
            double elapsed = difftime(now, pcb.last_heartbeat);
            if (elapsed > HEARTBEAT_TIMEOUT) {
                std::ostringstream oss;
                oss << "Watchdog: PID=" << pid << " Name=" << pcb.name
                    << " unresponsive for " << (int)elapsed
                    << "s  -> marking CRASHED";
                logger.log(LogLevel::CRITICAL, oss.str());
                pcb.state        = ProcessState::CRASHED;
                pcb.health_score = 0;
            }
        }

        // Auto-restart crashed critical processes
        if (pcb.state == ProcessState::CRASHED && pcb.is_critical) {
            restartHelper(pcb);
        }

        updateHealthScore(pcb);
    }
}

// ===== Private Helpers =====

void ProcessManager::restartHelper(PCB& pcb) {
    if (pcb.restart_count >= MAX_RESTART_COUNT) {
        std::ostringstream oss;
        oss << "CRITICAL: PID=" << pcb.pid << " Name=" << pcb.name
            << " exceeded max restarts (" << MAX_RESTART_COUNT
            << ")  -> PERMANENTLY FAILED";
        logger.log(LogLevel::CRITICAL, oss.str());
        pcb.state        = ProcessState::TERMINATED;
        return;
    }

    pcb.state          = ProcessState::RUNNING;
    pcb.restart_count++;
    pcb.last_heartbeat = time(nullptr);
    pcb.health_score   = 75;

    std::ostringstream oss;
    oss << "SELF-HEAL: Auto-restarted critical process  PID=" << pcb.pid
        << "  Name=" << pcb.name
        << "  (restart #" << pcb.restart_count << ")";
    logger.log(LogLevel::HEAL, oss.str());
}

void ProcessManager::updateHealthScore(PCB& pcb) {
    if (pcb.state != ProcessState::RUNNING) return;
    double elapsed = difftime(time(nullptr), pcb.last_heartbeat);
    if (elapsed < 3)
        pcb.health_score = std::min(100, pcb.health_score + 1);
    else
        pcb.health_score = std::max(0, pcb.health_score - (int)elapsed * 5);
}

PCB* ProcessManager::getProcess(int pid) {
    auto it = process_table.find(pid);
    return it != process_table.end() ? &it->second : nullptr;
}

std::map<int, PCB>& ProcessManager::getProcessTable() {
    return process_table;
}

// ===== Display =====

void ProcessManager::displayProcessTable() {
    std::cout << "\n";
    std::cout << std::string(95, '=') << "\n";
    std::cout << "\033[1m"
              << std::left
              << std::setw(6)  << "PID"
              << std::setw(16) << "NAME"
              << std::setw(13) << "STATE"
              << std::setw(10) << "PRIORITY"
              << std::setw(10) << "MEMORY"
              << std::setw(10) << "CRITICAL"
              << std::setw(10) << "RESTARTS"
              << std::setw(12) << "HEALTH"
              << "\033[0m\n";
    std::cout << std::string(95, '-') << "\n";

    for (auto& [pid, pcb] : process_table) {
        std::string color;
        switch (pcb.state) {
            case ProcessState::RUNNING:    color = "\033[32m"; break;
            case ProcessState::CRASHED:    color = "\033[31m"; break;
            case ProcessState::WAITING:    color = "\033[33m"; break;
            case ProcessState::TERMINATED: color = "\033[90m"; break;
            default:                       color = "\033[0m";  break;
        }

        std::cout << color
                  << std::left
                  << std::setw(6)  << pcb.pid
                  << std::setw(16) << pcb.name
                  << std::setw(13) << stateToStr(pcb.state)
                  << std::setw(10) << pcb.priority
                  << std::setw(10) << (std::to_string(pcb.memory_used) + "B")
                  << std::setw(10) << (pcb.is_critical ? "YES" : "NO")
                  << std::setw(10) << pcb.restart_count
                  << std::setw(12) << (std::to_string(pcb.health_score) + "%")
                  << "\033[0m\n";
    }
    std::cout << std::string(95, '=') << "\n\n";
}
