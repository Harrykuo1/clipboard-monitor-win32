# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -DUNICODE -D_UNICODE -DWINVER=0x0600 -D_WIN32_WINNT=0x0600

# Debug flags
DEBUG_FLAGS = -g -D_DEBUG

# Include directories
INCLUDES = -Isrc

# Source files
SOURCES = src/main.cpp \
          src/core/ClipboardMonitor.cpp \
          src/ui/MainWindow.cpp \
          src/utils/Utils.cpp

# Object files
OBJECTS = $(patsubst src/%.cpp,build/%.o,$(SOURCES))
DEBUG_OBJECTS = $(patsubst src/%.cpp,build/debug/%.o,$(SOURCES))

# Executable name
EXECUTABLE = build/ClipboardMonitor.exe
DEBUG_EXECUTABLE = build/debug/ClipboardMonitor_debug.exe

# Libraries
LIBS = -luser32 -lshell32 -lpsapi -lcomctl32 -lgdi32 -lole32 -luxtheme -lcomdlg32

# Default target
all: directories $(EXECUTABLE)

# Debug target
debug: debug_directories $(DEBUG_EXECUTABLE)

# Create necessary directories
directories:
	@if not exist build mkdir build
	@if not exist build\core mkdir build\core
	@if not exist build\ui mkdir build\ui
	@if not exist build\utils mkdir build\utils

# Create necessary directories for debug build
debug_directories:
	@if not exist build\debug mkdir build\debug
	@if not exist build\debug\core mkdir build\debug\core
	@if not exist build\debug\ui mkdir build\debug\ui
	@if not exist build\debug\utils mkdir build\debug\utils

# Linking rule for release
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Linking rule for debug
$(DEBUG_EXECUTABLE): $(DEBUG_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Compilation rule for release
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compilation rule for debug
build/debug/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	@if exist build rmdir /s /q build

# Run rule for release
run: all
	$(EXECUTABLE)

# Run rule for debug
run_debug: debug
	$(DEBUG_EXECUTABLE)

.PHONY: all debug clean run run_debug directories debug_directories