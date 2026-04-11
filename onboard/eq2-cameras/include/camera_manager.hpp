#pragma once
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

class CameraManager {
public:
  CameraManager();
  ~CameraManager();

  CameraManager(const CameraManager&) = delete;  // only have one instance
  CameraManager& operator=(const CameraManager&) = delete;
  
  void start_monitoring();
  void stop_monitoring();
  
  const std::map<std::string, CameraHardware>& get_cameras() const { return registry_map_; }

private:
  peel::RefPtr<peel::Gst::DeviceMonitor> setup_device_monitor();

  void handle_add(const peel::RefPtr<peel::Gst::Device>& device);
  void handle_remove(const peel::RefPtr<peel::Gst::Device>& device);

  static gboolean bus_callback(GstBus* bus, GstMessage* msg, gpointer data);

  void create_stream(CameraHardware device);
  
  peel::RefPtr<peel::Gst::DeviceMonitor> monitor_;
  std::map<std::string, std::shared_ptr<StreamInstance>> streams_;
  std::map<std::string, CameraHardware> registry_map_;
};