#include <iostream>
#include <peel/GLib/MainLoop.h>
#include <camera_manager.hpp>

using namespace peel;

int main(int argc, char *argv[]) {
  Gst::init(&argc, &argv);

  CameraManager manager;

  manager.start_monitoring();

  auto loop = GLib::MainLoop::create(nullptr, false);
  loop->run();

  manager.stop_monitoring();
  return 0;
}