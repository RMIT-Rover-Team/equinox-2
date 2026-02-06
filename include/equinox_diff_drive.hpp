#pragma once

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/logger.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "odrive_control.hpp"
#include "odrive_wrapper.hpp"
#include "torque_handler.hpp"

/* 
RMIT Rover

Authors:
* [Software] Hasin Raihan Dhrubo

*/

// Adapted from ROS2 control demo repository


namespace diffdrive
{
  constexpr int numMotor = 6;
  constexpr int MotorID[MotorCount]  = {1, 2, 3, 4, 5, 6};
  constexpr int MotorDir[MotorCount] = {-1, -1, -1, 1, 1, 1};
  constexpr float wheelDiam = 0.24f;
  constexpr float drive_ratio = 70.0f;
  
  class DiffDriveHardware : public hardware_interface::SystemInterface
  {
  public:
    RCLCPP_SHARED_PTR_DEFINITIONS(DiffDriveHardware)

    hardware_interface::CallbackReturn on_init(
      const hardware_interface::HardwareInfo & info) override;

    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

    hardware_interface::CallbackReturn on_activate(
      const rclcpp_lifecycle::State & previous_state) override;

    hardware_interface::CallbackReturn on_deactivate(
      const rclcpp_lifecycle::State & previous_state) override;

    hardware_interface::return_type read(
      const rclcpp::Time & time, const rclcpp::Duration & period) override;

    hardware_interface::return_type write(
      const rclcpp::Time & time, const rclcpp::Duration & period) override;

    void setup_drive();

  private:

    // Objects for logging
    std::shared_ptr<rclcpp::Logger> logger_;
    rclcpp::Clock::SharedPtr clock_;

    // Store the command for the simulated robot
    std::vector<double> hw_commands_;
    std::vector<double> hw_positions_;
    std::vector<double> hw_velocities_;

    std::unique_ptr<TorqueHandler> handler_;
  };

}
