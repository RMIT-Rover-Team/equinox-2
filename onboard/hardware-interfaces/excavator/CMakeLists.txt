cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Excavator VERSION 1.0
                  DESCRIPTION "Excavator hardware interface"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

set(SOURCES
  src/ExcavatorActuator.cpp
  src/ExcavatorPayload.cpp
  ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
  ../lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
)


add_library(${PROJECT_NAME} ${SOURCES})