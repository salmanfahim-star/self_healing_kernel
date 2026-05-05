#include "../include/mini_os.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

// ===== Utility: Enable ANSI colors on Windows =====
void enableANSIColors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004;  // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

// ===== Constructor/Destructor =====

MiniOS::MiniOS()
    : logger(Logger::getInstance()),
      deadlock_detector(),
      watchdog(process_manager, 3),
      health_monitor(process_manager, memory_manager, deadlock_detector),
      running(false) {
    setupResources();
}

MiniOS::~MiniOS() {
    shutdown();
}

void MiniOS::setupResources() {
    // Create some resources for deadlock detection demo
    deadlock_detector.addResource("Printer", 1);
    deadlock_detector.addResource("Scanner", 1);
    deadlock_detector.addResource("Disk", 2);
}

void MiniOS::boot() {
    enableANSIColors();

    std::cout << "\033[1;36m";
    std::cout << "╔════════════════════════════════════════════╗\n";
    std::cout << "║  SELF-HEALING OPERATING SYSTEM KERNEL v1  ║\n";
    std::cout << "║          (Mini OS in C++)                  ║\n";
    std::cout << "╚════════════════════════════════════════════╝\033[0m\n\n";

    logger.log(LogLevel::INFO, "System booting...");
    watchdog.start();
    logger.log(LogLevel::INFO, "System ready  Watchdog active");
    running = true;
}

void MiniOS::shutdown() {
    watchdog.stop();
    logger.log(LogLevel::INFO, "System shutdown");
    running = false;
}

// ===== Process Operations =====

int MiniOS::createProcess(const std::string& name, int priority,
                          bool is_critical, int memory_req) {
    return process_manager.createProcess(name, priority, is_critical, memory_req);
}

void MiniOS::crashProcess(int pid) {
    process_manager.crashProcess(pid);
}

void MiniOS::terminateProcess(int pid) {
    process_manager.terminateProcess(pid);
}

void MiniOS::sendHeartbeat(int pid) {
    process_manager.sendHeartbeat(pid);
}

// ===== Memory Operations =====

void MiniOS::allocateMemory(int pid, int size) {
    memory_manager.allocate(pid, size);
}

void MiniOS::freeMemory(int block_id) {
    memory_manager.freeBlock(block_id);
}

void MiniOS::leakMemory(int pid, int size) {
    memory_manager.simulateLeak(pid, size);
}

// ===== Resource Operations =====

void MiniOS::requestResource(int pid, int resource_id) {
    deadlock_detector.requestResource(pid, resource_id);
}

void MiniOS::releaseResource(int pid, int resource_id) {
    deadlock_detector.releaseResource(pid, resource_id);
}

// ===== Self-Healing =====

void MiniOS::runHealCycle() {
    logger.log(LogLevel::HEAL, "=== MANUAL HEAL CYCLE TRIGGERED ===");
    
    logger.log(LogLevel::INFO, "Scanning for memory leaks...");
    memory_manager.detectAndFixLeaks();

    logger.log(LogLevel::INFO, "Defragmenting memory...");
    memory_manager.defragment();

    logger.log(LogLevel::INFO, "Checking and healing processes...");
    process_manager.checkAndHealProcesses();

    logger.log(LogLevel::INFO, "=== HEAL CYCLE COMPLETE ===");
}

void MiniOS::detectAndResolveDeadlocks() {
    std::vector<int> deadlocked;
    if (deadlock_detector.detectDeadlock(deadlocked)) {
        logger.log(LogLevel::CRITICAL, "Deadlock detected! Running resolution...");
        deadlock_detector.resolveDeadlock(process_manager.getProcessTable());
    } else {
        logger.log(LogLevel::INFO, "Deadlock check complete  No deadlock found");
    }
}

// ===== Display Functions =====

void MiniOS::showDashboard() {
    health_monitor.displayDashboard();
}

void MiniOS::showProcessTable() {
    process_manager.displayProcessTable();
}

void MiniOS::showMemoryMap() {
    memory_manager.displayMemoryMap();
}

void MiniOS::showResourceTable() {
    deadlock_detector.displayResourceTable();
}

void MiniOS::showLogs() {
    logger.showLogs(25);
}

// ===== Menu System =====

void MiniOS::printMenu() {
    std::cout << "\n\033[1;33m";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║          MAIN MENU                     ║\n";
    std::cout << "╚════════════════════════════════════════╝\033[0m\n";
    std::cout << "  1. Create Process\n";
    std::cout << "  2. Crash Process (Fault Injection)\n";
    std::cout << "  3. Terminate Process\n";
    std::cout << "  4. Send Heartbeat\n";
    std::cout << "  5. Allocate Memory\n";
    std::cout << "  6. Free Memory\n";
    std::cout << "  7. Simulate Memory Leak\n";
    std::cout << "  8. Request Resource\n";
    std::cout << "  9. Release Resource\n";
    std::cout << "  10. Detect & Resolve Deadlocks\n";
    std::cout << "  11. Manual Heal Cycle\n";
    std::cout << "  12. Show Dashboard\n";
    std::cout << "  13. Show Process Table\n";
    std::cout << "  14. Show Memory Map\n";
    std::cout << "  15. Show Resource Table\n";
    std::cout << "  16. Show System Logs\n";
    std::cout << "  17. Run Auto Demo\n";
    std::cout << "  18. Exit\n";
    std::cout << "  Enter choice: ";
}

