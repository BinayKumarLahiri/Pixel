# 1. Point the target to its own dedicated folder in the build directory
TARGET = ../../build/test_app/test_app

OBJS = main.o

# 2. Update the library path to point to the new 'pixel' directory
LIB_OBJS = ../../build/pixel/*.o

CXX = g++
CXXFLAGS = -Wall -O2 -static -static-libgcc -static-libstdc++

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIB_OBJS) -o $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	# 3. ONLY delete local objects here!
	# DO NOT delete $(TARGET), otherwise the root Makefile
	# will delete your app immediately after building it.
	rm -f $(OBJS)
