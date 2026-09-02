# 1. Point the target to its own dedicated folder in the build directory
TARGET = ../../build/test_app/test_app

OBJS = main.o


# STB-style amalgamated header
PIXEL_HEADER = ../../dist/pixel.h
# STB-style test executable
STB_TARGET = ../../build/test_app/test_app

# 2. Update the library path to point to the new 'pixel' directory
LIB_OBJS = ../../build/pixel/*.o

CXX = g++
CXXFLAGS = -Wall -O2 -static -static-libgcc -static-libstdc++ -I../common

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIB_OBJS) -o $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o


stb: main.cpp
	@mkdir -p $(dir $(STB_TARGET))
	$(CXX) $(CXXFLAGS) -DPIXEL_H_IMPLEMENTATION main.cpp -o $(STB_TARGET)

clean:
	# 3. ONLY delete local objects here!
	# DO NOT delete $(TARGET), otherwise the root Makefile
	# will delete your app immediately after building it.
	rm -f $(OBJS)
