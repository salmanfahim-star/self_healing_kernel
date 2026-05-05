#include "../include/watchdog.h"
#include <iostream>
#include <chrono>
#include <thread>

Watchdog::Watchdog(ProcessManager& pm, int interval_sec)
    : process_manager(pm), logger(Logger::getInstance()),
      running(false), check_interval(interval_sec) {}

Watchdog::~Watchdog() {
    stop();
}

void Watchdog::start() {
    if (running.load()) return;
    running = true;
    monitor_thread = std::thread(&Watchdog::monitorLoop, this);
    logger.log(LogLevel::INFO, "Watchdog started  interval=" + std::to_string(check_interval) + "s");
}

void Watchdog::stop() {
    if (!running.load()) return;
    running = false;
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }
    logger.log(LogLevel::INFO, "Watchdog stopped");
}

void Watchdog::monitorLoop() {
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(check_interval));
        if (running.load()) {
            process_manager.checkAndHealProcesses();
        }
    }
}
