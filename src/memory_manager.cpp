#include "../include/memory_manager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

MemoryManager::MemoryManager(int total_size)
    : total_size(total_size), next_block_id(1), logger(Logger::getInstance()) {
    // Start with one big free block
    MemoryBlock init;
    init.block_id        = 0;
    init.size            = total_size;
    init.owner_pid       = -1;
    init.is_free         = true;
    init.allocated_at    = 0;
    init.flagged_as_leak = false;
    blocks.push_back(init);
}

// ===== Internal helpers (callers hold mtx) =====

int MemoryManager::allocateHelper(int pid, int size) {
    for (size_t i = 0; i < blocks.size(); i++) {
        if (!blocks[i].is_free || blocks[i].size < size) continue;

        if (blocks[i].size > size) {
            // Split: insert remainder block after current
            MemoryBlock rem;
            rem.block_id        = next_block_id++;
            rem.size            = blocks[i].size - size;
            rem.owner_pid       = -1;
            rem.is_free         = true;
            rem.allocated_at    = 0;
            rem.flagged_as_leak = false;
            blocks[i].size      = size;
            blocks.insert(blocks.begin() + (int)i + 1, rem);
        }

        blocks[i].is_free         = false;
        blocks[i].owner_pid       = pid;
        blocks[i].allocated_at    = time(nullptr);
        blocks[i].flagged_as_leak = false;
        return blocks[i].block_id;
    }
    return -1;
}

void MemoryManager::defragHelper() {
    bool merged = true;
    while (merged) {
        merged = false;
        for (size_t i = 0; i + 1 < blocks.size(); i++) {
            if (blocks[i].is_free && blocks[i + 1].is_free) {
                blocks[i].size += blocks[i + 1].size;
                blocks.erase(blocks.begin() + (int)i + 1);
                merged = true;
                break;
            }
        }
    }
}

// ===== Public API =====

int MemoryManager::allocate(int pid, int size) {
    std::lock_guard<std::mutex> lock(mtx);
    int bid = allocateHelper(pid, size);
    if (bid >= 0) {
        std::ostringstream oss;
        oss << "Memory allocated  " << size << "B  PID=" << pid
            << "  BlockID=" << bid;
        logger.log(LogLevel::INFO, oss.str());
    } else {
        std::ostringstream oss;
        oss << "Memory allocation FAILED  Requested=" << size
            << "B  Available=" << (total_size - getUsedMemory()) << "B";
        logger.log(LogLevel::ERR, oss.str());
    }
    return bid;
}

bool MemoryManager::freeBlock(int block_id) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& b : blocks) {
        if (b.block_id == block_id && !b.is_free) {
            int pid        = b.owner_pid;
            b.is_free      = true;
            b.owner_pid    = -1;
            b.flagged_as_leak = false;
            logger.log(LogLevel::INFO,
                "Memory freed  BlockID=" + std::to_string(block_id) +
                "  (was PID=" + std::to_string(pid) + ")");
            return true;
        }
    }
    return false;
}

bool MemoryManager::simulateLeak(int pid, int size) {
    std::lock_guard<std::mutex> lock(mtx);
    int bid = allocateHelper(pid, size);
    if (bid < 0) return false;

    for (auto& b : blocks) {
        if (b.block_id == bid) {
            // Backdate allocation to look like a very old leak
            b.allocated_at = time(nullptr) - LEAK_THRESHOLD - 15;
            break;
        }
    }
    std::ostringstream oss;
    oss << "FAULT INJECTED: Memory leak simulated  PID=" << pid
        << "  Size=" << size << "B  BlockID=" << bid;
    logger.log(LogLevel::WARNING, oss.str());
    return true;
}

// ===== Self-Healing =====

void MemoryManager::detectAndFixLeaks() {
    std::lock_guard<std::mutex> lock(mtx);
    time_t now       = time(nullptr);
    bool   any_fixed = false;

    for (auto& b : blocks) {
        if (b.is_free || b.owner_pid == -1) continue;

        double age = difftime(now, b.allocated_at);

        if (age > LEAK_THRESHOLD && !b.flagged_as_leak) {
            b.flagged_as_leak = true;
            std::ostringstream oss;
            oss << "LEAK DETECTED  BlockID=" << b.block_id
                << "  PID=" << b.owner_pid
                << "  Size=" << b.size << "B"
                << "  Age=" << (int)age << "s";
            logger.log(LogLevel::WARNING, oss.str());
        }

        if (b.flagged_as_leak) {
            std::ostringstream oss;
            oss << "SELF-HEAL: Reclaiming leaked memory  BlockID=" << b.block_id
                << "  " << b.size << "B  from PID=" << b.owner_pid;
            logger.log(LogLevel::HEAL, oss.str());
            b.is_free         = true;
            b.owner_pid       = -1;
            b.flagged_as_leak = false;
            any_fixed         = true;
        }
    }

    if (!any_fixed)
        logger.log(LogLevel::INFO, "Memory scan complete  No leaks detected");

    defragHelper();
}

void MemoryManager::defragment() {
    std::lock_guard<std::mutex> lock(mtx);
    size_t before = blocks.size();
    defragHelper();
    size_t after = blocks.size();
    std::ostringstream oss;
    oss << "SELF-HEAL: Memory defragmented  Fragments: "
        << before << " -> " << after;
    logger.log(LogLevel::HEAL, oss.str());
}

// ===== Accessors =====

int MemoryManager::getUsedMemory() const {
    int used = 0;
    for (const auto& b : blocks)
        if (!b.is_free) used += b.size;
    return used;
}

int MemoryManager::getFreeMemory() const {
    return total_size - getUsedMemory();
}

// ===== Display =====

void MemoryManager::displayMemoryMap() {
    std::cout << "\n\033[1m===== MEMORY MAP  (Total: " << total_size << "B) =====\033[0m\n";
    std::cout << std::left
              << std::setw(10) << "BlockID"
              << std::setw(10) << "Size"
              << std::setw(12) << "Owner PID"
              << std::setw(10) << "Status"
              << std::setw(12) << "Leak Flag"
              << "\n";
    std::cout << std::string(54, '-') << "\n";

    for (const auto& b : blocks) {
        std::string color = b.is_free ? "\033[32m" : "\033[36m";
        if (b.flagged_as_leak) color = "\033[31m";

        std::cout << color
                  << std::left
                  << std::setw(10) << b.block_id
                  << std::setw(9)  << b.size << "B"
                  << std::setw(12) << (b.is_free ? "-" : "PID " + std::to_string(b.owner_pid))
                  << std::setw(10) << (b.is_free ? "FREE" : "USED")
                  << std::setw(12) << (b.flagged_as_leak ? "*** LEAK" : "OK")
                  << "\033[0m\n";
    }

    // Visual usage bar
    int used      = getUsedMemory();
    int bar_width = 50;
    int filled    = total_size > 0 ? used * bar_width / total_size : 0;
    std::cout << "\n  [";
    for (int i = 0; i < bar_width; i++)
        std::cout << (i < filled ? "\033[31m#\033[0m" : "\033[32m.\033[0m");
    std::cout << "]  " << used << "/" << total_size << "B  ("
              << (total_size > 0 ? used * 100 / total_size : 0) << "% used)\n";
    std::cout << std::string(54, '=') << "\n\n";
}
