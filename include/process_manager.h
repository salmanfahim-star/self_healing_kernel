#pragma once
#include "types.h"
#include "logger.h"
#include <map>
#include <mutex>
#include <string>

class ProcessManager {
public:
    ProcessManager();

    int  createProcess(const std::string& name, int priority, bool is_critical, int memory_req);
    bool terminateProcess(int pid);
    bool crashProcess(int pid);
    void sendHeartbeat(int pid);

    void checkAndHealProcesses();   // main self-healing function (called by watchdog)

    PCB*              getProcess(int pid);
    std::map<int, PCB>& getProcessTable();

    void displayProcessTable();

private:
    std::map<int, PCB> process_table;
    int                next_pid;
    std::mutex         mtx;
    Logger&            logger;

    void restartHelper(PCB& pcb);   // no-lock internal restart
    void updateHealthScore(PCB& pcb);
};
