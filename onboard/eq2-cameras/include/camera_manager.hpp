#pragma once
#include <iostream>
#include <vector>
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

#define MAKE_CAM_ERROR(code, msg) \
  CamErrorDetails{code, msg, __FILE__, __LINE__}
    
#define MAKE_CAM_ERROR_EXPECTED(code, msg) \
  tl::make_unexpected(CamErrorDetails{code, msg, __FILE__, __LINE__})

namespace peel {
  template<>
  struct RefTraits<Gst::Caps> { // finish ref and unref for caps cause its unimplemented
    static constexpr bool can_ref_null = true;
    static constexpr bool can_unref_null = true;
    static void ref(Gst::Caps* ptr) { if (ptr) gst_caps_ref(reinterpret_cast<::GstCaps*>(ptr)); }
    static void unref(Gst::Caps* ptr) { if (ptr) gst_caps_unref(reinterpret_cast<::GstCaps*>(ptr)); }
    static void ref_sink(Gst::Caps* ptr) { ref(ptr); }
    static void sink(Gst::Caps* ptr) { }
  };
}

enum class CamError {
  MonitorCreationFailed,   // The monitor object couldn't be built
  CapsCreationFailed,      // The hardware filter (caps) couldn't be built
  BusWatchFailed,          // The main loop couldn't listen to the bus
  DeviceDestructionFailed, // The device couldn't be removed from the registry
  StreamDestructionFailed, // The stream couldn't be killed fully

  // Discovery / System (Components/States)
  MonitorBusNotFound,      // monitor->get_bus() returned null
  DevicePropertyNotFound,  // device property returned null

  // Hardware Registry (States)
  DeviceNotFound,          // UID isn't in the map
  HardwareInUse,           // The camera is locked by another app
  // Pipeline / Streaming (Actions)
  SourceCreationFailed,    // The v4l2src/pipewiresrc couldn't be built
  PipelineCreationFailed,  // The Gst::Pipeline couldn't be built
  AdditionFailed,          // Element couldn't be added to the pipeline
  LinkFailed,              // Elements can't link (caps mismatch)
  StateChangeFailed,        // State went to FAILURE (unplugged or driver crash)
  MonitorStartFailed       // Monitor object wouldn't start up
};

struct CamErrorDetails {
  CamError code;
  std::string message;
  std::string file;
  int line;

  std::string to_string() const {
    return "[Camera Error] " + message + 
           "\n  Code: " + std::to_string(static_cast<int>(code)) + 
           "\n  Location: " + file + ":" + std::to_string(line);
}
};

struct CameraHardware {
  std::string uid;  // camera serial
  std::string name;  // camera name
  std::string path;  // camera path
  peel::RefPtr<peel::Gst::Device> device;
  peel::RefPtr<peel::Gst::Caps> caps;
};

struct StreamInstance {
  std::string uid;
  peel::RefPtr<peel::Gst::Pipeline> pipeline;
  peel::RefPtr<peel::Gst::Element> source;
};

/**
 * @class CameraManager
 * @brief Manages hardware camera discovery and active GStreamer stream instances.
 * 
 * This class monitors the system for V4L2 devices and maintains a registry.
 * It allows users to request streams by UID and handles hot-plugging automatically.
 */
class CameraManager {
public:
  CameraManager();
  ~CameraManager();

  CameraManager(const CameraManager&) = delete;  // only have one instance
  CameraManager& operator=(const CameraManager&) = delete;
  
  /// @brief Start device monitor
  tl::expected<void, CamErrorDetails>  start_monitoring();

  /// @brief Stop device monitor
  tl::expected<void, CamErrorDetails> stop_monitoring();
  
  /// @brief Request a stream based off a device uid. If it doesn't exist, create it with the uid. 
  /// @param uid Device uid you would like to be streamed
  /// @return A pointer to the StreamInstance on success, on failure error message. 
  tl::expected<std::shared_ptr<StreamInstance>, CamErrorDetails> request_stream(const std::string&);

  const std::map<std::string, CameraHardware>& get_cameras() const { return registry_map_; }

private:
  /// @brief Setups device monitor
  /// @return Setup device monitor.
  tl::expected<peel::RefPtr<peel::Gst::DeviceMonitor>, CamErrorDetails> setup_device_monitor();

  /// @brief Adds a device to camera_registry.
  /// @param device Device to be added to camera_registry.
  tl::expected<void, CamErrorDetails> handle_device_add(const peel::RefPtr<peel::Gst::Device>&);
  
  /// @brief Removes a device from camera registry and kills streams associated with it
  /// @param uid uid of the camera to remove.
  tl::expected<void, CamErrorDetails> handle_device_remove(const std::string&);

  /// @brief Called every time the device monitor's bus sends a message (device added and device removed).
  /// @param bus The device monitor's bus for message handling.
  /// @param message The message delivered through the bus.
  /// @param user_data Unrelated data to be passed in.
  /// @return G_SOURCE_CONTINUE or G_SOURCE_REMOVE to continue or break the main loop.
  static gboolean bus_callback(GstBus*, GstMessage*, gpointer);

  /// @brief Create stream instance within the streams_ map. 
  /// @param camera camera to create stream with
  /// @return Returns a sharedptr stream instance, or the CamError
  tl::expected<std::shared_ptr<StreamInstance>, CamErrorDetails> create_stream(const CameraHardware&);

  peel::RefPtr<peel::Gst::DeviceMonitor> monitor_;
  std::map<std::string, std::shared_ptr<StreamInstance>> streams_;
  std::map<std::string, CameraHardware> registry_map_;
};
