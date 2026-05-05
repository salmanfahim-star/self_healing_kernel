#pragma once
#include "types.h"
#include "logger.h"
#include <vector>
#include <mutex>

class MemoryManager {
public:
    explicit MemoryManager(int total_size = TOTAL_MEMORY);

    int  allocate(int pid, int size);       // returns block_id or -1
    bool freeBlock(int block_id);
    bool simulateLeak(int pid, int size);   // fault injection: allocates with old timestamp

    void detectAndFixLeaks();               // self-healing: scan, flag, reclaim
    void defragment();                      // self-healing: merge adjacent free blocks

    void displayMemoryMap();

    int  getUsedMemory() const;
    int  getFreeMemory() const;
    int  getTotalMemory()  const { return total_size; }

    const std::vector<MemoryBlock>& getBlocks() const { return blocks; }

private:
    int                     total_size;
    std::vector<MemoryBlock> blocks;
    int                     next_block_id;
    mutable std::mutex      mtx;
    Logger&                 logger;

    // Internal helpers (no lock — callers must hold mtx)
    int  allocateHelper(int pid, int size);
    void defragHelper();
};
