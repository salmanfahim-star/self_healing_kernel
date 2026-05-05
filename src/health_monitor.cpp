#include "../include/health_monitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

HealthMonitor::HealthMonitor(ProcessManager& pm, MemoryManager& mm, DeadlockDetector& dd)
    : pm(pm), mm(mm), dd(dd), logger(Logger::getInstance()) {}

double HealthMonitor::calculateSystemHealth() {
    auto& proc_table = pm.getProcessTable();

    // Count process states
    int running   = 0;
    int crashed   = 0;
    int waiting   = 0;
    int total_active = 0;

    for (const auto& [_, pcb] : proc_table) {
        if (pcb.state == ProcessState::TERMINATED) continue;
        total_active++;
        if (pcb.state == ProcessState::RUNNING) running++;
        else if (pcb.state == ProcessState::CRASHED) crashed++;
        else if (pcb.state == ProcessState::WAITING) waiting++;
    }

    // Process health: 60% weight
    double process_health = 0.0;
    if (total_active > 0) {
        process_health = (running * 100.0) / total_active;
    }

    // Memory health: 40% weight
    int used = mm.getUsedMemory();
    int total = mm.getTotalMemory();
    double memory_health = 100.0 - (used * 100.0 / total);

    // Composite health
    double system_health = (process_health * 0.6) + (memory_health * 0.4);
    return system_health;
}

void HealthMonitor::displayDashboard() {
    auto& proc_table = pm.getProcessTable();

    int running   = 0;
    int crashed   = 0;
    int waiting   = 0;
    int total_restarts = 0;
    int total_active = 0;

    for (const auto& [_, pcb] : proc_table) {
        if (pcb.state == ProcessState::TERMINATED) continue;
        total_active++;
        if (pcb.state == ProcessState::RUNNING) running++;
        else if (pcb.state == ProcessState::CRASHED) crashed++;
        else if (pcb.state == ProcessState::WAITING) waiting++;
        total_restarts += pcb.restart_count;
    }

    double health = calculateSystemHealth();

    // Determine color
    std::string health_color;
    std::string health_status;
    if (health >= 80) {
        health_color = "\033[32m";      // green
        health_status = "EXCELLENT";
    } else if (health >= 60) {
        health_color = "\033[33m";      // yellow
        health_status = "GOOD";
    } else if (health >= 40) {
        health_color = "\033[33m";      // yellow
        health_status = "FAIR";
    } else {
        health_color = "\033[31m";      // red
        health_status = "CRITICAL";
    }

    // Memory info
    int used = mm.getUsedMemory();
    int total = mm.getTotalMemory();
    int percent = total > 0 ? (used * 100 / total) : 0;

    std::cout << "\n\033[1;36m";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║        SYSTEM HEALTH DASHBOARD         ║\n";
    std::cout << "╚════════════════════════════════════════╝\033[0m\n";

    std::cout << "  Overall Health:  " << health_color
              << std::fixed << std::setprecision(1) << health << "%  "
              << health_status << "\033[0m\n";

    std::cout << "  Process Status:  "
              << "\033[32mRunning: " << running << "\033[0m  "
              << "\033[31mCrashed: " << crashed << "\033[0m  "
              << "\033[33mWaiting: " << waiting << "\033[0m  "
              << "Total Active: " << total_active << "\n";

    std::cout << "  Memory Usage:    " << used << "/" << total << "B  ("
              << percent << "%)\n";

    std::cout << "  Total Restarts:  " << total_restarts << "\n";

    // Health bar
    std::cout << "  Health Bar:      [";
    int bar_width = 35;
    int filled = (int)((health / 100.0) * bar_width);
    for (int i = 0; i < bar_width; i++) {
        if (i < filled)
            std::cout << health_color << "█\033[0m";
        else
            std::cout << "░";
    }
    std::cout << "]\n\n";
}
