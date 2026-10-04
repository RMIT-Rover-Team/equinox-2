use gstreamer as gst;
use gstreamer::prelude::*;
use std::{
    collections::HashMap,
    sync::mpsc::Sender,
    time::{Duration, Instant},
};

use crate::{
    config::LiveKitConfig,
    device::{CameraId, DeviceCatalog},
    error::CamError,
    events::{AppEvent, DiscoveryEvent, MediaEvent},
    media::RoverMediaPipeline,
};

const INITIAL_RECONNECT_DELAY: Duration = Duration::from_secs(1);
const MAX_RECONNECT_DELAY: Duration = Duration::from_secs(30);
const STABLE_CONNECTION_TIME: Duration = Duration::from_secs(60);

#[derive(Debug, Eq, PartialEq)]
pub enum AppState {
    Running,
    Backoff,
    Rebuilding,
    Down,
}

#[derive(Debug, Eq, PartialEq)]
pub enum RecoveryTarget {
    Pipeline,
    Camera(String),
}

struct PendingRecovery {
    target: RecoveryTarget,
    retry_at: Instant,
}

impl PendingRecovery {
    fn is_due(&self, now: Instant) -> bool {
        now >= self.retry_at
    }
}

pub struct CameraApp {
    catalog: DeviceCatalog,
    libcamera_devices: HashMap<gst::Device, CameraId>,
    config: LiveKitConfig,
    event_tx: Sender<AppEvent>,
    media: RoverMediaPipeline,
    state: AppState,
    pending_recovery: Option<PendingRecovery>,
    reconnect_delay: Duration,
    last_media_failure: Option<Instant>,
}

impl CameraApp {
    pub fn start(config: &LiveKitConfig, event_tx: Sender<AppEvent>) -> Result<Self, CamError> {
        let media = RoverMediaPipeline::start(config, event_tx.clone())?;

        Ok(Self {
            catalog: DeviceCatalog::new(),
            libcamera_devices: HashMap::new(),
            config: config.clone(),
            event_tx,
            media,
            state: AppState::Running,
            pending_recovery: None,
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
                if Self::is_imx708(&device) {
                    let id = CameraId::libcamera(device.display_name().as_str());
                    self.media.add_libcamera_camera(id.clone(), &device)?;
                    self.libcamera_devices.insert(device, id.clone());
                    log::info!("Registered IMX708 camera: {}", id.as_str());
                    return Ok(());
                }

                if !Self::supports_mjpeg(&device) {
                    log::debug!(
                        "Skipping non-MJPEG video device: {} ({})",
                        device.display_name(),
                        device.device_class()
                    );
                    return Ok(());
                }

                let id = self.catalog.add(device)?;

                let media_result = self
                    .catalog
                    .get(&id)
                    .ok_or_else(|| CamError::DeviceNotFound(format!("camera {}", id.as_str())))
                    .and_then(|hardware| self.media.add_camera(id.clone(), hardware));

                match media_result {
                    Ok(()) => {
                        log::info!("Registered camera: {}", id.as_str());
                    }
                    // DeviceMonitor also reports the Raspberry Pi's internal
                    // CSI/codec endpoints. They are not MJPEG cameras and are
                    // intentionally unsupported by the current pipeline.
                    Err(CamError::UnsupportedCaps(reason)) => {
                        self.catalog.remove(&id);
                        log::debug!("Skipping unsupported camera {}: {reason}", id.as_str());
                    }
                    Err(error) => {
                        self.catalog.remove(&id);
                        return Err(error);
                    }
                }
            }
            DiscoveryEvent::Removed(device) => {
                if let Some(id) = self.libcamera_devices.remove(&device) {
                    self.media.remove_camera(&id)?;
                    log::info!("Removed IMX708 camera: {}", id.as_str());
                    return Ok(());
                }

                let Ok(id) = self.catalog.id_for_device(&device) else {
                    // This includes devices skipped on addition because their
                    // caps are unsupported by the MJPEG pipeline.
                    return Ok(());
                };
                self.media.remove_camera(&id)?;
                self.catalog
                    .remove(&id)
                    .ok_or_else(|| CamError::DeviceNotFound(format!("camera {}", id.as_str())))?;
                log::info!("Removed camera: {}", id.as_str());
            }
        }

        Ok(())
    }

