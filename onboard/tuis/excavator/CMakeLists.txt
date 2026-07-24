cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ExcavatorTUI VERSION 1.0
                  DESCRIPTION "Excavator TUI"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan
    ../../hardware-interfaces/excavator/src
)

set(SOURCES
    src/ExcavatorTUI.cpp
    src/CommsThread.cpp
    ../../hardware-interfaces/excavator/src/ExcavatorPayload.cpp
    ../../hardware-interfaces/excavator/src/ExcavatorActuator.cpp
    ../../hardware-interfaces/excavator/src/TeethActuator.cpp
    ../../hardware-interfaces/excavator/src/PaverMagnet.cpp

    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/CommandUtils.cpp
)

add_executable(ExcavatorTUI ${SOURCES})



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
target_link_libraries(ExcavatorTUI PRIVATE spdlog::spdlog)
