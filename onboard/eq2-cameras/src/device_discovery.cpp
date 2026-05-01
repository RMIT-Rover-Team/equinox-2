#include "device_discovery.hpp"

using namespace peel;
using namespace equinox::camera;

DeviceDiscovery::DeviceDiscovery() {}

DeviceDiscovery::~DeviceDiscovery()
{
  auto result = stop_monitoring();
  if (!result.has_value()) spdlog::error(result.error().to_string()); 
  
  shutdown_worker();
}

tl::expected<void, CamErrorDetails> DeviceDiscovery::start_monitoring(DeviceRegistry* registry)
{
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    this->registry_ = registry;
    keep_running_ = true;
  }
  
  if (!worker_thread_.joinable())
  {
    worker_thread_ = std::thread(&DeviceDiscovery::worker_thread_loop, this);
  }

  auto result = setup_device_monitor();
  if (!result.has_value()) return tl::make_unexpected(result.error());
  
  monitor_ = *result;
  if (!monitor_->start()) return MAKE_CAM_ERROR_EXPECTED(CamError::MonitorStartFailed, "Device Monitor created but failed to start hardware scan.");

  spdlog::info("Started main monitor successfully");
  return {};
}

void DeviceDiscovery::worker_thread_loop()
{
  while (true)
  {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      cv_.wait(lock, [this] { return !task_queue_.empty() || !keep_running_; });

      if (!keep_running_ && task_queue_.empty()) return;
      
      if (!task_queue_.empty()) {
        task = std::move(task_queue_.front());
        task_queue_.pop();
      }
    }
    if (task) task();
  }
}

void DeviceDiscovery::shutdown_worker()
{
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    keep_running_ = false;
  }
  cv_.notify_all();
  if (worker_thread_.joinable())
  {
      worker_thread_.join();
  }
}

tl::expected<RefPtr<Gst::DeviceMonitor>, CamErrorDetails> DeviceDiscovery::setup_device_monitor()
{
  auto monitor = Gst::DeviceMonitor::create();
  if (!monitor) return MAKE_CAM_ERROR_EXPECTED(CamError::MonitorCreationFailed, "Failed to create device monitor.");

  auto caps = RefPtr<Gst::Caps>::adopt_ref(reinterpret_cast<Gst::Caps*>(gst_caps_new_empty_simple("video/x-raw")));
  if (!caps) return MAKE_CAM_ERROR_EXPECTED(CamError::CapsCreationFailed, "Failed to create caps.");
  monitor->add_filter("Video/Source", caps);

  auto bus = monitor->get_bus();
  if (!bus) return MAKE_CAM_ERROR_EXPECTED(CamError::BusWatchFailed, "Failed to get device monitor bus");
  gst_bus_add_watch(reinterpret_cast<GstBus*>(&*bus), bus_callback, this);

  return monitor;
}

tl::expected<void, CamErrorDetails> DeviceDiscovery::stop_monitoring()
{
  if (monitor_)
  {
    auto bus = monitor_->get_bus();
    if (!bus) return MAKE_CAM_ERROR_EXPECTED(CamError::MonitorBusNotFound, "monitor->get_bus() returned null");
    bus->remove_watch();

    monitor_->stop();
    monitor_ = nullptr;
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    registry_ = nullptr; 
    return {};
  }
  spdlog::warn("Failed to close monitor as monitor is not up.");
  return {};
}

gboolean DeviceDiscovery::bus_callback(GstBus *bus, GstMessage *message, gpointer user_data)
{
  auto* self = static_cast<DeviceDiscovery*>(user_data);
  {
    std::lock_guard<std::mutex> lock(self->queue_mutex_);
    if (!self->keep_running_) return G_SOURCE_CONTINUE;
    
    auto m = RefPtr<Gst::Message>::adopt_ref(reinterpret_cast<Gst::Message*>(gst_message_ref(message)));
    self->task_queue_.push([m, self]()
    {
      RefPtr<Gst::Device> device;

      auto* registry = self->registry_;
      if (!registry)
      {
        auto result = self->stop_monitoring();
        if (!result.has_value()) spdlog::error(MAKE_CAM_ERROR(CamError::MonitorStopFailed, "Monitor could not stop successfully.").to_string());
        spdlog::error(MAKE_CAM_ERROR(CamError::DeviceRegistryNotFound, "Device discovery could not find a registry").to_string());
        return;
      } 

      switch (m->type)
      {
        case Gst::Message::Type::DEVICE_ADDED:
        {
          m->parse_device_added(&device);
          if (!device)
          {
            spdlog::error(MAKE_CAM_ERROR(CamError::DevicePropertyNotFound, "Could not find device property bus_info.").to_string());
            return;
          }

          auto result = registry->handle_device_add(device);
          if (!result.has_value())
          {
            spdlog::error(result.error().to_string());
            return;
          }
          break;
          
        }
        case Gst::Message::Type::DEVICE_REMOVED:
        {
          m->parse_device_removed(&device);
          if (!device)
          {
            spdlog::error(MAKE_CAM_ERROR(CamError::DevicePropertyNotFound, "Could not find device property bus_info.").to_string());
            return;
          }
          
          auto result = registry->handle_device_remove(device);
          if (!result.has_value()) spdlog::error(result.error().to_string());
          
          break;
        }
        default:
        {
          break;
        }
      }
    });
  }
  self->cv_.notify_one();
  return G_SOURCE_CONTINUE;
}
