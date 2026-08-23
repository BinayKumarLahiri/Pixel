# # 1. Target executable location in the build folder
# TARGET = ../../build/live_render_demo/live_render_demo.exe
# STB_TARGET = ../../build/live_render_demo/live_render_demo.exe

# # 2. Paths pointing to the shared app dependencies directory
# COMMON_DIR = ../common
# IMGUI_DIR = $(COMMON_DIR)/imgui
# GLFW_DIR = $(COMMON_DIR)/glfw

# # Combine your app objects with ImGui core and GLFW/OpenGL backend objects
# OBJS = main.o \
#        imgui.o \
#        imgui_draw.o \
#        imgui_tables.o \
#        imgui_widgets.o \
#        imgui_demo.o \
#        imgui_impl_glfw.o \
#        imgui_impl_opengl3.o

# # Library path pointing to your compiled 'pixel' engine objects
# LIB_OBJS = ../../build/pixel/*.o

# # Compiler and flags (Added -I$(IMGUI_DIR) alongside backends and glfw include)
# CXX = g++
# CXXFLAGS = -Wall -O2 -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(GLFW_DIR)/include

# # Linker flags (Added -L$(GLFW_DIR)/lib to locate libglfw3.a)
# LIBS = -L$(GLFW_DIR)/lib -lglfw3 -lgdi32 -lopengl32 -limm32

# $(TARGET): $(OBJS)
# 	@mkdir -p $(dir $(TARGET))
# 	$(CXX) $(OBJS) $(LIB_OBJS) -o $(TARGET) $(LIBS)

# main.o: main.cpp
# 	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

# # Compilation rules pointing to the shared dependencies folder
# imgui.o: $(IMGUI_DIR)/imgui.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui_demo.o: $(IMGUI_DIR)/imgui_demo.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui_impl_glfw.o: $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui_impl_opengl3.o: $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# stb: main.cpp
# 	@mkdir -p $(dir $(STB_TARGET))
# 	$(CXX) $(CXXFLAGS) -DPIXEL_H_IMPLEMENTATION main.cpp -o $(STB_TARGET) $(LIBS)

# clean:
# 	rm -f $(OBJS)


# 1. Target executable location in the build folder
TARGET = ../../build/live_render_demo/live_render_demo.exe
STB_TARGET = ../../build/live_render_demo/live_render_demo.exe

# 2. Paths pointing to the shared app dependencies directory
COMMON_DIR = ../common
IMGUI_DIR = $(COMMON_DIR)/imgui
GLFW_DIR = $(COMMON_DIR)/glfw

# ImGui object files
IMGUI_OBJS = imgui.o \
             imgui_draw.o \
             imgui_tables.o \
             imgui_widgets.o \
             imgui_demo.o \
             imgui_impl_glfw.o \
             imgui_impl_opengl3.o

# Combine app main object with ImGui objects
OBJS = main.o $(IMGUI_OBJS)

# Generate dependency files (.d) so make knows when headers change
DEPS = $(OBJS:.o=.d)

# Library path pointing to your compiled 'pixel' engine objects
LIB_OBJS = ../../build/pixel/*.o

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(GLFW_DIR)/include

# Windows-specific libraries for OpenGL, GLFW, and Windows API system calls
LIBS = -L$(GLFW_DIR)/lib -lglfw3 -lgdi32 -lopengl32 -limm32

$(TARGET): $(OBJS)
	@mkdir -p $(dir $(TARGET))
	$(CXX) $(OBJS) $(LIB_OBJS) -o $(TARGET) $(LIBS)

# Compilation rule for main.cpp with dependency tracking
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# --- Optimized Compilation Rules for Dear ImGui ---
imgui.o: $(IMGUI_DIR)/imgui.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

imgui_demo.o: $(IMGUI_DIR)/imgui_demo.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

imgui_impl_glfw.o: $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

imgui_impl_opengl3.o: $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

stb: main.cpp
	@mkdir -p $(dir $(STB_TARGET))
	$(CXX) $(CXXFLAGS) -DPIXEL_H_IMPLEMENTATION main.cpp -o $(STB_TARGET) $(LIBS)

clean:
	rm -f $(OBJS) $(DEPS)

# Include dependency files so unchanged files aren't recompiled
-include $(DEPS)
