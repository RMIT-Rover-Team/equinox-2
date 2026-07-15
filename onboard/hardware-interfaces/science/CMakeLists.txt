cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(SciencePayload
  VERSION 1.0
  DESCRIPTION "Science payload hardware interface"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SOURCES
        src/SciencePayload.cpp
        src/Heater.cpp
        src/Drill.cpp
        src/Microscope.cpp
        src/StepperMotor.cpp
        src/GenericMotor.cpp
    ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
  )

FetchContent_Declare(
        googletest
        GIT_TAG v1.17.0
        GIT_REPOSITORY https://github.com/google/googletest.git
        FIND_PACKAGE_ARGS 1.17.0 NAMES GTest COMPONENTS gmain_main
        EXCLUDE_FROM_ALL
        SYSTEM
)

add_executable(ScienceTests
        tests/DrillTests.cpp
        tests/GenericMotorTests.cpp
        tests/HeaterTests.cpp
        tests/MicroscopeTests.cpp
        tests/SciencePayloadTests.cpp
        tests/StepperMotorTests.cpp
)

target_include_directories(ScienceTests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ScienceTests PRIVATE 
    GTest::gtest_main
    GTest::gmock_main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

enable_testing()

  add_library(${PROJECT_NAME} ${SOURCES})
gtest_discover_tests(ScienceTests)
