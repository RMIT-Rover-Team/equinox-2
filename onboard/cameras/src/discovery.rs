//! Camera hotplug discovery using GStreamer's [`gst::DeviceMonitor`].

use crate::error::CamError;
use gstreamer as gst;
use gstreamer::prelude::*;

/// A raw device change reported by GStreamer.
pub enum DiscoveryEvent {
    Added(gst::Device),
    Removed(gst::Device),
}

/// An active camera discovery service.
///
/// A successfully constructed value always has a running monitor and an
/// installed bus watch. Dropping it stops monitoring and removes the watch.
pub struct DeviceDiscovery {
    _watch_guard: gst::bus::BusWatchGuard,
    monitor: gst::DeviceMonitor,
}

impl DeviceDiscovery {
    /// Starts monitoring video source devices.
    pub fn start<F>(mut on_event: F) -> Result<Self, CamError>
    where
        F: FnMut(DiscoveryEvent) + 'static,
    {
        let monitor = gst::DeviceMonitor::new();
        monitor.add_filter(Some("Video/Source"), None);

        let watch_guard = monitor
            .bus()
            .add_watch_local(move |_, message| {
                match message.view() {
                    gst::MessageView::DeviceAdded(message) => {
                        on_event(DiscoveryEvent::Added(message.device()));
                    }
                    gst::MessageView::DeviceRemoved(message) => {
                        on_event(DiscoveryEvent::Removed(message.device()));
                    }
                    _ => {}
                }

                glib::ControlFlow::Continue
            })
            .map_err(|error| {
                CamError::MonitorError(format!("failed to install bus watch: {error}"))
            })?;

        monitor
            .start()
            .map_err(|error| CamError::MonitorError(format!("failed to start monitor: {error}")))?;

        Ok(Self {
            _watch_guard: watch_guard,
            monitor,
        })
    }
}

impl Drop for DeviceDiscovery {
    fn drop(&mut self) {
        self.monitor.stop();
    }
}
