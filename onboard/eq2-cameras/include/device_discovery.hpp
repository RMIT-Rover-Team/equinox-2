#ifndef CAMERA_DISCOVERY_H
#define CAMERA_DISCOVERY_H

#include <iostream>
#include <string>
#include <gst/gst.h>
#include <peel/Gst/Gst.h>
#include <peel/Gst/Device.h>
#include <peel/Gst/Caps.h>
#include <peel/Gst/Pipeline.h>
#include <peel/Gst/DeviceMonitor.h>
#include <map>
#include <memory>
#include <tl/expected.hpp>
#include "spdlog/spdlog.h"
#include "camera_types.hpp"
#include "device_registry.hpp"
#include <queue>
#include <condition_variable>

class DeviceDiscovery
{
  public:
    DeviceDiscovery();
    ~DeviceDiscovery();

    DeviceDiscovery(const DeviceDiscovery&) = delete;  // only have one instance
    DeviceDiscovery& operator=(const DeviceDiscovery&) = delete;
    /// @brief Start device monitor
    /// @param registry Device Registry object to store device information.
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> start_monitoring(DeviceRegistry*);

    /// @brief Stop device monitor
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> stop_monitoring();
  private:
    /// @brief Setups device monitor
    /// @return Setup device monitor.
    /// @return If successful, returns the DeviceMonitor object. If it runs into an error, returns CamErrorDetails.
    tl::expected<peel::RefPtr<peel::Gst::DeviceMonitor>, equinox::camera::CamErrorDetails> setup_device_monitor();

    /// @brief Adds a device to camera_registry.
    /// @param device Device to be added to camera_registry.
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> handle_device_add(const peel::RefPtr<peel::Gst::Device>&);
    
    /// @brief Removes a device from camera registry and kills streams associated with it.
    /// @param uid uid of the camera to remove.
    /// @return If successful, doesn't return anything. If it runs into an error, returns CamErrorDetails.
    tl::expected<void, equinox::camera::CamErrorDetails> handle_device_remove(const std::string&);

    /// @brief Called every time the device monitor's bus sends a message (device added and device removed).
    /// @param bus The device monitor's bus for message handling.
    /// @param message The message delivered through the bus.
    /// @param user_data Pass in whatever data you want into the function, such as the device discovery object (self).
    /// @return G_SOURCE_CONTINUE or G_SOURCE_REMOVE to continue or break the main loop.
    static gboolean bus_callback(GstBus*, GstMessage*, gpointer);
    
    /// @brief Generates worker thread that stays asleep until task is assigned to it (from device monitor bus).
    void worker_thread_loop();
    /// @brief Kills worker loop.
    void shutdown_worker();
        
    // GStreamer related
    DeviceRegistry* registry_ = nullptr;
    peel::RefPtr<peel::Gst::DeviceMonitor> monitor_;

    // Thread stuff
    std::mutex queue_mutex_;
    std::queue<std::function<void ()>> task_queue_;
    std::thread worker_thread_;
    std::condition_variable cv_;
    bool keep_running_ = true;

};
#endif
