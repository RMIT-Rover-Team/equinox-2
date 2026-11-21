cmake_minimum_required(VERSION 3.21)
include(FetchContent)

project(SciencePayload
  VERSION 1.0
  DESCRIPTION "Science payload hardware interface"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(UNIVERSALCAN_BUILD_UNICANMON OFF CACHE BOOL "" FORCE)
if(NOT TARGET UniversalCAN::linux)  # build if not working
    add_subdirectory(
        ../lib-universal-canbus
        ${CMAKE_CURRENT_BINARY_DIR}/universalcan
    )
endif()

add_library(ScienceHardwareInterface STATIC
    src/Heater.cpp
    src/Drill.cpp
    src/Microscope.cpp
    src/StepperMotor.cpp
    src/GenericMotor.cpp
    src/CanDevice.cpp
)

target_compile_features(ScienceHardwareInterface PUBLIC cxx_std_23)
target_compile_options(ScienceHardwareInterface PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(ScienceHardwareInterface
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ScienceHardwareInterface PUBLIC UniversalCAN::linux)


if(PROJECT_IS_TOP_LEVEL)  # build if in-project, otherwise don't.
    include(CTest)
    if(BUILD_TESTING)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.18.0
            EXCLUDE_FROM_ALL
            SYSTEM
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        include(GoogleTest)

        add_executable(ScienceTests
            tests/DrillTests.cpp
            tests/GenericMotorTests.cpp
            tests/HeaterTests.cpp
            tests/MicroscopeTests.cpp
            tests/SciencePayloadTests.cpp
            tests/StepperMotorTests.cpp
            tests/CanDeviceTests.cpp
        )
        target_link_libraries(
            ScienceTests PRIVATE
            SciencePayload
            GTest::gtest_main
        )
        gtest_discover_tests(ScienceTests)
    endif()
endif()
