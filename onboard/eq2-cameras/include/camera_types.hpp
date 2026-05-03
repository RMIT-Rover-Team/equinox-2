#ifndef CAMERA_TYPES_H
#define CAMERA_TYPES_H

#include <iostream>
#include <peel/Gst/Gst.h>
#include <tl/expected.hpp>

#define MAKE_CAM_ERROR(code, msg) \
  CamErrorDetails{code, msg, __FILE__, __LINE__}
    
#define MAKE_CAM_ERROR_EXPECTED(code, msg) \
  tl::make_unexpected(CamErrorDetails{code, msg, __FILE__, __LINE__})


namespace peel
{
  template<>
  struct RefTraits<Gst::Caps>
  { // finish ref and unref for caps cause its unimplemented
    static constexpr bool can_ref_null = true;
    static constexpr bool can_unref_null = true;
    static void ref(Gst::Caps* ptr) { if (ptr) gst_caps_ref(reinterpret_cast<::GstCaps*>(ptr)); }
    static void unref(Gst::Caps* ptr) { if (ptr) gst_caps_unref(reinterpret_cast<::GstCaps*>(ptr)); }
    static void ref_sink(Gst::Caps* ptr) { ref(ptr); }
    static void sink(Gst::Caps* ptr) { }
  };

  template <>
  struct RefTraits<Gst::Message>
  {
    static constexpr bool can_ref_null = true;
    static constexpr bool can_unref_null = true;
    static void ref(Gst::Message* ptr) { if (ptr) gst_message_ref(reinterpret_cast<::GstMessage*>(ptr)); }
    static void unref(Gst::Message* ptr) { if (ptr) gst_message_unref(reinterpret_cast<::GstMessage*>(ptr)); }
  };
}

namespace equinox::camera
{

  struct CameraHardware
  {
    std::string uid;                          // camera serial
    std::string name;                         // camera name
    std::string path;                         // camera path
    peel::RefPtr<peel::Gst::Device> device;   // Device object
    peel::RefPtr<peel::Gst::Caps> caps;       // Device capabilities
  };

  struct StreamInstance
  {
    std::string uid;
    peel::RefPtr<peel::Gst::Pipeline> pipeline;
    peel::RefPtr<peel::Gst::Element> source;
  };

  enum class CamError
  { 
    MonitorCreationFailed,   // The monitor object couldn't be built
    CapsCreationFailed,      // The hardware filter (caps) couldn't be built
    BusWatchFailed,          // The main loop couldn't listen to the bus
    DeviceDestructionFailed, // The device couldn't be removed from the registry
    StreamDestructionFailed, // The stream couldn't be killed fully

    // Discovery / System (Components/States)
    MonitorBusNotFound,       // monitor->get_bus() returned null
    DevicePropertyNotFound,   // device property returned null
    DeviceRegistryNotFound,   // device registry returned null
    StreamNotFound,           // Stream was not found
    // Hardware Registry (States)
    DeviceNotFound,          // UID isn't in the map
    HardwareInUse,           // The camera is locked by another app
    // Pipeline / Streaming (Actions)
    SourceCreationFailed,    // The v4l2src/pipewiresrc couldn't be built
    PipelineCreationFailed,  // The Gst::Pipeline couldn't be built
    AdditionFailed,          // Element couldn't be added to the pipeline
    LinkFailed,              // Elements can't link (caps mismatch)
    StateChangeFailed,       // State went to FAILURE (unplugged or driver crash)
    MonitorStartFailed,       // Monitor object wouldn't start up
    MonitorStopFailed        // Monitor object won't stop. How???
  };

  struct CamErrorDetails
  {
    CamError code;
    std::string message;
    std::string file;
    int line;

    /// @brief Converts CamErrorDetail to string.
    /// @return Error message string.
    std::string to_string() const
    {
      return "[Camera Error] " + message + 
            "\n  Code: " + std::to_string(static_cast<int>(code)) + 
            "\n  Location: " + file + ":" + std::to_string(line);
    }
  };
}

#endif