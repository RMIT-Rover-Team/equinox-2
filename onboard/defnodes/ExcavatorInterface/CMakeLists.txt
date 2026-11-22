cmake_minimum_required(VERSION 3.21)
project(ExcavatorNode LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add DEFCOM library from local folder
add_subdirectory(external/DEFCOM)

# Add HW Interface
if(NOT TARGET ExcavatorHardwareInterface)
    add_subdirectory(
        ../../hardware-interfaces/excavator
        ${CMAKE_CURRENT_BINARY_DIR}/excavator-hi
    )
endif()

# Your executable
add_executable(ExcavatorInterface
    src/main.cpp
    src/ExcavatorNode.cpp
)

target_compile_options(ExcavatorInterface PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Link against DEFCOM
target_link_libraries(ExcavatorInterface
    PRIVATE
        ExcavatorHardwareInterface
        DEFCOM
)

# The node's own headers, plus DEFCOM's
target_include_directories(ExcavatorInterface
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        external/DEFCOM/include
)

# C++ standard
target_compile_features(ExcavatorInterface PUBLIC cxx_std_17)
