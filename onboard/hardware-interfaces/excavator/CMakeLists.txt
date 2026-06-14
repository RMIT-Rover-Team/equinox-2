cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Excavator VERSION 1.0
                  DESCRIPTION "Excavator actuator interface"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

set(SOURCES
  ExcavatorActuator.cpp
  ExcavatorPayload.cpp
  ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
)


add_library(${PROJECT_NAME} ${SOURCES})