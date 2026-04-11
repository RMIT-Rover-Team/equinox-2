cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Eq2Cameras VERSION 1.0
                  DESCRIPTION "Rewrite of Eq1 cameras for GStreamer in C++."
                  LANGUAGES CXX)

set(version "0.1.0") 
set(includedir "${CMAKE_CURRENT_SOURCE_DIR}/peel/include")
set(peel_gen_path "${CMAKE_CURRENT_SOURCE_DIR}/peel/peel-gen.py")

configure_file(
  "peel/cmake/peel-config.cmake.in" 
  "${CMAKE_CURRENT_BINARY_DIR}/peelConfig.cmake"
  @ONLY
)

include("${CMAKE_CURRENT_BINARY_DIR}/peelConfig.cmake")

peel_generate(Gst 1.0 RECURSIVE)
peel_generate(GstVideo 1.0)
peel_generate(GstApp 1.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-video-1.0 gstreamer-app-1.0)
find_package(Drogon REQUIRED)
find_package(GTest REQUIRED)

set(SOURCES
  src/main.cpp
  src/camera_manager.cpp
  include/camera_manager.hpp
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive)

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_BINARY_DIR}/peel-generated"
    "${CMAKE_CURRENT_SOURCE_DIR}/peel/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  peel::Gst 
  peel::GstVideo 
  peel::GstApp
  PkgConfig::GST 
  Drogon::Drogon
)