#pragma once
#include <vector>
#include <string>
#include <peel/Gst/Gst.h>
#include <peel/Gst/Device.h>
#include <peel/Gst/Caps.h>
#include <peel/Gst/Pipeline.h>
#include <peel/Gst/DeviceMonitor.h>

struct CameraHardware {
  std::string name;
  std::string path;
  peel::RefPtr<peel::Gst::Device> device;
  peel::RefPtr<peel::Gst::Caps> caps;
};

// Use full peel::Gst namespace here
extern peel::RefPtr<peel::Gst::Pipeline> g_pipeline;
extern std::vector<CameraHardware> camera_registry;

peel::RefPtr<peel::Gst::DeviceMonitor> setup_device_monitor();