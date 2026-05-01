#ifndef CAMERA_REGISTRY_H
#define CAMERA_REGISTRY_H

#include <memory>
#include <iostream>
#include <map>
#include "camera_types.hpp"
#include <tl/expected.hpp>
#include <gst/gst.h>
#include <spdlog/spdlog.h>
#include <tl/expected.hpp>

class DeviceRegistry
{
  public:
    /// @brief Adds a device to camera_registry.
    /// @param device Device to be added to camera_registry.
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> handle_device_add(const peel::RefPtr<peel::Gst::Device>&);

    /// @brief Removes a device from camera registry and kills streams associated with it.
    /// @param uid uid of the camera to remove.
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> handle_device_remove(const std::string&);
    /// @brief Removes a device from camera registry and kills streams associated with it.
    /// @param uid Device object of the camera to remove.
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> handle_device_remove(peel::RefPtr<peel::Gst::Device> device);
    
  private:
    std::mutex registry_mutex_;
    std::map<std::string, equinox::camera::CameraHardware> registry_map_;     // device data with uid as key.
    std::map<peel::RefPtr<peel::Gst::Device>, std::string> device_uid_map_;   // if the uid can not be fetched from a device, use this map.
};
#endif