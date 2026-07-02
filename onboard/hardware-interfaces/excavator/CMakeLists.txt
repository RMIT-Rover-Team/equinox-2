cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Excavator VERSION 1.0
                  DESCRIPTION "Excavator hardware interface"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-universal-canbus/libuniversalcan
)

set(SOURCES
    src/ExcavatorPayload.cpp
    src/ExcavatorActuator.cpp
    src/TeethActuator.cpp
    src/PaverMagnet.cpp
  ../lib-universal-canbus/libuniversalcan/RoverCanMaster.cpp
  ../lib-universal-canbus/libuniversalcan/SocketCanWrapper.cpp
)


add_library(Excavator ${SOURCES})

set(spdlog_DIR ~/spdlog/build/)

find_package(spdlog REQUIRED)
target_link_libraries(Excavator PRIVATE spdlog::spdlog)
# make sure to run this to install spdlog
# git clone https://github.com/gabime/spdlog.git
# cd spdlog && mkdir build && cd build
# cmake .. && cmake --build .