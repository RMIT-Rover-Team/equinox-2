cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Arm VERSION 1.0
                  DESCRIPTION "Arm hardware interface"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

set(SOURCES
    src/ArmPayload.cpp
    src/ArmActuator.cpp
    src/EndEffector.cpp
    # TODO: Change libuniversalcan subtree to a submodule or a subrepo. Newer commits in that repo have a CMakeLists so we can import that library like any other
    ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
    ../lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
    ../lib-universal-canbus/libuniversalcan/CommandUtils.cpp
)

add_library(Arm ${SOURCES})



# #
# # SPDLOG
# #
# include(FetchContent)
# FetchContent_Declare(
#     spdlog
#     GIT_TAG v1.17.0
#     GIT_REPOSITORY https://github.com/gabime/spdlog.git
#     EXCLUDE_FROM_ALL
#     SYSTEM
# )

# FetchContent_MakeAvailable(spdlog)
# target_link_libraries(Arm PRIVATE spdlog::spdlog)



# #
# # TESTING
# #
# # include(FetchContent)
# FetchContent_Declare(
#     googletest
#     GIT_TAG v1.17.0
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     FIND_PACKAGE_ARGS 1.17.0 NAMES GTest COMPONENTS gmain_main
#     EXCLUDE_FROM_ALL
#     SYSTEM
# )

# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# enable_testing()



# add_executable(ArmTests
#     test/ArmTest.cpp
#     test/VCanTest.cpp
#     ${SOURCES}
# )

# target_include_directories(ArmTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# target_link_libraries(ArmTests PRIVATE 
#     GTest::gtest_main
#     GTest::gmock_main
#     spdlog::spdlog
# )

# include(GoogleTest)
# gtest_discover_tests(ArmTests)