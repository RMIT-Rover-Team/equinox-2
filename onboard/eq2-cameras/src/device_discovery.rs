//! Device discovery module using GStreamer's `DeviceMonitor`.
//!
//! This module provides the infrastructure to listen for hardware changes (hotplugging)
//! and keep the global [`DeviceRegistry`] synchronized with the system's physical devices.

use gstreamer as gst;
use gstreamer::prelude::*;
use std::sync::Arc;
use parking_lot::Mutex;
use crate::camera_types::CamError;
use crate::camera_types::DiscoveryEvent;
use crate::device_registry::DeviceRegistry;

/// Manages the lifecycle of the GStreamer `DeviceMonitor`.
/// 
/// It watches the system for `Video/Source` devices and updates the provided 
/// [`DeviceRegistry`] whenever a camera is plugged in or removed.
pub struct DeviceDiscovery {
    monitor: Option<gst::DeviceMonitor>,
    registry: Arc<Mutex<DeviceRegistry>>,
    watch_id: Option<gst::bus::BusWatchGuard>,
}

impl DeviceDiscovery {
    /// Creates a new discovery service but does not start monitoring.
    ///
    /// # Arguments
    /// * `registry` - An Arc-wrapped Mutex to the central [`DeviceRegistry`].
    pub fn new(registry: Arc<Mutex<DeviceRegistry>>) -> Self {
        Self { monitor: None, registry, watch_id: None }
    }

    /// Starts the background monitoring process.
    ///
    /// This function attaches a listener to the GStreamer bus. Whenever a device 
    /// event occurs, the provided `on_event` callback is triggered.
    ///
    /// # Arguments
    /// * `on_event` - A closure that receives the updated device count: `Fn(usize)`.
    ///
    /// # Errors
    /// Returns [`CamError::MonitorError`] if the GStreamer bus watch cannot be initialized
    /// or if the monitor fails to start.
    // #[instrument(skip(self, on_event), level = "info")]
    pub fn start<F>(&mut self, on_event: impl Into<Option<F>>) -> Result<(), CamError> 
    where 
        F: Fn(DiscoveryEvent) + Send + Sync + 'static   
    {
        let monitor = gst::DeviceMonitor::new();
        let caps = gst::Caps::builder("video/x-raw").build();
        monitor.add_filter(Some("Video/Source"), Some(&caps));

        let bus = monitor.bus();
        let registry_clone = self.registry.clone();

        let on_event_opt: Option<F> = on_event.into();
        let on_event = Arc::new(on_event_opt);

        let watch = bus.add_watch(move |_, msg| {
            use gst::MessageView;
            let mut event = None;

            match msg.view() {
                MessageView::DeviceAdded(device_msg) => {
                    let mut reg = registry_clone.lock();
                    if let Ok(uid) = reg.handle_device_add(&device_msg.device()) {
                        event = Some(DiscoveryEvent::Added(uid));
                    }
                }
                MessageView::DeviceRemoved(device_msg) => {
                    let mut reg = registry_clone.lock();
                    if let Ok(uid) = reg.handle_device_remove(&device_msg.device()) {
                        event = Some(DiscoveryEvent::Removed(uid));
                    }
                }
                _ => (),
            }

            if let (Some(ev), Some(callback)) = (event, &*on_event) {
                callback(ev);
            }

            glib::ControlFlow::Continue
        }).map_err(|_| CamError::MonitorError("Failed to add bus watch".into()))?;

        monitor.start().map_err(|_| CamError::MonitorError("Failed to start monitor".into()))?;
        self.monitor = Some(monitor);
        self.watch_id = Some(watch);
        Ok(())
    }

    /// Stops the discovery service and clears the GStreamer monitor.
    ///
    /// Dropping the `watch_id` (BusWatchGuard) happens automatically here, 
    /// which safely removes the listener from the GLib MainLoop.
    pub fn stop(&mut self) {
        if let Some(monitor) = self.monitor.take() {
            monitor.stop();
        }
        self.watch_id = None;
    }
}