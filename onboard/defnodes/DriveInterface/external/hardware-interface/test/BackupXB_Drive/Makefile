# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -I ../../include/

# Source files
SRCS = simplecontroller/RemoteController.cpp ../../src/MyActuator_wrapper.cpp ../../src/SocketCanWrapper.cpp simplecontroller/TCPstreams.cpp main.cpp RoverCanMaster.cpp SocketCanWrapper.cpp CommandUtils.cpp ../../src/fake_wrapper.cpp ../../src/odrive_controllers.cpp ../../src/odrive_wrapper.cpp ../../src/linear_calc.cpp ../../src/torque_handler.cpp ../../src/pid_calc.cpp 
 

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = BackupXB_Server

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJS)
	$(CXX)  -o $@ $^ $(CXXFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean
