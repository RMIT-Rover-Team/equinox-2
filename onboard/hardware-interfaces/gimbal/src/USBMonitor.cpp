// UNUSED


#include "USBMonitor.h"


USBMonitor::USBMonitor() {
    monitor_thread = std::thread(&USBMonitor::thread_main, this);
}

USBMonitor::~USBMonitor() {
    stop_thread = true;
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }
}

void USBMonitor::thread_main() {
    while (!stop_thread) {
         

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}