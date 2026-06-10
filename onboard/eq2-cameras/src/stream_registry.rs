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

        let videoconvert = gst::ElementFactory::make("videoconvert")
            .name(format!("videoconvert_{}", uid))
            .build()
            .map_err(|_| CamError::ElementCreationFailed("videoconvert".into()))?;

        let videoscale = gst::ElementFactory::make("videoscale")
            .name(format!("videoscale_{}", uid))
            .build()
            .map_err(|_| CamError::ElementCreationFailed("videoscale".into()))?;
        
        let autovideosink = gst::ElementFactory::make("autovideosink")
            .name(format!("sink_{}", uid))
            .build()
            .map_err(|_| CamError::ElementCreationFailed("sink".into()))?;
        
        let webrtcbin = gst::ElementFactory::make("webrtcbin")
            .name(format!("webrtc_{}", uid))
            .build()
            .map_err(|_| CamError::ElementCreationFailed("webrtcbin".into()))?;

        pipeline.add_many([source.clone(), videoconvert.clone(), videoscale.clone(), autovideosink.clone()]);

        gst::Element::link_many([source.clone(), videoconvert.clone(), videoscale.clone(), autovideosink.clone()])?;

        pipeline.set_state(gst::State::Playing)
            .map_err(|_| CamError::PipelineError("pipeline".into()))?;

        self.streams.insert(uid.to_string(), StreamInstance {
            pipeline,
            source,
            webrtcbin: gst::ElementFactory::make("webrtcbin").build().unwrap(),
        });

        log::info!("Successfully created stream for UID: {}", uid);
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