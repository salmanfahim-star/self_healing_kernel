# Makefile for Self-Healing OS Kernel (Mini OS)
# Supports both Windows (MinGW) and Linux/Unix

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread

# Directories
INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/logger.cpp \
          $(SRC_DIR)/process_manager.cpp \
          $(SRC_DIR)/memory_manager.cpp \
          $(SRC_DIR)/deadlock_detector.cpp \
          $(SRC_DIR)/watchdog.cpp \
          $(SRC_DIR)/health_monitor.cpp \
          $(SRC_DIR)/mini_os.cpp \
          main.cpp

# Object files
OBJECTS = $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)

# Output executable
TARGET = $(BIN_DIR)/mini_os

# Default target
.PHONY: all clean run

all: $(TARGET)

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link the executable
$(TARGET): $(BIN_DIR) $(OBJ_DIR) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $(OBJECTS)
	@echo "Build complete! Run with: make run"

# Compile source files
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Run the program
run: $(TARGET)
	@$(TARGET)

# Clean build artifacts
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete!"

# Help
help:
	@echo "Available targets:"
	@echo "  make       - Build the project"
	@echo "  make run   - Build and run"
	@echo "  make clean - Remove build artifacts"
