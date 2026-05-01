#include <iostream>
#include <peel/GLib/MainLoop.h>
#include <device_discovery.hpp>
#include <device_registry.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iomanip>

using namespace peel;

int main(int argc, char *argv[]) {
  Gst::init(&argc, &argv);

  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);

  std::ostringstream oss;
  oss << "logs/" << std::put_time(&now_tm, "%Y-%m-%d-%H-%M-%S") << "-cameras.log";
  std::string path = oss.str();

  // setup logger
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
    
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(1));
    
    spdlog::info("--- Eq2Cameras Session Started ---");
  } catch (const spdlog::spdlog_ex& ex){
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
      return 1;
  }
  DeviceDiscovery manager;
  DeviceRegistry registry;

  auto start_monitor_result = manager.start_monitoring(&registry);

  if (!start_monitor_result.has_value()) spdlog::error(start_monitor_result.error().to_string());
  
  auto loop = GLib::MainLoop::create(nullptr, false);
  loop->run();

  auto stop_monitor_result = manager.stop_monitoring();
  if (!stop_monitor_result.has_value()) spdlog::error(stop_monitor_result.error().to_string());

  spdlog::shutdown(); 
  return 0;
}