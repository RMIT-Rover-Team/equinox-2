cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)

add_subdirectory(external/DEFCOM)

add_subdirectory(
    ../../hardware-interfaces/drive
    hardwareInterface_build
)

add_executable(DriveInterface
    src/main.cpp
)

target_link_libraries(DriveInterface
    PRIVATE
        DEFCOM
        hardwareInterface
)

target_include_directories(DriveInterface
    PRIVATE
        external/DEFCOM/include
        include
)

target_compile_features(DriveInterface PUBLIC cxx_std_17)