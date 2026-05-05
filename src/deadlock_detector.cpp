#include "../include/deadlock_detector.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

DeadlockDetector::DeadlockDetector()
    : next_resource_id(1), logger(Logger::getInstance()) {}

void DeadlockDetector::addResource(const std::string& name, int instances) {
    std::lock_guard<std::mutex> lock(mtx);
    Resource r;
    r.id              = next_resource_id++;
    r.name            = name;
    r.total_instances = instances;
    r.available       = instances;
    resources[r.id]   = r;

    std::ostringstream oss;
    oss << "Resource created  ID=" << r.id
        << "  Name=" << name
        << "  Instances=" << instances;
    logger.log(LogLevel::INFO, oss.str());
}

bool DeadlockDetector::requestResource(int pid, int resource_id) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = resources.find(resource_id);
    if (it == resources.end()) {
        logger.log(LogLevel::ERR,
            "Resource ID=" + std::to_string(resource_id) + " not found");
        return false;
    }

    if (it->second.available > 0) {
        it->second.available--;
        held[pid].push_back(resource_id);

        std::ostringstream oss;
        oss << "PID=" << pid << " acquired [" << it->second.name
            << "]  (remaining=" << it->second.available << ")";
        logger.log(LogLevel::INFO, oss.str());
        return true;
    } else {
        waiting_for[pid] = resource_id;
        std::ostringstream oss;
        oss << "PID=" << pid << " BLOCKED waiting for ["
            << it->second.name << "]  (none available)";
        logger.log(LogLevel::WARNING, oss.str());
        return false;
    }
}

bool DeadlockDetector::releaseResource(int pid, int resource_id) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& h   = held[pid];
    auto  pos = std::find(h.begin(), h.end(), resource_id);
    if (pos == h.end()) return false;

    h.erase(pos);
    resources[resource_id].available++;

    std::ostringstream oss;
    oss << "PID=" << pid << " released ["
        << resources[resource_id].name << "]";
    logger.log(LogLevel::INFO, oss.str());

    // Grant to a waiting process if any
    for (auto it = waiting_for.begin(); it != waiting_for.end(); ++it) {
        if (it->second == resource_id && resources[resource_id].available > 0) {
            resources[resource_id].available--;
            held[it->first].push_back(resource_id);
            std::ostringstream o2;
            o2 << "Resource [" << resources[resource_id].name
               << "] granted to waiting PID=" << it->first;
            logger.log(LogLevel::INFO, o2.str());
            waiting_for.erase(it);
            break;
        }
    }
    return true;
}

void DeadlockDetector::releaseAllResources(int pid) {
    std::lock_guard<std::mutex> lock(mtx);
    for (int rid : held[pid])
        resources[rid].available++;
    held.erase(pid);
    waiting_for.erase(pid);

    logger.log(LogLevel::INFO,
        "All resources released for PID=" + std::to_string(pid));
}

// ===== Deadlock Detection (DFS cycle detection on wait-for graph) =====

void DeadlockDetector::buildWaitGraph(std::map<int, std::vector<int>>& graph) {
    for (auto& [wpid, wrid] : waiting_for) {
        for (auto& [hpid, hlist] : held) {
            if (std::find(hlist.begin(), hlist.end(), wrid) != hlist.end()) {
                graph[wpid].push_back(hpid);
            }
        }
    }
}

bool DeadlockDetector::dfsCycle(int node,
                                 std::map<int, bool>& visited,
                                 std::map<int, bool>& in_stack,
                                 std::vector<int>& path,
                                 std::map<int, std::vector<int>>& graph) {
    visited[node]  = true;
    in_stack[node] = true;
    path.push_back(node);

    for (int neighbor : graph[node]) {
        if (!visited[neighbor]) {
            if (dfsCycle(neighbor, visited, in_stack, path, graph))
                return true;
        } else if (in_stack[neighbor]) {
            path.push_back(neighbor);  // close the cycle
            return true;
        }
    }

    in_stack[node] = false;
    path.pop_back();
    return false;
}

bool DeadlockDetector::detectDeadlock(std::vector<int>& deadlocked_pids) {
    std::lock_guard<std::mutex> lock(mtx);

    std::map<int, std::vector<int>> wait_graph;
    buildWaitGraph(wait_graph);
    if (wait_graph.empty()) return false;

    std::map<int, bool> visited, in_stack;
    std::vector<int>    path;

    for (auto& [pid, _] : wait_graph) {
        if (!visited[pid]) {
            path.clear();
            if (dfsCycle(pid, visited, in_stack, path, wait_graph)) {
                deadlocked_pids = path;
                std::ostringstream oss;
                oss << "DEADLOCK DETECTED  Cycle involves PIDs:";
                for (int p : deadlocked_pids) oss << " " << p;
                logger.log(LogLevel::CRITICAL, oss.str());
                return true;
            }
        }
    }
    return false;
}

void DeadlockDetector::resolveDeadlock(std::map<int, PCB>& process_table) {
    std::vector<int> cycle;
    if (!detectDeadlock(cycle)) {
        logger.log(LogLevel::INFO, "Deadlock check: No deadlock found");
        return;
    }

    // Kill lowest-priority process in the cycle (victim selection)
    int victim_pid      = cycle[0];
    int lowest_priority = 11;
    for (int pid : cycle) {
        auto it = process_table.find(pid);
        if (it != process_table.end() && it->second.priority < lowest_priority) {
            lowest_priority = it->second.priority;
            victim_pid      = pid;
        }
    }

    std::ostringstream oss;
    oss << "SELF-HEAL: Deadlock resolved  Victim PID=" << victim_pid
        << "  (lowest priority=" << lowest_priority << " in cycle)";
    logger.log(LogLevel::HEAL, oss.str());

    releaseAllResources(victim_pid);

    auto it = process_table.find(victim_pid);
    if (it != process_table.end()) {
        it->second.state        = ProcessState::TERMINATED;
        it->second.health_score = 0;
        logger.log(LogLevel::HEAL,
            "Victim PID=" + std::to_string(victim_pid) +
            " terminated  Deadlock resolved  Resources redistributed");
    }
}

int DeadlockDetector::getResourceCount() const {
    return (int)resources.size();
}

void DeadlockDetector::displayResourceTable() {
    std::cout << "\n\033[1m===== RESOURCE ALLOCATION TABLE =====\033[0m\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(16) << "NAME"
              << std::setw(10) << "TOTAL"
              << std::setw(12) << "AVAILABLE"
              << "\n";
    std::cout << std::string(44, '-') << "\n";

    for (auto& [id, r] : resources) {
        std::string color = r.available > 0 ? "\033[32m" : "\033[31m";
        std::cout << color
                  << std::left
                  << std::setw(6)  << r.id
                  << std::setw(16) << r.name
                  << std::setw(10) << r.total_instances
                  << std::setw(12) << r.available
                  << "\033[0m\n";
    }
    std::cout << std::string(44, '=') << "\n\n";
}
