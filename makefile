# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -DUNICODE -D_UNICODE -DWINVER=0x0600 -D_WIN32_WINNT=0x0600

# Include directories
INCLUDES = -Isrc

# Source files
SOURCES = src/main.cpp \
          src/core/ClipboardMonitor.cpp \
          src/ui/TrayIcon.cpp \
          src/utils/Utils.cpp

# Object files
OBJECTS = $(patsubst src/%.cpp,build/%.o,$(SOURCES))

# Executable name
EXECUTABLE = build/ClipboardMonitor.exe

# Libraries
LIBS = -luser32 -lshell32 -lpsapi

# Default target
all: directories $(EXECUTABLE)

# Create necessary directories
directories:
	if not exist build mkdir build
	if not exist build\core mkdir build\core
	if not exist build\ui mkdir build\ui
	if not exist build\utils mkdir build\utils

# Linking rule
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Compilation rule
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	if exist build rmdir /s /q build

# Run rule
run: all
	$(EXECUTABLE)

.PHONY: all clean run directories