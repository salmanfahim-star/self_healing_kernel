#pragma once
#include "logger.h"
#include "process_manager.h"
#include <thread>
#include <atomic>

class Watchdog {
public:
    explicit Watchdog(ProcessManager& pm, int interval_sec = 4);
    ~Watchdog();

    void start();
    void stop();
    bool isRunning() const { return running.load(); }

private:
    ProcessManager&   process_manager;
    Logger&           logger;
    std::thread       monitor_thread;
    std::atomic<bool> running;
    int               check_interval;

    void monitorLoop();
};
