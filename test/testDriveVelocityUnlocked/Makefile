# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11 -I ../../include/

# Source files
SRCS = ../../src/fake_wrapper.cpp ../../src/odrive_controllers.cpp ../../src/odrive_wrapper.cpp ../../src/linear_calc.cpp ../../src/torque_handler.cpp ../../src/pid_calc.cpp testDrive.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = TestDrive

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)


# Phony targets
.PHONY: all clean