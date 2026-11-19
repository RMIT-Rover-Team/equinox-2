cmake_minimum_required(VERSION 3.21)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ExcavatorHardwareInterface
    VERSION 1.0
    DESCRIPTION "Excavator hardware interface"
    LANGUAGES CXX
)

set(UNIVERSALCAN_BUILD_UNICANMON OFF CACHE BOOL "" FORCE)
if(NOT TARGET UniversalCAN::linux)  # build universal can
    add_subdirectory(
        ../lib-universal-canbus
        ${CMAKE_CURRENT_BINARY_DIR}/universalcan
    )
endif()


add_library(ExcavatorHardwareInterface STATIC
    src/ExcavatorPayload.cpp
    src/ExcavatorActuator.cpp
    src/TeethActuator.cpp
    src/PaverMagnet.cpp
)

target_compile_features(ExcavatorHardwareInterface PUBLIC cxx_std_23)
target_compile_options(ExcavatorHardwareInterface PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
target_include_directories(ExcavatorHardwareInterface
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ExcavatorHardwareInterface PUBLIC UniversalCAN::linux)


#
# SPDLOG
#
FetchContent_Declare(
    spdlog
    GIT_TAG v1.17.0
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(spdlog)
# PUBLIC: include/PaverMagnet.h includes spdlog
target_link_libraries(ExcavatorHardwareInterface PUBLIC spdlog::spdlog)



#
# TESTING (only when built on its own, not when a TUI pulls this in)
#
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if(BUILD_TESTING)
        FetchContent_Declare(
            googletest
            GIT_TAG v1.17.0
            GIT_REPOSITORY https://github.com/google/googletest.git
            FIND_PACKAGE_ARGS 1.17.0 NAMES GTest
            EXCLUDE_FROM_ALL
            SYSTEM
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)

        # TODO: re-enable once test/ is ported to universalcan v3
        # add_executable(ExcavatorTests
        #     test/ExcavatorTest.cpp
        #     test/VCanTest.cpp
        # )
        # target_link_libraries(ExcavatorTests PRIVATE
        #     ExcavatorHardwareInterface
        #     GTest::gtest_main
        #     GTest::gmock_main
        # )
        # include(GoogleTest)
        # gtest_discover_tests(ExcavatorTests DISCOVERY_MODE PRE_TEST)
    endif()
endif()
