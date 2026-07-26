cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ScienceTUI VERSION 1.0
                  DESCRIPTION "Science TUI"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan
    ../../hardware-interfaces/science/include
)

set(SOURCES
    src/ScienceTUI.cpp
    src/CommsThread.cpp
    ../../hardware-interfaces/science/src/CanDevice.cpp
    ../../hardware-interfaces/science/src/Drill.cpp
    ../../hardware-interfaces/science/src/GenericMotor.cpp
    ../../hardware-interfaces/science/src/Heater.cpp
    ../../hardware-interfaces/science/src/Microscope.cpp
    ../../hardware-interfaces/science/src/SciencePayload.cpp
    ../../hardware-interfaces/science/src/StepperMotor.cpp

    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
    ../../hardware-interfaces/lib-universal-canbus/libuniversalcan/CommandUtils.cpp
)

add_executable(ScienceTUI ${SOURCES})



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
target_link_libraries(ScienceTUI PRIVATE spdlog::spdlog)