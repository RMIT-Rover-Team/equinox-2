#include "camera_manager.hpp"
#include <iostream>
#include <gst/gst.h>
#include <memory>

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

CameraManager::CameraManager() {}

CameraManager::~CameraManager() {
  stop_monitoring();
}

/// @brief Start device monitor
void CameraManager::start_monitoring() {
  monitor_ = setup_device_monitor();
  if (monitor_) monitor_->start();
}

/// @brief Stop device monitor
void CameraManager::stop_monitoring() {
  if (monitor_) {
    monitor_->stop();
    monitor_ = nullptr;
  }
}

/// @brief Called every time the device monitor's bus sends a message (device added and device removed).
/// @param bus The device monitor's bus for message handling.
/// @param message The message delivered through the bus.
/// @param user_data Unrelated data to be passed in.
/// @return G_SOURCE_CONTINUE or G_SOURCE_REMOVE to continue or break the main loop.
gboolean CameraManager::bus_callback(GstBus *bus, GstMessage *message, gpointer user_data) {
  auto* self = static_cast<CameraManager*>(user_data);
  auto* m = reinterpret_cast<Gst::Message*>(message);
  RefPtr<Gst::Device> device;

  switch (m->type) {
    case Gst::Message::Type::DEVICE_ADDED:
      m->parse_device_added(&device);
      self->handle_add(device);
      break;
    case Gst::Message::Type::DEVICE_REMOVED:
      m->parse_device_removed(&device);
      if (device) {
          std::cout << "Device removed: " << device->get_display_name() << std::endl;
      }
      break;
    default:
      break;
  }

  return G_SOURCE_CONTINUE;
}

/// @brief Adds a device to camera_registry.
/// @param device Device to be added to camera_registry
void CameraManager::handle_add(const RefPtr<Gst::Device>& device) {
  if (!device) {
    std::cout << "Ran into error with adding device, no device was found" << std::endl;
    return;
  }
  auto device_properties = device->get_properties();
  
  // std::cout << "Raw Structure: " << device_properties->to_string() << std::endl;  // check all properties
  
  std::string uid = device_properties->get_string("api.v4l2.cap.bus_info");
  
  std::string name = static_cast<std::string>(device->get_display_name());
  
  const char* path_val = device_properties->get_string("api.v4l2.path");
  std::string path = path_val ? path_val : "unknown";

  ::GstDevice* raw_device = reinterpret_cast<::GstDevice*>(static_cast<Gst::Device*>(device));
  ::GstCaps* raw_caps = gst_device_get_caps(raw_device);
  if (!raw_caps) {
    return;
  }
  auto caps = RefPtr<Gst::Caps>::adopt_ref(reinterpret_cast<Gst::Caps*>(raw_caps));

  const CameraHardware camera = {
    uid,
    name,
    path,
    device,
    caps
  };

  std::cout << "Adding Camera: " << camera.uid << ", " << camera.name << ", " << camera.path << std::endl;
  
  registry_map_[camera.uid] = std::move(camera);
}

/// @brief Setups device monitor
/// @return Setup device monitor.
RefPtr<Gst::DeviceMonitor> CameraManager::setup_device_monitor() {
  auto monitor = Gst::DeviceMonitor::create();
  if (!monitor) { return nullptr; }

  auto* c_caps = gst_caps_new_empty_simple("video/x-raw");  // new_empty_simple is not accepted yet, so cast
  monitor->add_filter("Video/Source", reinterpret_cast<Gst::Caps*>(c_caps));
  gst_caps_unref(c_caps);  // manual unref cause no smart pointer cause c

  auto bus = monitor->get_bus();
  gst_bus_add_watch(reinterpret_cast<GstBus*>(&*bus), bus_callback, this);

  return monitor;
}

/// @brief Create stream instance within the streams_ map. 
/// @param camera camera to create stream with
void CameraManager::create_stream(CameraHardware camera) {
  auto src_float_ptr = camera.device->create_element("source");
  auto pipeline = Gst::Pipeline::create(camera.name.c_str());
  
  peel::RefPtr<peel::Gst::Element> src_ref_ptr = src_float_ptr;  // make ref so we can assign it to stream
  pipeline->add(std::move(src_float_ptr));
  
  auto stream = std::make_shared<StreamInstance>();
  stream->uid = camera.uid;
  stream->pipeline = pipeline;
  stream->source = src_ref_ptr; 

  streams_[stream->uid] = stream;
}