cmake_minimum_required(VERSION 3.20)
project(LEDHardwareInterface LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

add_library(led-hardware-interface
    src/APA102C.cpp
    src/WS2812B.cpp
)

target_compile_features(led-hardware-interface PUBLIC cxx_std_17)

target_include_directories(led-hardware-interface
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(led-hardware-interface PRIVATE
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
