cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # used for clangd

project(CommonHardwareInterfaces
    VERSION 1.0
    DESCRIPTION "Common building blocks shared by the hardware interfaces"
    LANGUAGES CXX
)

# CAN library (outside this source tree, so the binary dir is required)
if(NOT TARGET UniversalCAN::linux)
    if(NOT PROJECT_IS_TOP_LEVEL)
        set(BUILD_TESTING OFF)
    endif()
    set(UNIVERSALCAN_BUILD_UNICANMON OFF CACHE BOOL "" FORCE)
    add_subdirectory(
        ../lib-universal-canbus
        ${CMAKE_CURRENT_BINARY_DIR}/universalcan
    )
endif()

add_library(HardwareCommon STATIC
    src/common/VelocityActuator.cpp
    src/common/PositionActuator.cpp
    src/common/DualModeActuator.cpp
)
add_library(Hardware::common ALIAS HardwareCommon)

target_compile_features(HardwareCommon PUBLIC cxx_std_23)
target_compile_options(HardwareCommon PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(HardwareCommon
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(HardwareCommon PUBLIC UniversalCAN::linux)
