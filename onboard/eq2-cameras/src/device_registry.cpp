#include "device_registry.hpp"

using namespace peel;
using namespace equinox::camera;

tl::expected<void, CamErrorDetails> DeviceRegistry::handle_device_add(const RefPtr<Gst::Device>& device)
{
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
  
  {
    std::lock_guard<std::mutex> lock(registry_mutex_);
    registry_map_.insert_or_assign(uid, std::move(camera));
    device_uid_map_.insert_or_assign(device, uid);
  }
  
  return {};
}

tl::expected<void, CamErrorDetails> DeviceRegistry::handle_device_remove(const std::string& uid)
{
  std::lock_guard<std::mutex> lock(registry_mutex_);

  auto it = registry_map_.find(uid);
  
  if (it == registry_map_.end())
  {
    return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, 
      "Could not find unplugged device " + uid + " in registry");
  }

  device_uid_map_.erase(it->second.device);

  registry_map_.erase(it);

  spdlog::info("Camera {} successfully removed from registry", uid);
  return {};
}


tl::expected<void, CamErrorDetails> DeviceRegistry::handle_device_remove(RefPtr<Gst::Device> device)
{
  std::string uid;
  {
    std::lock_guard<std::mutex> lock(registry_mutex_);
    auto it = device_uid_map_.find(device);
    if (it == device_uid_map_.end()) {
        return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, "Device handle not found in mapping");
    }
    uid = it->second;
    
    device_uid_map_.erase(it);
    if (registry_map_.erase(uid) == 0 )
    {
      return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, 
      "Could not find unplugged device " + uid + " in registry");
    };

  }

  spdlog::info("Camera {} successfully removed from registry", uid);
  return {};
}