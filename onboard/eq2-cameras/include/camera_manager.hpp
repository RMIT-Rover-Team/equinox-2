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
#include <tl/expected.hpp>

enum class CamError {
  SourceCreationFailed,
  PipelineCreationFailed,
  AdditionToPipelineFailed,
  DeviceNotFound,
  MonitorCreationFailed,
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
  tl::expected<peel::RefPtr<peel::Gst::DeviceMonitor>, CamError> setup_device_monitor();

  void handle_device_add(const peel::RefPtr<peel::Gst::Device>&);
  void handle_device_remove(const std::string&);

  static gboolean bus_callback(GstBus*, GstMessage*, gpointer);

  tl::expected<std::shared_ptr<StreamInstance>, CamError> create_stream(CameraHardware);
  tl::expected<std::shared_ptr<StreamInstance>, CamError> request_stream(const std::string&);

  peel::RefPtr<peel::Gst::DeviceMonitor> monitor_;
  std::map<std::string, std::shared_ptr<StreamInstance>> streams_;
  std::map<std::string, CameraHardware> registry_map_;
};