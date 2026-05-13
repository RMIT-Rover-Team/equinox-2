use gstreamer as gst;
use gstreamer::prelude::*;

use std::collections::HashMap;
use crate::camera_types::{CamError, StreamInstance};

pub struct StreamRegistry {
    streams: HashMap<String, StreamInstance>,
}

// paused is equivilant to play for streams. either be null, ready or playing.
impl StreamRegistry {
    pub fn new() -> Self {
        Self { streams: HashMap::new() }
    }

    pub fn create_stream(&mut self, uid: &str, device: &gst::Device) -> Result<(), CamError> {
        let pipeline = gst::Pipeline::with_name(uid);
        
        let source = device.create_element(Some("source"))
            .map_err(|_| CamError::ElementCreationFailed("source".into()))?;

        pipeline.add(&source)?;

        self.streams.insert(uid.to_string(), StreamInstance {
            pipeline,
            source,
        });

        Ok(())
    }

    pub fn remove_stream(&mut self, uid: &str) {
        if let Some(instance) = self.streams.remove(uid) {
            let _ = instance.pipeline.set_state(gst::State::Null);
            log::info!("Stream {} stopped and removed", uid);
        }
    }

    pub fn setup_stream(&mut self, uid: &str) -> Result<(), CamError> {
        Ok(())
    }

    /// Starts stream based off uid
    pub fn start_stream(&mut self, uid: &str) -> Result<(), CamError> {
        let instance = self.streams.get(uid)
            .ok_or_else(|| CamError::DeviceNotFound(format!("Stream not found for UID: {}", uid)))?; 
            
        instance.pipeline.set_state(gst::State::Playing);
        Ok(())
    }
}