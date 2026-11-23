# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -I../../include -g

# Directories
SRC_DIR := ../../src
OBJ_DIR := obj

# Source files (excluding main.cpp)
SRCS := $(filter-out ./main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Target
TARGET := TestComm

# Default rule
all: $(TARGET)

# Link step
$(TARGET): $(OBJS) main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main.cpp
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) *.o $(TARGET)
