## Build Guide

Use one method only.

### Windows (MinGW)

```bat
.\build.bat
bin\mini_os.exe
```

If `g++` is not found, install MinGW-w64 and add its `bin` directory to PATH.

### Linux/macOS (Makefile)

```bash
make
make run
```

### Any OS (CMake)

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Executable output:
- Linux/macOS: `build/bin/mini_os` or `bin/mini_os`
- Windows: `build/bin/mini_os.exe` or `bin/mini_os.exe`

### Troubleshooting

- `g++: command not found`: install GCC/MinGW and update PATH.
- `make: command not found`: install build tools (`build-essential` on Ubuntu, Xcode tools on macOS).
- pthread link issue: always compile with `-pthread`.
clang++ -std=c++17 -pthread -I./include -c src/watchdog.cpp -o obj/src/watchdog.o
clang++ -std=c++17 -pthread -I./include -c src/health_monitor.cpp -o obj/src/health_monitor.o
clang++ -std=c++17 -pthread -I./include -c src/mini_os.cpp -o obj/src/mini_os.o
clang++ -std=c++17 -pthread -I./include -c main.cpp -o obj/main.o

# Link
clang++ -std=c++17 -pthread obj/main.o obj/src/*.o -o bin/mini_os

# Run
./bin/mini_os
```

---

## Troubleshooting

### Compilation Errors

**Error: "cannot find -lpthread"**
- Solution: Use `-pthread` flag (not `-lpthread`)
- Correct: `g++ -pthread ...`

**Error: "unrecognized command line option '-std=c++17'"**
- Solution: Update compiler to support C++17
- Windows: Update MinGW-w64
- Linux: `sudo apt-get install g++-7` (or later)
- macOS: `xcode-select --install` (to update Xcode)

**Error: "no such file or directory: 'include/types.h'"**
- Solution: Ensure you're in the project root directory
- Correct: `cd self_healing_kernel` first

**Error: Build fails with "g++ not found"**
- Windows: Install MinGW-w64 and add to PATH
- Linux: `sudo apt-get install build-essential`
- macOS: `xcode-select --install`

### Runtime Errors

**Error: "ANSI escape codes not working on Windows"**
- This is normal - colors work but may not display correctly
- They will work properly if:
  - Running in modern terminal (Windows Terminal, ConEmu, etc.)
  - Using Visual Studio Code terminal
  - Using WSL (Windows Subsystem for Linux)

**Program hangs on startup:**
- Wait 5 seconds for watchdog to initialize
- If still hung, press Ctrl+C to exit and check logs

**Watchdog not starting:**
- On some systems, may require additional thread initialization
- Try running menu option 16 (Show Logs) to check for errors

---

## Verification

After building, verify the executable works:

```bash
# Windows
bin\mini_os.exe

# Linux/macOS
./bin/mini_os
```

You should see:
```
╔════════════════════════════════════════════╗
║  SELF-HEALING OPERATING SYSTEM KERNEL v1  ║
║          (Mini OS in C++)                  ║
╚════════════════════════════════════════════╝

[HH:MM:SS]  [INFO]     System booting...
[HH:MM:SS]  [INFO]     System ready  Watchdog active
```

If you see this, the build was successful!

---

## Advanced Options

### Debug Build (with symbols for debugging)

```bash
# Linux/macOS
g++ -std=c++17 -pthread -g -I./include -c src/*.cpp main.cpp
g++ -std=c++17 -pthread -g obj/*.o -o bin/mini_os

# Then debug with:
gdb ./bin/mini_os
```

### Release Build (optimized)

```bash
g++ -std=c++17 -pthread -O3 -I./include -c src/*.cpp main.cpp
g++ -std=c++17 -pthread -O3 obj/*.o -o bin/mini_os
```

### Sanitizers (for debugging memory issues)

```bash
# Address Sanitizer
g++ -std=c++17 -pthread -g -fsanitize=address -I./include src/*.cpp main.cpp -o mini_os
./mini_os

# Thread Sanitizer
g++ -std=c++17 -pthread -g -fsanitize=thread -I./include src/*.cpp main.cpp -o mini_os
./mini_os
```

---

## Questions?

Consult the main README.md for usage documentation and examples.
