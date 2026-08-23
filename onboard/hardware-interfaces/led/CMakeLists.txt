cmake_minimum_required(VERSION 3.20)
project(LEDHardwareInterface LANGUAGES C CXX)

find_package(Threads REQUIRED)

set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_TEST OFF CACHE BOOL "" FORCE)
# rpi_ws281x still declares CMake 3.0 compatibility, which CMake 4 removed.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(external/rpi_ws281x EXCLUDE_FROM_ALL)

add_library(led-hardware-interface
    src/APA102C.cpp
    src/WS2812B.cpp
)

target_compile_features(led-hardware-interface PUBLIC cxx_std_17)

target_include_directories(led-hardware-interface
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/rpi_ws281x>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/external/rpi_ws281x>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(led-hardware-interface
    PUBLIC
        ws2811
    PRIVATE
        Threads::Threads
)

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        FIND_PACKAGE_ARGS 1.17.0 NAMES GTest
        EXCLUDE_FROM_ALL
        SYSTEM
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()
