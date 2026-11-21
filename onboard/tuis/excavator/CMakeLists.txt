cmake_minimum_required(VERSION 3.21)

project(ExcavatorTUI
    VERSION 1.0
    DESCRIPTION "Excavator TUI"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Excavator hardware interface (brings in UniversalCAN::linux transitively).
# It lives outside this source tree, so the binary dir is required.
# The TUI has no tests, so keep dependencies from building theirs.
set(BUILD_TESTING OFF)
if(NOT TARGET ExcavatorHardwareInterface)
    add_subdirectory(
        ../../hardware-interfaces/excavator
        ${CMAKE_CURRENT_BINARY_DIR}/excavator-hi
    )
endif()

#
# SPDLOG
#
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_TAG v1.17.0
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(spdlog)

add_executable(ExcavatorTUI
    src/ExcavatorTUI.cpp
    src/CommsThread.cpp
)

target_compile_features(ExcavatorTUI PRIVATE cxx_std_23)
target_compile_options(ExcavatorTUI PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(ExcavatorTUI PRIVATE
    ExcavatorHardwareInterface
    spdlog::spdlog
)