void MiniOS::handleInput(int choice) {
    int pid, priority, size, resource_id, block_id;
    std::string name;
    bool is_critical;

    switch (choice) {
        case 1: {
            // Create Process
            std::cout << "Enter process name: ";
            std::getline(std::cin, name);
            std::cout << "Enter priority (1-10): ";
            std::cin >> priority;
            std::cout << "Is critical? (0=no, 1=yes): ";
            std::cin >> is_critical;
            std::cout << "Memory required (bytes): ";
            std::cin >> size;
            std::cin.ignore();
            pid = createProcess(name, priority, is_critical, size);
            if (pid > 0)
                std::cout << "\n✓ Process created with PID=" << pid << "\n";
            break;
        }
        case 2: {
            // Crash Process
            std::cout << "Enter PID to crash: ";
            std::cin >> pid;
            std::cin.ignore();
            crashProcess(pid);
            std::cout << "\n✓ Process crashed  Watchdog should detect and heal...\n";
            break;
        }
        case 3: {
            // Terminate Process
            std::cout << "Enter PID to terminate: ";
            std::cin >> pid;
            std::cin.ignore();
            terminateProcess(pid);
            std::cout << "\n✓ Process terminated\n";
            break;
        }
        case 4: {
            // Send Heartbeat
            std::cout << "Enter PID to send heartbeat: ";
            std::cin >> pid;
            std::cin.ignore();
            sendHeartbeat(pid);
            std::cout << "\n✓ Heartbeat sent\n";
            break;
        }
        case 5: {
            // Allocate Memory
            std::cout << "Enter PID: ";
            std::cin >> pid;
            std::cout << "Enter size (bytes): ";
            std::cin >> size;
            std::cin.ignore();
            allocateMemory(pid, size);
            break;
        }
        case 6: {
            // Free Memory
            std::cout << "Enter Block ID to free: ";
            std::cin >> block_id;
            std::cin.ignore();
            freeMemory(block_id);
            break;
        }
        case 7: {
            // Simulate Memory Leak
            std::cout << "Enter PID: ";
            std::cin >> pid;
            std::cout << "Enter leak size (bytes): ";
            std::cin >> size;
            std::cin.ignore();
            leakMemory(pid, size);
            break;
        }
        case 8: {
            // Request Resource
            std::cout << "Enter PID: ";
            std::cin >> pid;
            std::cout << "Enter Resource ID: ";
            std::cin >> resource_id;
            std::cin.ignore();
            requestResource(pid, resource_id);
            break;
        }
        case 9: {
            // Release Resource
            std::cout << "Enter PID: ";
            std::cin >> pid;
            std::cout << "Enter Resource ID: ";
            std::cin >> resource_id;
            std::cin.ignore();
            releaseResource(pid, resource_id);
            break;
        }
        case 10: {
            // Deadlock detection & resolution
            detectAndResolveDeadlocks();
            break;
        }
        case 11: {
            // Manual Heal Cycle
            runHealCycle();
            break;
        }
        case 12: {
            // Show Dashboard
            showDashboard();
            break;
        }
        case 13: {
            // Show Process Table
            showProcessTable();
            break;
        }
        case 14: {
            // Show Memory Map
            showMemoryMap();
            break;
        }
        case 15: {
            // Show Resource Table
            showResourceTable();
            break;
        }
        case 16: {
            // Show Logs
            showLogs();
            break;
        }
        case 17: {
            // Auto Demo
            runDemo();
            break;
        }
        case 18: {
            running = false;
            std::cout << "\nExiting...\n";
            break;
        }
        default:
            std::cout << "\nInvalid choice!\n";
    }
}

void MiniOS::run() {
    int choice;
    while (running) {
        printMenu();
        std::cin >> choice;
        std::cin.ignore();
        handleInput(choice);
    }
}

// ===== Automated Demo =====

void MiniOS::runDemo() {
    std::cout << "\n\033[1;35m╔════════════════════════════════════════╗\n";
    std::cout << "║       AUTOMATED DEMO STARTING          ║\n";
    std::cout << "╚════════════════════════════════════════╝\033[0m\n\n";

    logger.log(LogLevel::INFO, "Demo: Creating system processes...");
    int p1 = createProcess("WebServer", 8, true, 150);
    int p2 = createProcess("Database", 9, true, 200);
    int p3 = createProcess("Cache", 5, false, 100);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n--- Demo Phase 1: Memory Management ---\n";
    logger.log(LogLevel::INFO, "Demo: Allocating memory blocks...");
    allocateMemory(p1, 50);
    allocateMemory(p2, 75);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    showMemoryMap();

    std::cout << "\n--- Demo Phase 2: Process Health & Auto-Restart ---\n";
    logger.log(LogLevel::INFO, "Demo: Simulating process crash...");
    crashProcess(p1);

    std::cout << "\033[33mWaiting for watchdog to detect and heal...\033[0m\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    showProcessTable();

    std::cout << "\n--- Demo Phase 3: Memory Leak Detection ---\n";
    logger.log(LogLevel::INFO, "Demo: Injecting memory leak...");
    leakMemory(p3, 60);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    runHealCycle();
    showMemoryMap();

    std::cout << "\n--- Demo Phase 4: Deadlock Scenario ---\n";
    logger.log(LogLevel::INFO, "Demo: Setting up deadlock scenario...");
    
    // Create circular wait scenario
    requestResource(p1, 1);  // WebServer gets Printer
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    requestResource(p2, 2);  // Database gets Scanner
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    requestResource(p1, 2);  // WebServer waits for Scanner (held by DB)
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    requestResource(p2, 1);  // Database waits for Printer (held by WebServer)

    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "\n\033[31mDEADLOCK CYCLE CREATED!\033[0m\n";
    showResourceTable();

    logger.log(LogLevel::INFO, "Demo: Running deadlock detection & resolution...");
    detectAndResolveDeadlocks();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n--- Demo Summary ---\n";
    showDashboard();
    showLogs();

    std::cout << "\n\033[1;35m✓ DEMO COMPLETE\033[0m\n";
}
