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
    // #[error("Failed to create element: {0}")]
    // ElementCreationFailed(String),
    #[error("Monitor failure: {0}")]
    MonitorError(String),
    // #[error("Pipeline failure: {0}")]
    // PipelineError(String),
}
