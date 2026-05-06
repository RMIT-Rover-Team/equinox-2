use gstreamer as gst;
use thiserror::Error;
use glib::BoolError;
#[derive(Debug, Error)]
pub enum CamError {
    #[error("GStreamer error: {0}")]
    Gst(#[from] glib::Error),
    #[error("GStreamer boolean error: {0}")]
    Bool(#[from] BoolError),
    #[error("Property not found: {0}")]
    PropertyNotFound(String),
    #[error("Device not found: {0}")]
    DeviceNotFound(String),
    #[error("Failed to create element: {0}")]
    ElementCreationFailed(String),
    #[error("Monitor failure: {0}")]
    MonitorError(String),
}

pub struct CameraHardware {
    pub uid: String,
    pub name: String,
    pub path: String,
    pub device: gst::Device,
    pub caps: gst::Caps,
}

pub struct StreamInstance {
    pub pipeline: gst::Pipeline,
    pub source: gst::Element,
}