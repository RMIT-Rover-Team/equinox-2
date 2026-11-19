cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)

# Add DEFCOM library from local folder
add_subdirectory(external/DEFCOM)

# Your executable
add_executable(ExcavatorInterface
    src/main.cpp
)

# Link against DEFCOM
target_link_libraries(ExcavatorInterface
    PRIVATE
        DEFCOM
)

# Include DEFCOM headers
target_include_directories(ExcavatorInterface
    PRIVATE
        external/DEFCOM/include
)

# C++ standard
target_compile_features(ExcavatorInterface PUBLIC cxx_std_17)
