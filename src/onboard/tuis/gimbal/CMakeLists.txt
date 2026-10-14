cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(GimbalTUI VERSION 1.0
                  DESCRIPTION "Gimbal TUI"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan
    ../../hardware-interfaces/gimbal/src
)

set(SOURCES
    src/GimbalTUI.cpp
    src/CommsThread.cpp
    ../../hardware-interfaces/gimbal/src/Gimbal.cpp

    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/CommandUtils.cpp
)

add_executable(GimbalTUI ${SOURCES})



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
target_link_libraries(GimbalTUI PRIVATE spdlog::spdlog)