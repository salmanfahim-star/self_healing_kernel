@echo off
REM Build script for Self-Healing OS Kernel on Windows
REM Requires: MinGW GCC, Clang, or MSVC

setlocal enabledelayedexpansion

echo Compiling Self-Healing OS Kernel...

REM Create directories
if not exist obj mkdir obj
if not exist obj\src mkdir obj\src
if not exist bin mkdir bin

REM Compile source files using g++ (MinGW)
echo Compiling source files...
g++ -std=c++17 -pthread -I./include -c src/logger.cpp -o obj/src/logger.o 2>nul
if errorlevel 1 (
    echo Error: g++ not found. Install MinGW-w64 or GCC for Windows.
    echo Visit: https://www.mingw-w64.org/
    exit /b 1
)

g++ -std=c++17 -pthread -I./include -c src/process_manager.cpp -o obj/src/process_manager.o
g++ -std=c++17 -pthread -I./include -c src/memory_manager.cpp -o obj/src/memory_manager.o
g++ -std=c++17 -pthread -I./include -c src/deadlock_detector.cpp -o obj/src/deadlock_detector.o
g++ -std=c++17 -pthread -I./include -c src/watchdog.cpp -o obj/src/watchdog.o
g++ -std=c++17 -pthread -I./include -c src/health_monitor.cpp -o obj/src/health_monitor.o
g++ -std=c++17 -pthread -I./include -c src/mini_os.cpp -o obj/src/mini_os.o
g++ -std=c++17 -pthread -I./include -c main.cpp -o obj/main.o

if errorlevel 1 (
    echo Compilation failed!
    exit /b 1
)

echo Linking executable...
g++ -std=c++17 -pthread obj\main.o obj\src\*.o -o bin\mini_os.exe

if errorlevel 1 (
    echo Linking failed!
    exit /b 1
)

echo.
echo Build successful! Executable: bin\mini_os.exe
echo.
echo To run: bin\mini_os.exe
