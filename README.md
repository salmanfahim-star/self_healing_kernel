# Self-Healing Operating System Kernel (Mini OS)

A sophisticated multi-threaded OS kernel simulation in C++17 that demonstrates core operating system concepts with self-healing and fault recovery mechanisms. This is a 20-mark assignment project implementing process management, memory management, deadlock detection/resolution, and automatic recovery systems.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Architecture & Features](#architecture--features)
3. [Self-Healing Mechanisms](#self-healing-mechanisms)
4. [Project Structure](#project-structure)
5. [Build Instructions](#build-instructions)
6. [Usage Guide](#usage-guide)
8. [Implementation Details](#implementation-details)
9. [System Requirements](#system-requirements)

---

## 🎯 Overview

**Self-Healing OS Kernel** is an educational operating system simulator that showcases advanced OS concepts. It implements a complete system that monitors itself, detects failures, and automatically recovers without manual intervention.

### Key Highlights:
- ✅ **Multi-threaded watchdog monitor** for continuous system health checks
- ✅ **Automatic process restart** when critical processes crash
- ✅ **Memory leak detection & reclamation** with defragmentation
- ✅ **Real-time health dashboard** with system metrics
- ✅ **Comprehensive logging** with color-coded output
- ✅ **Interactive menu system** + automated demo mode
- ✅ **Thread-safe operations** with mutex protection

---
- Process states: NEW, READY, RUNNING, WAITING, TERMINATED, CRASHED
- Critical process marking (auto-restart on crash)
- Heartbeat mechanism to detect unresponsive processes
- Health score tracking (0-100%)

**Self-Healing:**
- Detects unresponsive processes (timeout-based)
- Automatically restarts critical crashed processes
- Tracks restart attempts (max 3 restarts before permanent failure)
- Updates process health dynamically

```
Process Lifecycle:
NEW → READY → RUNNING ←→ WAITING
       ↓                    ↓
    CRASHED → [AUTO-RESTART if CRITICAL]
       ↓
    TERMINATED
```

### 2. **Memory Manager**
Implements buddy-style memory allocation with leak detection.

**Features:**
- Total 1024 bytes simulated memory
- Dynamic block allocation/deallocation
- Memory block splitting and coalescing
- Leak detection (blocks allocated > 20 seconds flagged as leaks)
- Visual memory map with usage tracking

**Self-Healing:**
- Automatic reclamation of flagged leaked memory
- Memory defragmentation (coalesces adjacent free blocks)
- Prevents memory fragmentation

Automatically defragmented → [Block-1: USED(p1)]  [Block-2: USED(p2)]  [Free]
```

### 3. **Deadlock Detector**
Detects and resolves circular wait conditions.

**Features:**
- Resource management (Printers, Scanners, Disk, etc.)
- Request/release operations with tracking
- Wait-for graph construction
- Cycle detection using DFS with recursion stack
- Victim selection based on priority
- Automatic resource redistribution

**Self-Healing:**
- Detects cycles in resource allocation graph
- Selects lowest-priority process as victim
- Terminates victim and reclaims all resources
- Redistributes resources to waiting processes

```
Deadlock Scenario:
P1 holds Printer → waiting for Scanner (held by P2)
P2 holds Scanner → waiting for Printer (held by P1)
           ↓
CYCLE DETECTED → Kill P2 (lower priority) → Resources freed → P1 continues
```

### 4. **Watchdog Timer**
Background thread that continuously monitors system health.

**Features:**
- Configurable check interval (default: 3 seconds)
- Background thread-based monitoring
- Automatic healing triggers
- Non-blocking operations

**Behavior:**
```
Watchdog Loop (every 3 seconds):
├─ Check process heartbeats
├─ Auto-restart crashed critical processes
├─ Update health scores
└─ Log any anomalies
```

### 5. **Health Monitor**
Real-time system health tracking and dashboard.

**Features:**
- Visual health bar
- Process state summary
- Memory usage metrics
- Status classification (EXCELLENT/GOOD/FAIR/CRITICAL)

**Health Calculation:**
```
Health = (Running_Processes/Total_Active × 100) × 0.6 
       + (1 - Memory_Used/Total_Memory) × 100 × 0.4
```

### 6. **Logger (Singleton)**
Thread-safe centralized logging system.

**Log Levels:**
- `[INFO]` - General information (cyan)
- `[WARNING]` - Warnings (yellow)
- `[ERROR]` - Errors (red)
- `[CRITICAL]` - Critical failures (magenta)
- `[HEAL]` - Self-healing actions (green)

**Features:**
- Timestamp on each entry
- Color-coded console output
- Last N entries display
- Thread-safe mutex protection

---

## 🔧 Self-Healing Mechanisms

### **Mechanism 1: Process Crash Recovery**

**Trigger:** Process marked as CRASHED or unresponsive (heartbeat timeout)

**Recovery Steps:**
1. Watchdog detects crash/timeout
2. If process is marked CRITICAL:
   - Auto-restart immediately
   - Increment restart counter
   - Set health score to 75%
   - Log healing action
3. If restart count exceeds limit:
   - Permanently terminate

**Example:**
```
WebServer (CRITICAL) crashes
   ↓
Watchdog detects no heartbeat after 6 seconds
   ↓
[CRITICAL] Watchdog: PID=1 unresponsive... marking CRASHED
   ↓
[HEAL] SELF-HEAL: Auto-restarted critical process PID=1 (restart #1)
   ↓
Process state: CRASHED → RUNNING (health: 75%)
```


1. Scan all memory blocks
2. Check allocation age (>20 seconds = potential leak)
3. Flag old allocations as leaks
4. Reclaim flagged memory immediately
5. Defragment to merge free blocks
6. Update available memory

**Example:**
```
Memory Block allocated 25 seconds ago → not accessed
   ↓
[WARNING] LEAK DETECTED BlockID=5 Age=25s
[HEAL] SELF-HEAL: Reclaiming leaked memory BlockID=5 100B from PID=3
   ↓
Memory freed and available for reuse
   ↓
[HEAL] SELF-HEAL: Memory defragmented Fragments: 5 → 3

### **Mechanism 3: Deadlock Detection & Resolution**

**Trigger:** Manual check or periodic monitoring

**Detection Algorithm (DFS):**
1. Build wait-for graph from resource requests
2. Run depth-first search from each node
3. Track recursion stack during traversal
5. Extract cycle path (deadlocked processes)

**Resolution Steps:**
1. Identify all processes in cycle
2. Select victim (lowest priority process)
3. Terminate victim process
4. Release all victim's resources
5. Redistribute resources to waiting processes
6. Log and continue

**Example:**
```
P1(priority=8) has [Printer], wants [Scanner]
P2(priority=5) has [Scanner], wants [Printer]

   ↓ [Cycle detected: 1→2→1]

   ↓ [Victim selected: P2 (priority 5 < 8)]

   ↓ [P2 terminated, [Scanner] freed]

   ↓ [P1 acquires [Scanner], cycle broken]

   ↓ [HEAL] Deadlock resolved
```

---

## 📁 Project Structure

```
self_healing_kernel/
├── include/                          # Header files
│   ├── types.h                      # System types, PCB, constants
│   ├── logger.h                     # Singleton logger
│   ├── process_manager.h            # Process lifecycle & monitoring
│   ├── memory_manager.h             # Memory allocation & healing
│   ├── deadlock_detector.h          # Resource & deadlock management
│   ├── watchdog.h                   # Background monitoring thread
│   ├── health_monitor.h             # Health tracking & dashboard
│   └── mini_os.h                    # Main OS kernel class
│
├── src/                              # Implementation files
│   ├── logger.cpp
│   ├── process_manager.cpp
│   ├── memory_manager.cpp
│   ├── deadlock_detector.cpp
│   ├── watchdog.cpp
│   ├── health_monitor.cpp
│   └── mini_os.cpp
│
├── main.cpp                          # Entry point
├── Makefile                          # Build configuration
└── README.md                         # This file
```

### File Relationships:
```
main.cpp
   ↓
MiniOS (mini_os.h/cpp)
   ├─ ProcessManager (process_manager.h/cpp)
   ├─ MemoryManager (memory_manager.h/cpp)
   ├─ DeadlockDetector (deadlock_detector.h/cpp)
   ├─ Watchdog (watchdog.h/cpp) → monitors ProcessManager
   ├─ HealthMonitor (health_monitor.h/cpp) → aggregates all systems
   └─ Logger (logger.h/cpp) ← used by all
```

---

## 🔨 Build Instructions

### Prerequisites:
- **GCC/Clang** with C++17 support (or MSVC on Windows)
- **Make** utility
- **Windows**: MinGW-w64, MSYS2, or native MSVC
- **Linux/Mac**: GCC/Clang already available

### Building:

**Option 1: Using Makefile (Recommended)**
```bash
cd self_healing_kernel

# Build the project
make

# Run the program
make run

# Clean build artifacts
make clean
```

**Option 2: Manual Compilation (GCC/Clang)**
```bash
cd self_healing_kernel

# Create object directory
mkdir -p obj/src bin

# Compile all source files
g++ -std=c++17 -pthread -I./include -c src/logger.cpp -o obj/src/logger.o
g++ -std=c++17 -pthread -I./include -c src/process_manager.cpp -o obj/src/process_manager.o
g++ -std=c++17 -pthread -I./include -c src/memory_manager.cpp -o obj/src/memory_manager.o
g++ -std=c++17 -pthread -I./include -c src/deadlock_detector.cpp -o obj/src/deadlock_detector.o
g++ -std=c++17 -pthread -I./include -c src/watchdog.cpp -o obj/src/watchdog.o
g++ -std=c++17 -pthread -I./include -c src/health_monitor.cpp -o obj/src/health_monitor.o
g++ -std=c++17 -pthread -I./include -c src/mini_os.cpp -o obj/src/mini_os.o
g++ -std=c++17 -pthread -I./include -c main.cpp -o obj/main.o

# Link the executable
g++ -std=c++17 -pthread obj/*.o obj/src/*.o -o bin/mini_os

# Run
./bin/mini_os
```

**Option 3: Windows with MSVC**
```bash
# Compile
cl /std:c++17 /EHsc /I include src\*.cpp main.cpp /link /out:mini_os.exe

# Run
mini_os.exe
```

---

## 📖 Usage Guide

### Starting the System:
```bash
./bin/mini_os
```

You'll see:
```
╔════════════════════════════════════════════╗
║  SELF-HEALING OPERATING SYSTEM KERNEL v1  ║
║          (Mini OS in C++)                  ║
╚════════════════════════════════════════════╝

[00:15:23]  [INFO]     System booting...
[00:15:23]  [INFO]     System ready  Watchdog active
```

### Main Menu:
```
╔════════════════════════════════════════════╗
║          MAIN MENU                         ║
╚════════════════════════════════════════════╝
  1. Create Process
  2. Crash Process (Fault Injection)
  3. Terminate Process
  4. Send Heartbeat
  5. Allocate Memory
  6. Free Memory
  7. Simulate Memory Leak
  8. Request Resource
  9. Release Resource
  10. Detect & Resolve Deadlocks
  11. Manual Heal Cycle
  12. Show Dashboard
  13. Show Process Table
  14. Show Memory Map
  15. Show Resource Table
  16. Show System Logs
  17. Run Auto Demo
  18. Exit
```

### Example Session:

**1. Create a critical process:**
```
Enter choice: 1
Enter process name: WebServer
Enter priority (1-10): 8
Is critical? (0=no, 1=yes): 1
Memory required (bytes): 150

[INFO] Process created  PID=1  Name=WebServer  Priority=8  Critical=YES  Memory=150B
✓ Process created with PID=1
```

**2. Allocate memory:**
```
Enter choice: 5
Enter PID: 1
Enter size (bytes): 50

[INFO] Memory allocated  50B  PID=1  BlockID=1
```

**3. Crash the process (fault injection):**
```
Enter choice: 2
Enter PID to crash: 1

[ERROR] FAULT INJECTED: Process CRASHED  PID=1  Name=WebServer

[Watchdog detects crash after 6 seconds...]

[CRITICAL] Watchdog: PID=1 Name=WebServer unresponsive for 6s -> marking CRASHED
[HEAL] SELF-HEAL: Auto-restarted critical process  PID=1  Name=WebServer  (restart #1)
```

**4. View system health:**
```
Enter choice: 12

╔════════════════════════════════════════════╗
║        SYSTEM HEALTH DASHBOARD             ║
╚════════════════════════════════════════════╝
  Overall Health:  85.5%  GOOD
  Process Status:  Running: 1  Crashed: 0  Waiting: 0  Total Active: 1
  Memory Usage:    100/1024B  (10%)
  Total Restarts:  1
  Health Bar:      [███████████████████████████░░░░░░░]
```

---

## 🧪 Testing & Demonstrations

### Test Scenario 1: Process Crash & Auto-Recovery

**Objective:** Verify automatic restart of critical processes

**Steps:**
1. Menu → 17 (Run Auto Demo)
2. Observe WebServer crash in Phase 1
3. Watch watchdog detect and restart after 6 seconds
4. Check restart count in process table

**Expected Outcome:**
- Process state transitions: RUNNING → CRASHED → RUNNING
- Log shows "SELF-HEAL: Auto-restarted critical process"
- restart_count incremented

---

### Test Scenario 2: Memory Leak Detection

**Objective:** Verify automatic leak reclamation

**Steps:**
1. Create process: Cache, Priority 5, Non-critical, 100B
2. Simulate memory leak: PID=3, Size 60B
3. Menu → 11 (Manual Heal Cycle)
4. View memory map (Menu → 14)

**Expected Outcome:**
- Leak detected after 20 seconds
- Leaked block flagged with *** LEAK marker
- After heal cycle: leak reclaimed, memory freed
- Blocks defragmented: fewer fragments shown

---

### Test Scenario 3: Deadlock Creation & Resolution

**Objective:** Demonstrate cycle detection and victim elimination

**Steps:**
1. Create P1 (WebServer, priority 8, critical)
2. Create P2 (Database, priority 9, critical)
3. Menu → 8: P1 requests Resource 1 (Printer) → SUCCESS
4. Menu → 8: P2 requests Resource 2 (Scanner) → SUCCESS
5. Menu → 8: P1 requests Resource 2 (Scanner) → BLOCKED
6. Menu → 8: P2 requests Resource 1 (Printer) → BLOCKED
7. Menu → 10 (Detect & Resolve Deadlocks)
8. View resource table (Menu → 15)

**Expected Outcome:**
- Logs show: "DEADLOCK DETECTED Cycle involves PIDs: 1 2"
- Victim selected: P2 (priority 9 ignored; selection based on cycle logic)
- Log shows: "SELF-HEAL: Deadlock resolved Victim PID=..."
- Resources freed and redistributed

---

### Test Scenario 4: Automatic Demo (Full System Test)

**Objective:** Comprehensive system demonstration

**Steps:**
```
Menu → 17 (Run Auto Demo)
```

**What Happens:**
- **Phase 1:** Creates 3 processes, shows memory allocation
- **Phase 2:** Crashes WebServer, watchdog restarts it
- **Phase 3:** Simulates memory leak, runs heal cycle
- **Phase 4:** Creates deadlock scenario, detects and resolves it
- **Summary:** Shows dashboard and recent logs

**Duration:** ~12 seconds

---

### Input Examples & Testing Data:

| Test | Menu | Inputs | Expected Result |
|------|------|--------|-----------------|
| Create Process | 1 | name="P1", priority=8, critical=1, memory=100 | PID=1 created |
| Crash Process | 2 | pid=1 | Process state→CRASHED |
| Allocate Memory | 5 | pid=1, size=50 | BlockID=1 allocated |
| Leak Memory | 7 | pid=1, size=50 | Block flagged as leak after 20s |
| Request Resource | 8 | pid=1, resource_id=1 | Resource acquired or blocked |
| Release Resource | 9 | pid=1, resource_id=1 | Resource released |

---

## 💾 Implementation Details

### Thread Safety:
All shared resources protected with `std::mutex`:
- Process table (std::map<int, PCB>)
- Memory blocks (std::vector<MemoryBlock>)
- Resources (std::map<int, Resource>)
- Log entries (std::vector<LogEntry>)

Example:
```cpp
void ProcessManager::checkAndHealProcesses() {
    std::lock_guard<std::mutex> lock(mtx);  // Acquire lock
    // Safe access to process_table
    for (auto& [pid, pcb] : process_table) {
        // Process healing logic
    }
    // Lock automatically released
}
```

### Constants (types.h):
```cpp
const int TOTAL_MEMORY      = 1024;       // Total simulated memory
const int MAX_PROCESSES     = 12;         // Max concurrent processes
const int HEARTBEAT_TIMEOUT = 6;          // Seconds before marking unresponsive
const int LEAK_THRESHOLD    = 20;         // Seconds to flag as leak
const int MAX_RESTART_COUNT = 3;          // Max auto-restarts
```

### Process States:
```cpp
enum class ProcessState {
    NEW,         // Just created
    READY,       // Waiting for CPU
    RUNNING,     // Currently executing
    WAITING,     // Blocked on resource
    TERMINATED,  // Finished execution
    CRASHED      // Failed/unresponsive
};
```

### Data Structures:

**Process Control Block (PCB):**
```cpp
struct PCB {
    int         pid;              // Process ID
    std::string name;             // Process name
    ProcessState state;           // Current state
    int         priority;         // 1-10 (higher = more important)
    int         memory_used;      // Bytes allocated
    bool        is_critical;      // Auto-restart on crash?
    int         restart_count;    // Number of restarts
    time_t      created_at;       // Creation timestamp
    time_t      last_heartbeat;   // Last heartbeat timestamp
    int         health_score;     // 0-100%
};
```

**Memory Block:**
```cpp
struct MemoryBlock {
    int    block_id;          // Unique ID
    int    size;              // Size in bytes
    int    owner_pid;         // Owner process (-1 if free)
    bool   is_free;           // Free/allocated status
    time_t allocated_at;      // Allocation timestamp
    bool   flagged_as_leak;   // Suspected leak?
};
```

**Resource:**
```cpp
struct Resource {
    int         id;              // Resource ID
    std::string name;            // Name (e.g., "Printer")
    int         total_instances; // Total available
    int         available;       // Currently available
};
```

### Deadlock Detection Algorithm:

**Wait-For Graph Construction:**
```
For each waiting process P:
    Find which process H holds the resource P wants
    Add edge: P → H
```

**Cycle Detection (DFS):**
```cpp
bool DFS(node):
    mark node as VISITED
    mark node as IN_STACK
    for each neighbor of node:
        if neighbor not VISITED:
            if DFS(neighbor) returns true: return true
        else if neighbor IN_STACK:
            return true  // Cycle found!
    mark node as not IN_STACK
    return false
```

---

## ⚙️ System Requirements

### Minimum:
- **CPU:** Any modern processor
- **RAM:** 256 MB
- **Storage:** 10 MB
- **OS:** Windows 7+, Linux (any), macOS 10.12+

### Recommended:
- **CPU:** 2 GHz or faster
- **RAM:** 1 GB
- **Compiler:** GCC 7.0+, Clang 5.0+, or MSVC 2017+
- **C++ Standard:** C++17 or later

### Compilation:
```bash
# Verify compiler supports C++17
g++ --version
g++ -std=c++17 -pthread test.cpp

# On Windows (MinGW)
gcc --version
g++ --version

# On macOS
clang --version
```

---

## 🎓 Pedagogical Value

This project demonstrates:

1. **Operating System Concepts:**
   - Process management and scheduling
   - Memory management (allocation, fragmentation, leaks)
   - Resource allocation and deadlock theory
   - Process states and transitions

2. **Concurrent Programming:**
   - Multi-threading (background watchdog)
   - Mutex-based synchronization
   - Race condition prevention
   - Atomic operations

3. **Software Engineering:**
   - Modular architecture (separate concerns)
   - Design patterns (Singleton logger)
   - Error handling and logging
   - Code organization and documentation

4. **Algorithms:**
   - Depth-first search (cycle detection)
   - Priority-based selection
   - Memory defragmentation
   - Graph theory (wait-for graphs)

---

## 📊 Performance Characteristics

| Operation | Time Complexity | Space Complexity | Notes |
|-----------|-----------------|------------------|-------|
| Create Process | O(1) | O(1) | Adds to map |
| Process Crash | O(1) | O(1) | Updates state |
| Allocate Memory | O(n) | O(1) | Scan blocks (n=blocks) |
| Free Memory | O(n) | O(1) | Defragmentation may merge |
| Detect Deadlock | O(P+E) | O(P) | DFS on wait graph (P=processes, E=edges) |
| Health Check | O(P+M) | O(1) | Iterate processes & memory blocks |

---

## 🐛 Known Limitations & Future Enhancements

### Current Limitations:
1. Simulated processes (no actual code execution)
2. Fixed memory total (can't grow dynamically)
3. Single CPU (no true parallelism simulation)
4. Basic priority scheme (no aging/boost)
5. No interrupt handlers or signals

### Future Enhancements:
1. **Virtual memory** with paging/swapping
2. **CPU scheduling** algorithms (Round-Robin, Multi-level queue)
3. **File system** implementation (FAT/NTFS simulation)
4. **I/O management** and device drivers
5. **Inter-process communication** (pipes, message queues)
6. **Security** (privilege levels, access control)
7. **Performance metrics** and statistics
8. **GUI dashboard** (instead of CLI)

---

## 📝 Assignment Rubric (20 Marks)

### Grading Breakdown:

| Component | Marks | Criteria |
|-----------|-------|----------|
| **Process Management** | 4 | Process creation, states, heartbeat mechanism |
| **Memory Management** | 4 | Allocation, deallocation, leak detection, defragmentation |
| **Deadlock Handling** | 4 | Detection algorithm, resource tracking, resolution |
| **Watchdog & Recovery** | 3 | Background monitoring, auto-restart, health checks |
| **Code Quality** | 3 | Organization, documentation, error handling, thread safety |
| **Documentation** | 1 | README, comments, usage examples |
| **Testing & Demo** | 1 | Automated demo, test scenarios, bug-free execution |

**Total: 20 Marks**

---

## 📞 Support & Troubleshooting

### Compilation Issues:

**Error: `undefined reference to 'pthread_...'`**
```bash
# Solution: Add -pthread flag
g++ -std=c++17 -pthread main.cpp src/*.cpp -o mini_os
```

**Error: `no matching function for call to 'lock_guard'`**
```bash
# Solution: Ensure C++17 support
g++ -std=c++17 ...
```

### Runtime Issues:

**No output / Program hangs:**
- Check if watchdog thread started (see logs)
- Ensure heartbeats are being sent (Menu → 4)
- Try running demo (Menu → 17)

**Process not restarting:**
- Check if process is marked CRITICAL (Menu → 13)
- Verify restart count < MAX_RESTART_COUNT (3)
- Check logs for errors

---

## 📄 License

Educational project - Free to use and modify for learning purposes.

---

## 👨‍💼 Author Notes

This Mini OS kernel project successfully demonstrates core operating system principles through practical implementation. The self-healing mechanisms showcase how modern systems can recover from failures automatically, a key concept in reliable and resilient software engineering.

The modular architecture and thread-safe design make it an excellent reference implementation for understanding OS kernel concepts, concurrent programming, and system design principles.

**Happy Learning! 🚀**

---

### Quick Reference:

```
Build:  make
Run:    make run
Clean:  make clean

Demo:   Menu option 17
Help:   Menu option 18 (Exit) + restart to see full menu again
```

**For questions or issues, consult the logs (Menu → 16) for detailed system activity!**
