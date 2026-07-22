cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ExcavatorTUI VERSION 1.0
                  DESCRIPTION "Excavator TUI"
                  LANGUAGES CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
    src/ExcavatorTUI.cpp
)

add_executable(ExcavatorTUI ${SOURCES})


