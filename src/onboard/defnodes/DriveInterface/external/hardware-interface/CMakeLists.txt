cmake_minimum_required(VERSION 3.16)
project(hardwareInterface LANGUAGES CXX)

# Collect all source files
file(GLOB_RECURSE hardwareInterface_SOURCES src/*.cpp)

list(REMOVE_ITEM hardwareInterface_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pyTorqueWrapper.cpp
)


# Build the library
add_library(hardwareInterface SHARED ${hardwareInterface_SOURCES})

# Public include directory
target_include_directories(hardwareInterface
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)



# C++ standard
target_compile_features(hardwareInterface PUBLIC cxx_std_17)

# Install 
install(TARGETS hardwareInterface
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include)
