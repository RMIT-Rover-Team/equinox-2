# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11 -I ../../include/

# Source files
SRCS = ../../src/pid_calc.cpp ../../src/linear_calc.cpp testPID.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = TestPID

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