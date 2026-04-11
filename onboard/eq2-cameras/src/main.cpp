#include <iostream>
#include <peel/Gst/Gst.h>
#include <peel/GLib/MainLoop.h>
#include <gst/gst.h>

using namespace peel;

RefPtr<Gst::Pipeline> g_pipeline;

FloatPtr<Gst::Element> check_hardware_convert(const char* name) {
  FloatPtr<Gst::Element> element = Gst::ElementFactory::make("v4l2convert", name);

  if (!element) {
    std::cout << "v4l2convert not found, falling back to software videoconvert" << std::endl;
    element = Gst::ElementFactory::make("videoconvert", name);
  }
  return element;
}

FloatPtr<Gst::Element> check_h264_hardware_encoding(const char* name) {
    auto element = Gst::ElementFactory::make("v4l2h264enc", name); // Pi 4
    if (element) {
      std::cout << "Encoding as v4l2h264enc" << std::endl;
      return element;
    }
    std::cout << "Could not encode as v4l2h264enc (You are probably not on the pi 4)" << std::endl;
    element = Gst::ElementFactory::make("vaapih264enc", name); // Intel PC
    if (element) {
      std::cout << "Encoding as vaapih264enc" << std::endl;
      return element;
    }
    if (!element) {
        std::cout << "No HW encoder found, falling back to software (x264enc)" << std::endl;
        element = Gst::ElementFactory::make("x264enc", name); // Software (Everywhere)
    }
    return element;
}

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
        // Get device details
        String device_name = device->get_display_name();
        
        UniquePtr<Gst::Structure> device_properties = device->get_properties();
        const char* path_ptr = device_properties->get_string("api.v4l2.path");
        std::string device_path = (path_ptr != nullptr) ? path_ptr : "N/A (Virtual or Unknown)";

        // std::cout << "Raw Structure: " << device_properties->to_string() << std::endl;  // check all properties

        std::cout << "Device added: " << device_name << std::endl;
        std::cout << "Device path: " << device_path << std::endl;
        
        FloatPtr<Gst::Element> src = device->create_element("src");  // generates frames
        FloatPtr<Gst::Element> filter = Gst::ElementFactory::make("capsfilter", "filter");  // filters to Video/Source
        FloatPtr<Gst::Element> conv = check_hardware_convert("conv");
        FloatPtr<Gst::Element> enc = check_h264_hardware_encoding("encoder");
        FloatPtr<Gst::Element> que = Gst::ElementFactory::make("queue", "que");
        // FloatPtr<Gst::Element> conv = Gst::ElementFactory::make("videoconvert", "conv");
        FloatPtr<Gst::Element> sink = Gst::ElementFactory::make("autovideosink", "sink");

        if (src && filter && que && conv && sink) {
          ::GstCaps* raw_filter_caps = gst_caps_from_string("video/x-raw, width=640, height=480, framerate=30/1");
          g_object_set(reinterpret_cast<::GObject*>(&*filter), "caps", raw_filter_caps, NULL);
          gst_caps_unref(raw_filter_caps);
          
          // Get raw pointers for linking before we move ownership
          Gst::Element* r_src = &*src;
          Gst::Element* r_filter = &*filter;
          Gst::Element* r_que = &*que;
          Gst::Element* r_conv = &*conv;
          Gst::Element* r_sink = &*sink;

          // Add to pipeline (must use std::move for FloatPtr)
          g_pipeline->add(std::move(src));
          g_pipeline->add(std::move(filter));
          g_pipeline->add(std::move(que));
          g_pipeline->add(std::move(conv));
          g_pipeline->add(std::move(sink));

          // 4. Link: src -> conv -> sink
          r_src->link(r_filter);
          r_filter->link(r_que);
          r_que->link(r_conv);
          r_conv->link(r_sink);

          // 5. Play
          g_pipeline->set_state(Gst::State::PLAYING);
          std::cout << "Streaming started!" << std::endl;
        }
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
  RefPtr<Gst::DeviceMonitor> monitor;
  RefPtr<Gst::Bus> bus;
  GstCaps* c_caps;

  monitor = Gst::DeviceMonitor::create();
  if (!monitor) { return nullptr; }

  c_caps = gst_caps_new_empty_simple("video/x-raw");  // new_empty_simple is not accepted yet, so cast
  monitor->add_filter("Video/Source", reinterpret_cast<Gst::Caps*>(c_caps));
  gst_caps_unref(c_caps);  // manual unref cause no smart pointer cause c

  bus = monitor->get_bus();
  gst_bus_add_watch(reinterpret_cast<GstBus*>(&*bus), device_bus_function, nullptr);

  return monitor;
}

int main(int argc, char *argv[]) {
  Gst::init(&argc, &argv);

  RefPtr<Gst::DeviceMonitor> device_monitor = setup_device_monitor();
  
  g_pipeline = Gst::Pipeline::create("pipeline");
  if (!device_monitor->start()) {
      std::cerr << "Failed to start monitor" << std::endl;
      return -1;
  }

  std::cout << "Device monitor name: " << device_monitor->get_name() << std::endl;
  std::cout << "Running... (Ctrl+C to stop)" << std::endl;
  
  auto loop = GLib::MainLoop::create(nullptr, false);
  loop->run();

  device_monitor->stop();
  return 0;
}