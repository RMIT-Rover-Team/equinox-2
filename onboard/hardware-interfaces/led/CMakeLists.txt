cmake_minimum_required(VERSION 3.20)
project(LEDHardwareInterface LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
# pkg_check_modules(GPIODCXX REQUIRED IMPORTED_TARGET libgpiodcxx)

add_library(led-interface
    src/Led.cpp
)

# target_link_libraries(led-interface
#     PRIVATE
#         PkgConfig::GPIODCXX

# )

target_compile_features(led-interface PRIVATE cxx_std_17)