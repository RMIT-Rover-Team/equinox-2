//! Camera hotplug discovery using GStreamer's [`gst::DeviceMonitor`].

use crate::{
    error::CamError,
    events::{AppEvent, DiscoveryEvent},
};
use gstreamer as gst;
use gstreamer::prelude::*;
use std::sync::mpsc::Sender;

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
    pub fn start(event_tx: Sender<AppEvent>) -> Result<Self, CamError> {
        let monitor = gst::DeviceMonitor::new();
        monitor.add_filter(Some("Video/Source"), None);

        let watch_guard = monitor
            .bus()
            .add_watch_local(move |_, message| {
                match message.view() {
                    gst::MessageView::DeviceAdded(message) => {
                        if event_tx
                            .send(AppEvent::Discovery(DiscoveryEvent::Added(message.device())))
                            .is_err()
                        {
                            return glib::ControlFlow::Break;
                        }
                    }
                    gst::MessageView::DeviceRemoved(message) => {
                        if event_tx
                            .send(AppEvent::Discovery(DiscoveryEvent::Removed(
                                message.device(),
                            )))
                            .is_err()
                        {
                            return glib::ControlFlow::Break;
                        }
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
