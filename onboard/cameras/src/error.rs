use glib::BoolError;
use thiserror::Error;

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
    #[error("Camera ID is already registered: {0}")]
    DuplicateDevice(String),
    #[error("Camera already has an active media branch: {0}")]
    DuplicateMediaBranch(String),
    #[error("Failed to create element: {0}")]
    ElementCreationFailed(String),
    #[error("Unsupported camera caps: {0}")]
    UnsupportedCaps(String),
    #[error("Monitor failure: {0}")]
    MonitorError(String),
    #[error("Failed to remove device: {0}")]
    RemoveDeviceFailed(String),
    #[error("Pipeline failure: {0}")]
    PipelineError(String),
}
