cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)



# Add DEFCOM library from local folder
add_subdirectory(external/DEFCOM)

# Your executable
add_executable(ArmXBControl
    src/main.cpp
)

# Link against DEFCOM
target_link_libraries(ArmXBControl
    PRIVATE
        DEFCOM
)

# Include DEFCOM headers
target_include_directories(ArmXBControl
    PRIVATE
        external/DEFCOM/include
        include
)

# C++ standard
target_compile_features(ArmXBControl PUBLIC cxx_std_17)
