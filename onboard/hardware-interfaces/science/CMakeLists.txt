cmake_minimum_required(VERSION 3.21)
include(FetchContent)

project(ScienceHardwareInterface
    VERSION 1.0
    DESCRIPTION "Science payload hardware interface"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Shared actuator building blocks; also brings in UniversalCAN::linux.
if(NOT TARGET Hardware::common)
    add_subdirectory(
        ../common
        ${CMAKE_CURRENT_BINARY_DIR}/common
    )
endif()

add_library(ScienceHardwareInterface STATIC
    src/Heater.cpp
    src/Drill.cpp
    src/DrillMotor.cpp
    src/Microscope.cpp
    src/Science.cpp
)

target_compile_features(ScienceHardwareInterface PUBLIC cxx_std_23)
target_compile_options(ScienceHardwareInterface PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(ScienceHardwareInterface
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ScienceHardwareInterface PUBLIC Hardware::common)


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
            tests/DrillMotorTests.cpp
            tests/DrillTests.cpp
            tests/HeaterTests.cpp
            tests/MicroscopeTests.cpp
            tests/ScienceTests.cpp
        )
        target_link_libraries(
            ScienceTests PRIVATE
            ScienceHardwareInterface
            GTest::gtest_main
        )
        gtest_discover_tests(ScienceTests DISCOVERY_MODE PRE_TEST)
    endif()
endif()
