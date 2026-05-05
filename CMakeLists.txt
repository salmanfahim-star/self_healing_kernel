cmake_minimum_required(VERSION 3.10)
project(SelfHealingKernel VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Find pthread (required for watchdog)
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    src/logger.cpp
    src/process_manager.cpp
    src/memory_manager.cpp
    src/deadlock_detector.cpp
    src/watchdog.cpp
    src/health_monitor.cpp
    src/mini_os.cpp
    main.cpp
)

# Include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create executable
add_executable(mini_os ${SOURCES})

# Link with pthread
target_link_libraries(mini_os Threads::Threads)

# Output directory
set_target_properties(mini_os PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Print build info
message(STATUS "Self-Healing OS Kernel v1.0")
message(STATUS "C++ Standard: C++17")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Executable: ${CMAKE_BINARY_DIR}/bin/mini_os${CMAKE_EXECUTABLE_SUFFIX}")
