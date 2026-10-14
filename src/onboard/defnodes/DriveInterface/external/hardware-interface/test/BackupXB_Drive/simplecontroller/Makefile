# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -lSDL2

# Common sources (shared across executables)
COMMON_SRCS = SimpleController.cpp TCPstreams.cpp RemoteController.cpp
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)

# Executable names
LOCAL  = LocalExample
REMOTE = RemoteExample
HELPER = RemoteInputHelper

# Default target: build everything
all: $(LOCAL) $(REMOTE) $(HELPER)

# ControllerDemo (full demo build)

# LocalExample (example.cpp + common)
$(LOCAL): example.o $(COMMON_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# RemoteExample (remoteExample.cpp + common)
$(REMOTE): remoteExample.o $(COMMON_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# RemoteInputHelper (RemoteControlInput.cpp + common)
$(HELPER): RemoteControlInput.o $(COMMON_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(COMMON_OBJS) example.o remoteExample.o RemoteControlInput.o $(LOCAL) $(REMOTE) $(HELPER)

# Phony targets
.PHONY: all clean
