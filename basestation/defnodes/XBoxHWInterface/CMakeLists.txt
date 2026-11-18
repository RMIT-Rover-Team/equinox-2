cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)

# Include SDL2
find_package(SDL2 REQUIRED)

# Add DEFCOM library from local folder
add_subdirectory(external/DEFCOM)

# Your executable
add_executable(XBoxHWInterface
    src/main.cpp
    src/SimpleController.cpp
)

# Link against DEFCOM
target_link_libraries(XBoxHWInterface
    PRIVATE
        DEFCOM
        SDL2::SDL2
)

# Include DEFCOM headers
target_include_directories(XBoxHWInterface
    PRIVATE
        external/DEFCOM/include
        include
)

# C++ standard
target_compile_features(XBoxHWInterface PUBLIC cxx_std_17)
