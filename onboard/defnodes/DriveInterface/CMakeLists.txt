cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)

# External libraries
add_subdirectory(external/DEFCOM)
add_subdirectory(external/hardware-interface)


# Collect all source files except main.cpp
file(GLOB_RECURSE DRIVE_SOURCES
    src/*.cpp
)
list(REMOVE_ITEM DRIVE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Build the DriveInterface library
add_library(DriveInterfaceLib SHARED ${DRIVE_SOURCES})

target_link_libraries(DriveInterfaceLib
    PUBLIC
        DEFCOM
        hardwareInterface
)

target_include_directories(DriveInterfaceLib
    PUBLIC
        external/DEFCOM/include
        external/hardware-interface/include
        include
)

# Build the executable
add_executable(DriveInterface src/main.cpp)

target_link_libraries(DriveInterface
    PRIVATE
        DriveInterfaceLib
)

target_compile_features(DriveInterface PUBLIC cxx_std_17)

