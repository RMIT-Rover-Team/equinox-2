cmake_minimum_required(VERSION 3.8)
project(rmit_rat3_hardware_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

find_package(controller_manager)
find_package(backward_ros REQUIRED)
find_package(ament_cmake REQUIRED)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)


add_library(diffdrive SHARED 
  src/rat3_diff_drive.cpp
  src/odrive_wrapper.cpp
  src/odrive_controllers.cpp
  src/wheel_speed_cal.cpp
  src/linear_calc.cpp
  src/fake_wrapper.cpp
  src/pid_calc.cpp
  src/torque_handler.cpp
)
# add_executable(diffdrive ${CMAKE_CURRENT_SOURCE_DIR}/include/ src/rat3_diff_drive.cpp)
target_compile_features(diffdrive PUBLIC cxx_std_17)  # We arent compiling with c99 :skull
target_include_directories(diffdrive PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/>)

ament_target_dependencies(
  diffdrive 
  ${THIS_PACKAGE_INCLUDE_DEPENDS}
)



pluginlib_export_plugin_description_file(hardware_interface diffdrive.xml)
install(
  DIRECTORY include/
  DESTINATION include/rmit_rat3_hardware_interface
)
install(TARGETS diffdrive
  EXPORT export_diffdrive
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)




ament_export_targets(export_diffdrive HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})


ament_package()
