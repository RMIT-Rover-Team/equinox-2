cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)

# Add DEFCOM library from local folder
add_subdirectory(external/DEFCOM)

# Add HW Interface
if(NOT TARGET ExcavatorHardwareInterface)
    add_subdirectory(
        ../../hardware-interfaces/excavator
        ${CMAKE_CURRENT_BINARY_DIR}/excavator-hi
    )
endif()

# Your executable
add_executable(ExcavatorInterface
    src/main.cpp
)

# Link against DEFCOM
target_link_libraries(ExcavatorInterface
    PRIVATE
        ExcavatorHardwareInterface
        DEFCOM
)

# Include DEFCOM headers
target_include_directories(ExcavatorInterface
    PRIVATE
        external/DEFCOM/include
)

# C++ standard
target_compile_features(ExcavatorInterface PUBLIC cxx_std_17)
