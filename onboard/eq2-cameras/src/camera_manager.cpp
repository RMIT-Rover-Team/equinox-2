#include "camera_manager.hpp"
#include <iostream>

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
  auto result = setup_device_monitor();
  if (result) {
    monitor_ = *result;
    if (!monitor_->start()) {
      std::cerr << "Monitor found but failed to start hardware scan" << std::endl;
    } else {
      std::cout << "Started main monitor successfully" << std::endl;
    }
  } else {
    std::cerr << "Failed to intialize device monitor: " << static_cast<int>(result.error()) << std::endl;
  }
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
      self->handle_device_add(device);
      break;
    case Gst::Message::Type::DEVICE_REMOVED:
      m->parse_device_removed(&device);
      if (device) {
        auto props = device->get_properties();
        const char* bus_info = props->get_string("api.v4l2.cap.bus_info");
        
        if (bus_info) { self->handle_device_remove(bus_info); }
      }
      break;
    default:
      break;
  }

  return G_SOURCE_CONTINUE;
}


/// @brief Adds a device to camera_registry.
/// @param device Device to be added to camera_registry.
void CameraManager::handle_device_add(const RefPtr<Gst::Device>& device) {
  if (!device) {
    std::cout << "Ran into error with adding device, no device was found" << std::endl;
    return;
  }
  auto device_properties = device->get_properties();
  
  // std::cout << "Raw Structure: " << device_properties->to_string() << std::endl;  // check all properties
  
  // uid
  std::string uid; 
  const char* bus_info = device_properties->get_string("api.v4l2.cap.bus_info");
  const char * dev_path = device_properties->get_string("api.v4l2.path");
  if (bus_info) uid = bus_info;  // set uid to bus info
  else if (dev_path) uid = dev_path;  // if that doesn't exist, set it to /dev/video*

  // name
  std::string name = static_cast<std::string>(device->get_display_name());
  
  // path
  std::string path = dev_path ? dev_path : "unknown";

  // caps
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

  std::cout << "Adding Camera to registry: " << camera.uid << ", " << camera.name << ", " << camera.path << std::endl;
  
  registry_map_[camera.uid] = std::move(camera);
}


/// @brief Removes a device from camera registry and kills streams associated with it
/// @param uid uid of the camera to remove.
void CameraManager::handle_device_remove(const std::string& uid) {
  if (streams_.count(uid)) {
    std::cout << "Cleaning up active stream for unplugged device: " << uid << std::endl;
    streams_[uid]->pipeline->set_state(Gst::State::NULL_);
    streams_.erase(uid);
  }
  registry_map_.erase(uid);
}


/// @brief Setups device monitor
/// @return Setup device monitor.
tl::expected<RefPtr<Gst::DeviceMonitor>, CamError> CameraManager::setup_device_monitor() {
  auto monitor = Gst::DeviceMonitor::create();
  if (!monitor) { return tl::make_unexpected(CamError::MonitorCreationFailed); }

  auto* c_caps = gst_caps_new_empty_simple("video/x-raw");  // new_empty_simple is not accepted yet, so cast
  monitor->add_filter("Video/Source", reinterpret_cast<Gst::Caps*>(c_caps));
  gst_caps_unref(c_caps);  // manual unref cause no smart pointer cause c

  auto bus = monitor->get_bus();
  gst_bus_add_watch(reinterpret_cast<GstBus*>(&*bus), bus_callback, this);

  return monitor;
}


/// @brief Create stream instance within the streams_ map. 
/// @param camera camera to create stream with
/// @return Returns a sharedptr stream instance, or the CamError
tl::expected<std::shared_ptr<StreamInstance>, CamError>
CameraManager::create_stream(CameraHardware camera) {
  auto src_float_ptr = camera.device->create_element("source");
  if (!src_float_ptr) return tl::make_unexpected(CamError::SourceCreationFailed);

  auto pipeline = Gst::Pipeline::create(camera.name.c_str());
  if (!pipeline) return tl::make_unexpected(CamError::PipelineCreationFailed);
  
  peel::RefPtr<peel::Gst::Element> src_ref_ptr = src_float_ptr;  // make ref so we can assign it to stream
  if (!pipeline->add(std::move(src_float_ptr))) return tl::make_unexpected(CamError::AdditionToPipelineFailed);

  auto stream = std::make_shared<StreamInstance>();
  stream->uid = camera.uid;
  stream->pipeline = pipeline;
  stream->source = src_ref_ptr; 

  streams_[stream->uid] = stream;
  return stream;
}

tl::expected<std::shared_ptr<StreamInstance>, CamError> 
CameraManager::request_stream(const std::string& uid) {
  // find device
  auto it = registry_map_.find(uid);
  if (it == registry_map_.end()) {
      return tl::make_unexpected(CamError::DeviceNotFound);
  }

  // if active stream, return
  if (streams_.count(uid)) {
      return streams_[uid];
  }

  // if not active stream, create stream
  return create_stream(it->second);
}