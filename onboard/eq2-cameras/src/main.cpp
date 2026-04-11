#include <iostream>
#include <peel/GLib/MainLoop.h>
#include <device_manager.hpp>

using namespace peel;

int main(int argc, char *argv[]) {
  Gst::init(&argc, &argv);

  auto device_monitor = setup_device_monitor();
  
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