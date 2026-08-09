// UNUSED


#pragma once
#include <thread>
#include <chrono>
// #include <mutex>
#include <atomic>

class USBMonitor {
private:
    std::thread monitor_thread;
    std::atomic<double> usb_usage = 0.0;
    std::atomic<bool> stop_thread = false;
    void thread_main();
public:
    USBMonitor();
    ~USBMonitor();
};
