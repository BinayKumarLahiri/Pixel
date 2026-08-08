# --- Compiler Settings ---
CC ?= gcc
CXX ?= g++
CFLAGS ?= -Wall -O2
CXXFLAGS ?= -Wall -O2 -static -static-libgcc -static-libstdc++

# --- Directories ---
SRC_DIR := src
BUILD_DIR := build
PIXEL_BUILD_DIR := $(BUILD_DIR)/pixel
APPS_DIR := apps

# --- Find Sources and Define Objects ---
# Find all C and C++ files in the src directory
SRCS_C   := $(wildcard $(SRC_DIR)/*.c)
SRCS_CPP := $(wildcard $(SRC_DIR)/*.cpp)
SRCS_CC  := $(wildcard $(SRC_DIR)/*.cc)

# Map the source files to their corresponding object files in the pixel build directory
OBJS := $(patsubst $(SRC_DIR)/%.c, $(PIXEL_BUILD_DIR)/%.o, $(SRCS_C)) \
        $(patsubst $(SRC_DIR)/%.cpp, $(PIXEL_BUILD_DIR)/%.o, $(SRCS_CPP)) \
        $(patsubst $(SRC_DIR)/%.cc, $(PIXEL_BUILD_DIR)/%.o, $(SRCS_CC))

# Generate a list of dependency files (.d) created during compilation
DEPS := $(OBJS:.o=.d)

# Find all application directories
APPS := $(wildcard $(APPS_DIR)/*)

.PHONY: all lib dist clean $(APPS)

all: lib $(APPS)


apps: $(APPS)

# Build the library object files in /build/pixel
lib: $(OBJS)
	@echo "Library objects built successfully in $(PIXEL_BUILD_DIR)."

# --- Compilation Rules ---

# Rule for C files
$(PIXEL_BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Rule for C++ files (.cpp)
$(PIXEL_BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Rule for C++ files (.cc)
$(PIXEL_BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Tell Make to go into each app directory, run its Makefile, then clean it
$(APPS): lib
	@echo "--- Building App: $(notdir $@) ---"
	@mkdir -p $(BUILD_DIR)/$(notdir $@)
	@$(MAKE) -C $@
	@echo "--- Cleaning App: $(notdir $@) ---"
	@$(MAKE) -C $@ clean

dist:
	@echo "Amalgamating library into a single header..."
	python3 tools/amalgamate.py src/ dist/pixel.h

clean:
	rm -rf $(BUILD_DIR)/*
	for app in $(APPS); do $(MAKE) -C $$app clean; done

# Include the generated dependency files so Make knows when a header changes
-include $(DEPS)
