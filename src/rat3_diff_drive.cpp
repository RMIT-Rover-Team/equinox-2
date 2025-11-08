#include "rat3_diff_drive.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "hardware_interface/lexical_casts.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

/* 
RMIT Rover

Authors:
* [Software] Hasin Raihan Dhrubo

*/

// Adapted from ros2 control demo repository and bob_diff_drive

namespace diffdrive
{
    hardware_interface::CallbackReturn DiffDriveHardware::on_init(const hardware_interface::HardwareInfo & info)
    {
        // Base init
        if (hardware_interface::SystemInterface::on_init(info) 
            != hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }
        
        // Get CAN interface name from params
        auto CANinterfName = info_.hardware_parameters["canbus"];
        
        // Sanity-check joint count
        if (info_.joints.size() != 6)
        {
            RCLCPP_FATAL(
            rclcpp::get_logger("DiffDriveHardware"), "Expected 6 joints, but got %zu", info_.joints.size());
            return hardware_interface::CallbackReturn::ERROR;
        }

        // Resize state/command vectors
        size_t N = info_.joints.size();
        hw_positions_.assign(N, std::numeric_limits<double>::quiet_NaN());
        hw_velocities_.assign(N, std::numeric_limits<double>::quiet_NaN());
        hw_commands_.assign(N, std::numeric_limits<double>::quiet_NaN());
        
        // Validate that each joint only exposes a velocity interface
        for (const auto & joint : info_.joints)
        {
            if (joint.command_interfaces.size() != 1 ||
                joint.command_interfaces[0].name != hardware_interface::HW_IF_VELOCITY)
            {
            RCLCPP_FATAL(
                rclcpp::get_logger("DiffDriveHardware"),
                "Joint '%s' must have exactly one velocity command interface",
                joint.name.c_str());
            return hardware_interface::CallbackReturn::ERROR;
            }
        }
        
        // Instantiate the one-and-only ODriveController
        handler_ = std::make_unique<TorqueHandler>(CANinterfName.c_str());

        // Finally, do full ODrive setup
        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"),
                        "Initialized Drive.");

        return hardware_interface::CallbackReturn::SUCCESS;
    }
    
    
    std::vector<hardware_interface::StateInterface> DiffDriveHardware::export_state_interfaces()
    {
        std::vector<hardware_interface::StateInterface> state_interfaces;
        state_interfaces.reserve(info_.joints.size() * 2);

        for (size_t i = 0; i < info_.joints.size(); ++i) {
            const auto & joint_name = info_.joints[i].name;
            // position interface
            state_interfaces.emplace_back(joint_name,
                                        hardware_interface::HW_IF_POSITION,
                                        &hw_positions_[i]);
            // velocity interface
            state_interfaces.emplace_back(joint_name,
                                        hardware_interface::HW_IF_VELOCITY,
                                        &hw_velocities_[i]);
        }

        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"), "Exported %zu state interfaces", state_interfaces.size());
        return state_interfaces;
    }

    std::vector<hardware_interface::CommandInterface> DiffDriveHardware::export_command_interfaces()
    {
        std::vector<hardware_interface::CommandInterface> command_interfaces;
        command_interfaces.reserve(info_.joints.size());

        for (size_t i = 0; i < info_.joints.size(); ++i) {
            const auto & joint_name = info_.joints[i].name;
            command_interfaces.emplace_back(
            joint_name,
            hardware_interface::HW_IF_VELOCITY,
            &hw_commands_[i]
            );
        }

        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"),
                    "Exported %zu command interfaces",
                    command_interfaces.size());
        return command_interfaces;
    }

    hardware_interface::CallbackReturn DiffDriveHardware::on_activate(const rclcpp_lifecycle::State & /*previous_state*/)
    {
        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"), "Activating ...please wait...");
        // setup_drive();
        for (auto i = 0u; i < hw_velocities_.size(); i++)
        {
            if (std::isnan(hw_velocities_[i]))
            {
                hw_positions_[i] = 0;
                hw_velocities_[i] = 0;
                hw_commands_[i] = 0;
            }
        }
        // We should probably read the current state of the motors and save it back?
        // read needs rclcpp::Time and Duration and i dont know the purpose of those parameters (probably twist header)
        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"), "Successfully activated!");
        return hardware_interface::CallbackReturn::SUCCESS;
    }
    
    hardware_interface::CallbackReturn DiffDriveHardware::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/)
    {
        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"), "Deactivating ...please wait...");
        // od.~ODriveController() ?
        RCLCPP_INFO(rclcpp::get_logger("DiffDriveHardware"), "Successfully deactivated!");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    hardware_interface::return_type DiffDriveHardware::read(
        const rclcpp::Time & /*time*/, 
        const rclcpp::Duration & /*period*/)
        {
        // Zero everything first (in case of exceptions)
        std::fill(hw_positions_.begin(), hw_positions_.end(), 0.0);
        std::fill(hw_velocities_.begin(), hw_velocities_.end(), 0.0);

        try {
            // Grab a snapshot of odom (may throw if CAN map is empty)
            auto odom = handler_->getOdom();

            // Left wheels: joints 0,1,2
            for (size_t i = 0; i < info_.joints.size()/2; ++i) {
            hw_positions_[i]  = (odom.leftPos*M_PI*wheelDiam)/drive_ratio;
            hw_velocities_[i] = (odom.leftSpeed*M_PI*wheelDiam)/drive_ratio;
            }
            // Right wheels: joints 3,4,5
            for (size_t j = info_.joints.size()/2; j < info_.joints.size(); ++j) {
            hw_positions_[j]  = (odom.rightPos*M_PI*wheelDiam)/drive_ratio;
            hw_velocities_[j] = (odom.rightSpeed*M_PI*wheelDiam)/drive_ratio;
            }
        } 
        catch (const std::out_of_range & e) {
            // No CAN data yet for one or more wheels — we'll report zeros this cycle
            RCLCPP_WARN(rclcpp::get_logger("DiffDriveHardware"), "read(): no ODrive state yet (%s)", e.what());
        }

        return hardware_interface::return_type::OK;
    }


    hardware_interface::return_type DiffDriveHardware::write(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
    {
        handler_->setSpeed((hw_commands_[0]*drive_ratio)/(wheelDiam*M_PI), (hw_commands_[3]*drive_ratio)/(wheelDiam*M_PI));

        return hardware_interface::return_type::OK;
    }

}

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(diffdrive::DiffDriveHardware, hardware_interface::SystemInterface);
