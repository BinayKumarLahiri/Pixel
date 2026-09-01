# 1. Target executable location in the build folder
TARGET = ../../build/fenster_demo/fenster_demo.exe

# 2. Paths pointing to the shared app dependencies directory
COMMON_DIR = ../common
FENSTER_DIR = $(COMMON_DIR)/fenster

# Only main.o is needed since fenster is header-only
OBJS = main.o

# Generate dependency files (.d) so make knows when headers change
DEPS = $(OBJS:.o=.d)

# Library path pointing to your compiled 'pixel' engine objects
LIB_OBJS = ../../build/pixel/*.o

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11 -I$(FENSTER_DIR)

# Windows-specific library for fenster Win32 API system calls
LIBS = -lgdi32

$(TARGET): $(OBJS)
	@mkdir -p $(dir $(TARGET))
	$(CXX) $(OBJS) $(LIB_OBJS) -o $(TARGET) $(LIBS)

# Compilation rule for main.cpp with dependency tracking
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

# Include dependency files so unchanged files aren't recompiled
-include $(DEPS)
