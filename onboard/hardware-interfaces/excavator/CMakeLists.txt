cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Excavator VERSION 1.0
                  DESCRIPTION "Excavator actuator interface"
                  LANGUAGES CXX)

set(SOURCES
  ExcavatorActuator.cpp
  ExcavatorActuator.h
  ../lib-universal-canbus/libuniversalcan/GenericCan.h
  ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
  ../lib-universal-canbus/libuniversalcan/RoverCanMaster.h
)

add_executable(${PROJECT_NAME} ${SOURCES})