use std::{
    sync::mpsc::Sender,
    time::{Duration, Instant},
};

use crate::{
    config::LiveKitConfig,
    device::DeviceCatalog,
    error::CamError,
    events::{AppEvent, DiscoveryEvent, MediaEvent},
    media::RoverMediaPipeline,
};

const INITIAL_RECONNECT_DELAY: Duration = Duration::from_secs(1);
const MAX_RECONNECT_DELAY: Duration = Duration::from_secs(30);
const STABLE_CONNECTION_TIME: Duration = Duration::from_secs(60);

pub enum AppState {
    Running,
    Rebuilding,
    Down,
}

pub struct CameraApp {
    catalog: DeviceCatalog,
    config: LiveKitConfig,
    event_tx: Sender<AppEvent>,
    media: RoverMediaPipeline,
    state: AppState,
    reconnect_at: Option<Instant>,
    reconnect_delay: Duration,
    last_media_failure: Option<Instant>,
}

impl CameraApp {
    pub fn start(config: &LiveKitConfig, event_tx: Sender<AppEvent>) -> Result<Self, CamError> {
        let media = RoverMediaPipeline::start(config, event_tx.clone())?;

        Ok(Self {
            catalog: DeviceCatalog::new(),
            config: config.clone(),
            event_tx: event_tx,
            media: media,
            state: AppState::Running,
            reconnect_at: None,
            reconnect_delay: INITIAL_RECONNECT_DELAY,
            last_media_failure: None,
        })
    }

    pub fn handle_event(&mut self, event: AppEvent) -> Result<(), CamError> {
        match event {
            AppEvent::Discovery(event) => self.handle_discovery(event),
            AppEvent::Media(event) => self.handle_media_event(event),
        }
    }

    fn handle_discovery(&mut self, event: DiscoveryEvent) -> Result<(), CamError> {
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

    fn handle_media_event(&mut self, event: MediaEvent) -> Result<(), CamError> {
        match event {
            MediaEvent::LiveKitDisconnected { reason } => self.schedule_rebuild_media(&reason),
            MediaEvent::EndOfStream => {
                self.schedule_rebuild_media("the media pipeline reached end-of-stream")
            }
        }
    }

    pub fn poll(&mut self) -> Result<(), CamError>{
        let Some(reconnect_at) = self.reconnect_at else {
            return Ok(())
        };

        if Instant::now() < reconnect_at {
            return Ok(());
        }

        let _ = self.rebuild_media()
            .map_err(|_| CamError::PipelineRebuildError("Failed to rebuild media pipeline".into()));
        self.reconnect_at = None;
        Ok(())
    }

    fn set_state(&mut self, state: AppState) -> Result<(), CamError> {
        self.state = state;
        Ok(())
    }

    fn schedule_rebuild_media(&mut self, reason: &str) -> Result<(), CamError> {
        self.set_state(AppState::Rebuilding).map_err(|_| {
            CamError::PipelineRebuildError("Failed to set app state to rebuilding".into())
        })?;

        let now = Instant::now();

        if self
            .last_media_failure
            .is_some_and(|last_failure| now.duration_since(last_failure) >= STABLE_CONNECTION_TIME)
        {
            self.reconnect_delay = INITIAL_RECONNECT_DELAY;
        }
        self.last_media_failure = Some(now);

        if self.reconnect_at.is_some() {
            log::debug!("A media rebuild is already scheduled; ignoring: {reason}");
            return Ok(());
        }

        let delay = self.reconnect_delay;
        self.reconnect_at = Some(now + delay);
        self.reconnect_delay = self
            .reconnect_delay
            .saturating_mul(2)
            .min(MAX_RECONNECT_DELAY);

        log::warn!(
            "LiveKit media failed ({reason}); rebuilding in {:.1}s",
            delay.as_secs_f32()
        );

        Ok(())
    }

    fn rebuild_media(&mut self) -> Result<(), CamError> {
        self.media.stop()?;

        let mut replacement = RoverMediaPipeline::start(&self.config, self.event_tx.clone())?;
        for (id, hardware) in self.catalog.iter() {
            replacement.add_camera(id.clone(), hardware)?;
        }

        self.media = replacement;
        log::info!("Rebuilt the LiveKit media pipeline");
        Ok(())
    }
}
