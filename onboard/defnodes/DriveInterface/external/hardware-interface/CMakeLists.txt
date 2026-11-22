cmake_minimum_required(VERSION 3.16)
project(hardwareInterface LANGUAGES CXX)

# Keep the established CAN implementation as the default hardware target.
option(DRIVE_USE_MINIROVER_GPIO "Use the mini-rover libgpiod drive backend" OFF)

# Collect all source files
file(GLOB_RECURSE hardwareInterface_SOURCES src/*.cpp)

list(REMOVE_ITEM hardwareInterface_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pyTorqueWrapper.cpp
)

if(NOT DRIVE_USE_MINIROVER_GPIO)
    # Do not require libgpiod for normal CAN-only builds.
    list(REMOVE_ITEM hardwareInterface_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/minirover_gpio.cpp
    )
endif()


# Build the library
add_library(hardwareInterface SHARED ${hardwareInterface_SOURCES})

# Public include directory
target_include_directories(hardwareInterface
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)



# C++ standard
target_compile_features(hardwareInterface PUBLIC cxx_std_17)

if(DRIVE_USE_MINIROVER_GPIO)
    # libgpiod v2 exposes the maintained Linux GPIO character-device API.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBGPIOD REQUIRED IMPORTED_TARGET libgpiod)
    # Propagate the selected backend to TorqueHandler and DriveInterface's entry point.
    target_compile_definitions(hardwareInterface PUBLIC TORQUE_HANDLER_USE_MINIROVER_GPIO=1)
    target_link_libraries(hardwareInterface PUBLIC PkgConfig::LIBGPIOD)
endif()

# Install 
install(TARGETS hardwareInterface
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include)
