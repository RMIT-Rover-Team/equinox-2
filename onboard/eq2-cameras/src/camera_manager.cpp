// to do: add proper mutex to add and remove cameras
// adding and removing cameras should probably be multithreaded as to not block the bus from detecting new shit 

#include "camera_manager.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

using namespace peel;


CameraManager::CameraManager() {}


CameraManager::~CameraManager() {
  auto result = stop_monitoring();
  if (!result.has_value()) spdlog::error(result.error().to_string()); 
}


/// @brief Start device monitor
tl::expected<void, CamErrorDetails> CameraManager::start_monitoring() {
  auto result = setup_device_monitor();
  if (!result.has_value()) return tl::make_unexpected(result.error());
  
  monitor_ = *result;
  if (!monitor_->start()) return MAKE_CAM_ERROR_EXPECTED(CamError::MonitorStartFailed, "Device Monitor created but failed to start hardware scan.");

  spdlog::info("Started main monitor successfully");
  return {};
}


/// @brief Stop device monitor
tl::expected<void, CamErrorDetails> CameraManager::stop_monitoring() {
  if (monitor_) {
    auto bus = monitor_->get_bus();
    if (!bus) return MAKE_CAM_ERROR_EXPECTED(CamError::MonitorBusNotFound, "monitor->get_bus() returned null");
    bus->remove_watch();
    monitor_->stop();
    monitor_ = nullptr;
    return {};
  }
  spdlog::warn("Failed to close monitor as monitor is not up.");
  return {};
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
    {
      m->parse_device_added(&device);
      auto result = self->handle_device_add(device);
      if (!result.has_value()) spdlog::error(result.error().to_string());
      break;
    }
    case Gst::Message::Type::DEVICE_REMOVED:
    {
      m->parse_device_removed(&device);
      if (device) {
        auto props = device->get_properties();
        const char* bus_info = props->get_string("api.v4l2.cap.bus_info");
        if (!bus_info) {
          spdlog::error(MAKE_CAM_ERROR(CamError::DevicePropertyNotFound, "Could not find device property bus_info.").to_string());
          break;
        }

        auto result = self->handle_device_remove(bus_info);
        if (!result.has_value()) spdlog::error(result.error().to_string());
      }
      break;
    }
    default:
    {
      break;
    }
  }

  return G_SOURCE_CONTINUE;
}


/// @brief Adds a device to camera_registry.
/// @param device Device to be added to camera_registry.
tl::expected<void, CamErrorDetails> CameraManager::handle_device_add(const RefPtr<Gst::Device>& device) {
  if (!device) return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, "GStreamer bus delivered a null device pointer.");
  auto device_properties = device->get_properties();
  
  // std::cout << "Raw Structure: " << device_properties->to_string() << std::endl;  // check all properties
  
  // index by serial later?
  const char* bus_info = device_properties->get_string("api.v4l2.cap.bus_info");
  if (!bus_info) return MAKE_CAM_ERROR_EXPECTED(CamError::DevicePropertyNotFound, "Could not find device property bus_info.");
  std::string uid = bus_info;
  
  // name
  const std::string name = static_cast<std::string>(device->get_display_name());
  if (name.empty()) return MAKE_CAM_ERROR_EXPECTED(CamError::DevicePropertyNotFound, "Could not find device property display_name.");

  // path
  const char * dev_path = device_properties->get_string("api.v4l2.path");
  if (!dev_path) return MAKE_CAM_ERROR_EXPECTED(CamError::DevicePropertyNotFound, "Could not find device property device path.");
  std::string path = dev_path ? dev_path : "unknown";

  // caps
  ::GstDevice* raw_device = reinterpret_cast<::GstDevice*>(static_cast<Gst::Device*>(device));
  ::GstCaps* raw_caps = gst_device_get_caps(raw_device);
  if (!raw_caps) return MAKE_CAM_ERROR_EXPECTED(CamError::CapsCreationFailed, "Failed to create caps.");
  auto caps = RefPtr<Gst::Caps>::adopt_ref(reinterpret_cast<Gst::Caps*>(raw_caps));

  const CameraHardware camera = {
    uid,
    name,
    path,
    device,
    caps
  };

  spdlog::info("Adding Camera to registry: " + camera.uid + ", " + camera.name + ", " + camera.path);
  
  registry_map_.insert_or_assign(uid, std::move(camera));

  return {};
}


