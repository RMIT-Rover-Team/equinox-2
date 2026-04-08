#include <iostream>
#include <peel/Gst/Gst.h>

int main(int argc, char* argv[]) {
    peel::Gst::init(&argc, &argv);

    auto device_monitor = peel::Gst::DeviceMonitor::create();

    if (device_monitor) {
        // Use .get() to convert the peel::String into a standard const char* for cout
        std::cout << "Device monitor name " << device_monitor->get_name() << std::endl;
    }


  std::cout << "Running " <<  std::endl;
  return 0;
}