#pragma once
#include "process_manager.h"
#include "memory_manager.h"
#include "deadlock_detector.h"

class HealthMonitor {
public:
    HealthMonitor(ProcessManager& pm, MemoryManager& mm, DeadlockDetector& dd);

    double calculateSystemHealth();
    void   displayDashboard();

private:
    ProcessManager&   pm;
    MemoryManager&    mm;
    DeadlockDetector& dd;
    Logger&           logger;
};
