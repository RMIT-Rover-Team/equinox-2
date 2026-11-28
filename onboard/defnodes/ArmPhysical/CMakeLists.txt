cmake_minimum_required(VERSION 3.16)
project(MyNode LANGUAGES CXX)

# Add DEFCOM library from local folder
add_subdirectory(external/DEFCOM)

# Your executable
add_executable(ArmPhysical
    src/main.cpp
    src/MyActuatorMotor.cpp
    src/SocketCanWrapper.cpp
)

# Link against DEFCOM
target_link_libraries(ArmPhysical
    PRIVATE
        DEFCOM
)

# Include DEFCOM headers
target_include_directories(ArmPhysical
    PRIVATE
        external/DEFCOM/include
        include
)

# C++ standard
target_compile_features(ArmPhysical PUBLIC cxx_std_17)
