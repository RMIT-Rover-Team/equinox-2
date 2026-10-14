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

# Public include paths for library and tests
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
        src/CanDevice.cpp
        ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
        ../lib-universal-canbus/libuniversalcan/CommandUtils.cpp
        ../lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
)

# Configure GoogleTest via FetchContent
FetchContent_Declare(
        googletest
        GIT_TAG v1.17.0
        GIT_REPOSITORY https://github.com/google/googletest.git
        EXCLUDE_FROM_ALL
        SYSTEM
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Build the SciencePayload library
add_library(SciencePayload STATIC ${SOURCES})

target_include_directories(SciencePayload PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

# Tests executable
add_executable(ScienceTests
        tests/DrillTests.cpp
        tests/GenericMotorTests.cpp
        tests/HeaterTests.cpp
        tests/MicroscopeTests.cpp
        tests/SciencePayloadTests.cpp
        tests/StepperMotorTests.cpp
        tests/CanDeviceTests.cpp
)

target_include_directories(ScienceTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

# Link tests against the compiled library and gtest
target_link_libraries(ScienceTests PRIVATE
    SciencePayload
    GTest::gtest_main
)

enable_testing()

gtest_discover_tests(ScienceTests)
