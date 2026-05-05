#pragma once
#include "logger.h"
#include "process_manager.h"
#include "memory_manager.h"
#include "deadlock_detector.h"
#include "watchdog.h"
#include "health_monitor.h"

class MiniOS {
public:
    MiniOS();
    ~MiniOS();

    void boot();
    void run();

    // Process ops
    int  createProcess(const std::string& name, int priority, bool is_critical, int memory_req);
    void crashProcess(int pid);
    void terminateProcess(int pid);
    void sendHeartbeat(int pid);

    // Memory ops
    void allocateMemory(int pid, int size);
    void freeMemory(int block_id);
    void leakMemory(int pid, int size);

    // Resource ops
    void requestResource(int pid, int resource_id);
    void releaseResource(int pid, int resource_id);

    // Self-healing
    void runHealCycle();
    void detectAndResolveDeadlocks();

    // Display
    void showDashboard();
    void showProcessTable();
    void showMemoryMap();
    void showResourceTable();
    void showLogs();

    // Demo
    void runDemo();

private:
    Logger&           logger;
    ProcessManager    process_manager;
    MemoryManager     memory_manager;
    DeadlockDetector  deadlock_detector;
    Watchdog          watchdog;
    HealthMonitor     health_monitor;
    bool              running;

    void setupResources();
    void shutdown();
    void printMenu();
    void handleInput(int choice);
};