    fn is_imx708(device: &gst::Device) -> bool {
        let model = device
            .properties()
            .and_then(|properties| properties.get::<String>("Model").ok());

        model
            .unwrap_or_else(|| device.display_name().to_string())
            .to_ascii_lowercase()
            .contains("imx708")
    }

    fn supports_mjpeg(device: &gst::Device) -> bool {
        let mjpeg_caps = gst::Caps::builder("image/jpeg").build();
        device
            .caps()
            .is_some_and(|caps| caps.can_intersect(&mjpeg_caps))
    }

    fn handle_media_event(&mut self, event: MediaEvent) -> Result<(), CamError> {
        match event {
            MediaEvent::PipelineEos => {
                log::info!("PIPELINE EOS");
                self.schedule_recovery(RecoveryTarget::Pipeline, "pipeline reached EOS");
            }

            MediaEvent::PipelineFailed { reason } => {
                log::error!("Media pipeline requires a full rebuild: {reason}");
                self.schedule_recovery(RecoveryTarget::Pipeline, &reason);
            }

            MediaEvent::CameraFailed { id, reason } => {
                log::info!("CAMERA FAIL");
                self.schedule_recovery(RecoveryTarget::Camera(id), &reason);
            }
        }

        Ok(())
    }

    pub fn poll(&mut self) -> Result<(), CamError> {
        let Some(pending) = self.pending_recovery.as_ref() else {
            return Ok(());
        };

        if !pending.is_due(Instant::now()) {
            return Ok(());
        }

        let pending = self
            .pending_recovery
            .take()
            .expect("pending recovery was checked above");
        self.set_state(AppState::Rebuilding);

        let result = match &pending.target {
            RecoveryTarget::Pipeline => self.rebuild_pipeline(),
            RecoveryTarget::Camera(id) => {
                log::warn!(
                    "Targeted recovery for camera {id} is not implemented; rebuilding the pipeline"
                );
                self.rebuild_pipeline()
            }
        };

        match result {
            Ok(()) => {
                self.set_state(AppState::Running);
                Ok(())
            }
            Err(error) => {
                self.set_state(AppState::Down);
                let reason = error.to_string();
                self.schedule_recovery(pending.target, &reason);
                Err(error)
            }
        }
    }

    fn set_state(&mut self, state: AppState) {
        self.state = state;
    }

    fn schedule_recovery(&mut self, target: RecoveryTarget, reason: &str) {
        let now = Instant::now();

        if self
            .last_media_failure
            .is_some_and(|last_failure| now.duration_since(last_failure) >= STABLE_CONNECTION_TIME)
        {
            self.reconnect_delay = INITIAL_RECONNECT_DELAY;
        }
        self.last_media_failure = Some(now);

        if let Some(pending) = self.pending_recovery.as_mut() {
            if matches!(&target, RecoveryTarget::Pipeline) {
                pending.target = RecoveryTarget::Pipeline;
            }

            log::debug!("A media recovery is already scheduled; coalescing: {reason}");
            return;
        }

        let delay = self.reconnect_delay;
        self.pending_recovery = Some(PendingRecovery {
            target,
            retry_at: now + delay,
        });
        self.reconnect_delay = self
            .reconnect_delay
            .saturating_mul(2)
            .min(MAX_RECONNECT_DELAY);
        self.set_state(AppState::Backoff);

        log::warn!(
            "LiveKit media failed ({reason}); attempting recovery in {:.1}s",
            delay.as_secs_f32()
        );
    }

    fn rebuild_pipeline(&mut self) -> Result<(), CamError> {
        self.set_state(AppState::Rebuilding);
        self.media.stop()?;

        let mut replacement = RoverMediaPipeline::start(&self.config, self.event_tx.clone())?;
        for (device, id) in &self.libcamera_devices {
            replacement.add_libcamera_camera(id.clone(), device)?;
        }
        for (id, hardware) in self.catalog.iter() {
            replacement.add_camera(id.clone(), hardware)?;
        }

        self.media = replacement;
        log::info!("Rebuilt the pipeline");

        Ok(())
    }
}
