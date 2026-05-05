#pragma once
#include "types.h"
#include "logger.h"
#include <map>
#include <vector>
#include <mutex>

class DeadlockDetector {
public:
    DeadlockDetector();

    void addResource(const std::string& name, int instances);
    bool requestResource(int pid, int resource_id);
    bool releaseResource(int pid, int resource_id);
    void releaseAllResources(int pid);  // called when process crashes/terminates

    bool detectDeadlock(std::vector<int>& deadlocked_pids);
    void resolveDeadlock(std::map<int, PCB>& process_table);

    void displayResourceTable();
    int  getResourceCount() const;

private:
    std::map<int, Resource>         resources;
    std::map<int, std::vector<int>> held;        // pid -> resource ids held
    std::map<int, int>              waiting_for; // pid -> resource id waiting for
    int                             next_resource_id;
    std::mutex                      mtx;
    Logger&                         logger;

    void buildWaitGraph(std::map<int, std::vector<int>>& graph);
    bool dfsCycle(int node,
                  std::map<int, bool>& visited,
                  std::map<int, bool>& in_stack,
                  std::vector<int>&    path,
                  std::map<int, std::vector<int>>& graph);
};
