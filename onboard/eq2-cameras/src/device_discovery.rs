use gstreamer as gst;
use gstreamer::prelude::*;
use std::sync::Arc;
use parking_lot::Mutex;
use crate::camera_types::CamError;
use crate::device_registry::DeviceRegistry;

pub struct DeviceDiscovery {
    monitor: Option<gst::DeviceMonitor>,
    registry: Arc<Mutex<DeviceRegistry>>,
    watch_id: Option<gst::bus::BusWatchGuard>,
}

impl DeviceDiscovery {
    pub fn new(registry: Arc<Mutex<DeviceRegistry>>) -> Self {
        Self { monitor: None, registry, watch_id: None }
    }

    pub fn start<F>(&mut self, on_changed: F) -> Result<(), CamError> 
    where 
        F: Fn(usize) + Send + Sync + 'static 
    {
        let monitor = gst::DeviceMonitor::new();
        let caps = gst::Caps::builder("video/x-raw").build();
        monitor.add_filter(Some("Video/Source"), Some(&caps));

        let bus = monitor.bus();
        let registry_clone = self.registry.clone();
        let on_changed = Arc::new(on_changed);

        let watch = bus.add_watch(move |_, msg| {
            use gst::MessageView;
            let mut changed = false;

            match msg.view() {
                MessageView::DeviceAdded(device_msg) => {
                    let device = device_msg.device();
                    let mut reg = registry_clone.lock();
                    if reg.handle_device_add(&device).is_ok() {
                        changed = true;
                    }
                }
                MessageView::DeviceRemoved(device_msg) => {
                    let device = device_msg.device();
                    let mut reg = registry_clone.lock();
                    if reg.handle_device_remove(&device).is_ok() {
                        changed = true;
                    }
                }
                _ => (),
            }

            if changed { // callback for bus updates
                let count = registry_clone.lock().get_device_count();
                on_changed(count);
            }

            glib::ControlFlow::Continue
        }).map_err(|_| CamError::MonitorError("Failed to add bus watch".into()))?;

        monitor.start().map_err(|_| CamError::MonitorError("Failed to start monitor".into()))?;
        self.monitor = Some(monitor);
        self.watch_id = Some(watch);
        Ok(())
    }

    pub fn stop(&mut self) {
        if let Some(monitor) = self.monitor.take() {
            monitor.stop();
        }
        // The watch_id (BusWatchGuard) automatically removes 
        // the watch from the loop when it is dropped.
        self.watch_id = None;
    }
}