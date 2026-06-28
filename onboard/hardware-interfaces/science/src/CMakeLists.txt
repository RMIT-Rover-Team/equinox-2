cmake_minimum_required(VERSION 3.15)

project(SciencePayload
  VERSION 1.0
  DESCRIPTION "Science payload hardware interface"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

set(SOURCES
    SciencePayload.cpp
    Heater.cpp
    Drill.cpp
    Microscope.cpp
    StepperMotor.cpp
    GenericMotor.cpp
    ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
  )

  add_library(${PROJECT_NAME} ${SOURCES})