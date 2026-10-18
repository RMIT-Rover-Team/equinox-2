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
        // With the libcamera plugin installed, the V4L2 provider is hidden
        // and USB cameras are only reported through libcamera, which gives no
        // V4L2 device.path. Show hidden providers so USB cameras come through
        // V4L2; their libcamera duplicates are skipped in `CameraApp`.
        monitor.set_show_all_devices(true);
        // Libcamera advertises CSI cameras as `Source/Video`, while USB V4L2
        // cameras typically use `Video/Source`. Filtering on both keeps audio
        // providers (ALSA, PulseAudio, OSS) from probing and logging errors.
        for classes in ["Video/Source", "Source/Video"] {
            if monitor.add_filter(Some(classes), None).is_none() {
                return Err(CamError::MonitorError(format!(
                    "failed to add device filter {classes}"
                )));
            }
        }

        let watch_guard = monitor
            .bus()
            .add_watch_local(move |_, message| {
                match message.view() {
                    gst::MessageView::DeviceAdded(message) => {
                        if !message.device().device_class().contains("Video") {
                            return glib::ControlFlow::Continue;
                        }
                        if event_tx
                            .send(AppEvent::Discovery(DiscoveryEvent::Added(message.device())))
                            .is_err()
                        {
                            return glib::ControlFlow::Break;
                        }
                    }
                    gst::MessageView::DeviceRemoved(message) => {
                        if !message.device().device_class().contains("Video") {
                            return glib::ControlFlow::Continue;
                        }
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
