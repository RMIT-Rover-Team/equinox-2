cmake_minimum_required(VERSION 3.16)
project(DEFCOM LANGUAGES CXX)

# Collect all source files
file(GLOB_RECURSE DEFCOM_SOURCES src/*.cpp)

# Build the library
add_library(DEFCOM SHARED ${DEFCOM_SOURCES})

# Public include directory
target_include_directories(DEFCOM
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# C++ standard
target_compile_features(DEFCOM PUBLIC cxx_std_17)

# Install 
install(TARGETS DEFCOM
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include)