/// @brief Removes a device from camera registry and kills streams associated with it
/// @param uid uid of the camera to remove.
tl::expected<void, CamErrorDetails> CameraManager::handle_device_remove(const std::string& uid) {
  if (streams_.count(uid)) {
    spdlog::info("Cleaning up active stream for unplugged device " + uid + ".");

    streams_[uid]->pipeline->set_state(Gst::State::NULL_);  // synchronous so no need to check but be careful of blocking bus
    streams_.erase(uid);

    spdlog::info("Stream successfully destroyed");
  } else {
    spdlog::warn("Attempted to clean up stream for unplugged device " + uid + " but there were no active stream running.");
  }

  if (registry_map_.erase(uid) == 0) {
    return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, 
      "Could not find unplugged device " + uid + " in registry");
  }

  spdlog::info("Camera {} successfully removed from registry", uid);
  return {};
}


/// @brief Setups device monitor
/// @return Setup device monitor.
tl::expected<RefPtr<Gst::DeviceMonitor>, CamErrorDetails> CameraManager::setup_device_monitor() {
  auto monitor = Gst::DeviceMonitor::create();
  if (!monitor) { return MAKE_CAM_ERROR_EXPECTED(CamError::MonitorCreationFailed, "Failed to create device monitor."); }

  auto caps = RefPtr<Gst::Caps>::adopt_ref(reinterpret_cast<Gst::Caps*>(gst_caps_new_empty_simple("video/x-raw")));
  if (!caps) return MAKE_CAM_ERROR_EXPECTED(CamError::CapsCreationFailed, "Failed to create caps.");
  monitor->add_filter("Video/Source", caps);

  auto bus = monitor->get_bus();
  if (!bus) return MAKE_CAM_ERROR_EXPECTED(CamError::BusWatchFailed, "Failed to get device monitor bus");
  gst_bus_add_watch(reinterpret_cast<GstBus*>(&*bus), bus_callback, this);

  return monitor;
}


/// @brief Create stream instance within the streams_ map. 
/// @param camera camera to create stream with
/// @return Returns a sharedptr stream instance, or the CamError
tl::expected<std::shared_ptr<StreamInstance>, CamErrorDetails>
CameraManager::create_stream(const CameraHardware& camera) {
  auto src_float_ptr = camera.device->create_element("source");
  if (!src_float_ptr) return MAKE_CAM_ERROR_EXPECTED(CamError::SourceCreationFailed, "Failed to create source from device.");

  auto pipeline = Gst::Pipeline::create(camera.name.c_str());
  if (!pipeline) return MAKE_CAM_ERROR_EXPECTED(CamError::PipelineCreationFailed, "Failed to create pipeline.");
  
  peel::RefPtr<peel::Gst::Element> src_ref_ptr = src_float_ptr;  // make ref so we can assign it to stream
  if (!pipeline->add(std::move(src_float_ptr))) return MAKE_CAM_ERROR_EXPECTED(CamError::AdditionFailed, "Failed to add source element to pipeline.");;

  auto stream = std::make_shared<StreamInstance>();

  stream->uid = camera.uid;
  stream->pipeline = pipeline;
  stream->source = src_ref_ptr;

  streams_[stream->uid] = stream;
  return stream;
}

tl::expected<std::shared_ptr<StreamInstance>, CamErrorDetails> 
CameraManager::request_stream(const std::string& uid) {
  // find device
  auto it = registry_map_.find(uid);
  if (it == registry_map_.end()) {
    return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, "Could not find device in registry.");
  }

  // if active stream, return
  if (streams_.count(uid)) {
    return streams_[uid];
  }

  // if not active stream, create stream
  return create_stream(it->second);
}