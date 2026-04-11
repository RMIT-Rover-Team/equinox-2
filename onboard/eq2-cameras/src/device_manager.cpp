#include "device_manager.hpp"
#include <iostream>
#include <gst/gst.h>

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

using namespace peel;

RefPtr<Gst::Pipeline> g_pipeline;
std::vector<CameraHardware> camera_registry;

/// @brief Called every time the device monitor's bus sends a message.
/// @param bus The device monitor's bus for message handling.
/// @param message The message delivered through the bus.
/// @param user_data Unrelated data to be passed in.
/// @return G_SOURCE_CONTINUE or G_SOURCE_REMOVE to continue or remove the main loop.
static gboolean device_bus_function(GstBus *bus, GstMessage *message, gpointer user_data) {
  RefPtr<Gst::Device> device;
  auto* m = reinterpret_cast<Gst::Message*>(message);

  switch (m->type) {
    case Gst::Message::Type::DEVICE_ADDED:
      m->parse_device_added(&device);
      if (device) {
        CameraHardware camera;
        camera.name = device->get_display_name();
        
        auto device_properties = device->get_properties();
        const char* path_val = device_properties->get_string("api.v4l2.path");
        camera.path = path_val ? path_val : "unknown";
        
        camera.device = device;
        
        Gst::Device* wrapped_ptr = device; 
        ::GstDevice* raw_device = reinterpret_cast<::GstDevice*>(wrapped_ptr);
        ::GstCaps* raw_caps = gst_device_get_caps(raw_device);
        if (raw_caps) {
          Gst::Caps* wrapped_caps = reinterpret_cast<Gst::Caps*>(raw_caps);
          camera.caps = RefPtr<Gst::Caps>::adopt_ref(wrapped_caps);
        }
        
        camera_registry.push_back(std::move(camera));
      }
      break;
    case Gst::Message::Type::DEVICE_REMOVED:
      m->parse_device_removed(&device);
      if (device) {
          std::cout << "Device removed: " << device->get_display_name() << std::endl;
      }
      if (g_pipeline) {
          g_pipeline->set_state(Gst::State::NULL_);
          g_pipeline = nullptr;
          std::cout << "Stopped streaming." << std::endl;
      }
      break;
    default:
      break;
  }

  return G_SOURCE_CONTINUE;
}

/// @brief Setups device monitor
/// @return Setup device monitor.
RefPtr<Gst::DeviceMonitor> setup_device_monitor() {
  auto monitor = Gst::DeviceMonitor::create();
  if (!monitor) { return nullptr; }

  auto c_caps = gst_caps_new_empty_simple("video/x-raw");  // new_empty_simple is not accepted yet, so cast
  monitor->add_filter("Video/Source", reinterpret_cast<Gst::Caps*>(c_caps));
  gst_caps_unref(c_caps);  // manual unref cause no smart pointer cause c

  auto bus = monitor->get_bus();
  gst_bus_add_watch(reinterpret_cast<GstBus*>(&*bus), device_bus_function, nullptr);

  return monitor;
}