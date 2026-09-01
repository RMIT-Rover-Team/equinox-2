use crate::{
    config::LiveKitConfig, device::DeviceCatalog, discovery::DiscoveryEvent, error::CamError,
    media::RoverMediaPipeline,
};

pub struct CameraApp {
    catalog: DeviceCatalog,
    media: RoverMediaPipeline,
}

impl CameraApp {
    pub fn start(config: &LiveKitConfig) -> Result<Self, CamError> {
        Ok(Self {
            catalog: DeviceCatalog::new(),
            media: RoverMediaPipeline::start(config)?,
        })
    }

    pub fn handle_discovery(&mut self, event: DiscoveryEvent) -> Result<(), CamError> {
        match event {
            DiscoveryEvent::Added(device) => {
                let id = self.catalog.add(device)?;

                let media_result = self
                    .catalog
                    .get(&id)
                    .ok_or_else(|| CamError::DeviceNotFound(format!("camera {}", id.as_str())))
                    .and_then(|hardware| self.media.add_camera(id.clone(), hardware));

                if let Err(error) = media_result {
                    self.catalog.remove(&id);
                    return Err(error);
                }

                log::info!("Registered camera: {}", id.as_str());
            }
            DiscoveryEvent::Removed(device) => {
                let id = self.catalog.id_for_device(&device)?;
                self.media.remove_camera(&id)?;
                self.catalog
                    .remove(&id)
                    .ok_or_else(|| CamError::DeviceNotFound(format!("camera {}", id.as_str())))?;
                log::info!("Removed camera: {}", id.as_str());
            }
        }

        Ok(())
    }
}
