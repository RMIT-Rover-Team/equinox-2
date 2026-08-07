cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ArmTUI VERSION 1.0
                  DESCRIPTION "Arm TUI"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan
    ../../hardware-interfaces/arm/src
)

set(SOURCES
    src/ArmTUI.cpp
    src/CommsThread.cpp
    ../../hardware-interfaces/arm/src/ArmPayload.cpp
    ../../hardware-interfaces/arm/src/EndEffector.cpp
    ../../hardware-interfaces/arm/src/MyActuatorMotor.cpp

    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/CommandUtils.cpp
)

add_executable(ArmTUI ${SOURCES})



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
target_link_libraries(ArmTUI PRIVATE spdlog::spdlog)