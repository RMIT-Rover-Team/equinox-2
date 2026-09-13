use crate::{
    config::LiveKitConfig,
    device::{CameraHardware, CameraId},
    error::CamError,
    events::{AppEvent, MediaEvent},
};
use gstreamer::prelude::*;
use gstreamer::{self as gst, bus::BusWatchGuard};
use std::{collections::HashMap, sync::mpsc::Sender};


struct CameraBranch {
    bin: gst::Bin,
    capsfilter: gst::Element,
    supported_caps: gst::Caps,
    valve: gst::Element,
    livekit_pad: Option<gst::Pad>,
}

pub struct RoverMediaPipeline {
    pipeline: gst::Pipeline,
    branches: HashMap<CameraId, CameraBranch>,
    bus_watch: BusWatchGuard,
    livekit_sink: gst::Element,
}

impl RoverMediaPipeline {
    fn build_source_bin(elements: &[&gst::Element]) -> Result<gst::Bin, CamError> {
        let terminal = elements.last().ok_or_else(|| {
            CamError::PipelineError("camera branch must contain at least one element".into())
        })?;
        let terminal_src = terminal.static_pad("src").ok_or_else(|| {
            CamError::PipelineError(format!("{} has no src pad", terminal.name()))
        })?;

        let bin = gst::Bin::new();
        bin.add_many(elements)?;
        gst::Element::link_many(elements)?;

        let ghost_src = gst::GhostPad::builder_with_target(&terminal_src)?
            .name("src")
            .build();
        bin.add_pad(&ghost_src)?;

        Ok(bin)
    }

    pub fn start(config: &LiveKitConfig, event_tx: Sender<AppEvent>) -> Result<Self, CamError> {
        let pipeline = gst::Pipeline::new();

        let livekit_sink = gst::ElementFactory::make("livekitwebrtcsink")
            .build()
            .map_err(|_| {
                CamError::ElementCreationFailed("failed to create livekitwebrtcsink".into())
            })?;

        let child_proxy = livekit_sink
            .clone()
            .dynamic_cast::<gst::ChildProxy>()
            .map_err(|_| CamError::MonitorError("failed to make child proxy".into()))?;
        child_proxy.set_child_property("signaller::ws-url", config.ws_url.as_str());
        child_proxy.set_child_property("signaller::auth-token", config.auth_token.as_str());

        pipeline
            .add(&livekit_sink)
            .map_err(|_| CamError::PipelineError("failed to add LiveKit sink".into()))?;

        let bus_watch = Self::install_bus_watch(&pipeline, event_tx)?;

        pipeline
            .set_state(gst::State::Ready)
            .map_err(|_| CamError::PipelineError("failed to make media pipeline ready".into()))?;

        Ok(Self {
            pipeline,
            branches: HashMap::new(),
            bus_watch: bus_watch,
            livekit_sink: livekit_sink,
        })
    }

    pub fn add_camera(&mut self, id: CameraId, hardware: &CameraHardware) -> Result<(), CamError> {
        if self.branches.contains_key(&id) {
            return Err(CamError::DuplicateMediaBranch(id.as_str().to_owned()));
        }

        let mut branch = Self::build_webrtc(&id, hardware)?;

        let branch_src = branch
            .bin
            .static_pad("src")
            .ok_or_else(|| CamError::PipelineError("camera branch has no src pad".into()))?;

        self.pipeline
            .add(&branch.bin)
            .map_err(|_| CamError::PipelineError("failed to add camera branch".into()))?;

        let livekit_pad = match self.livekit_sink.request_pad_simple("video_%u") {
            Some(pad) => pad,
            None => {
                let _ = self.pipeline.remove(&branch.bin);
                return Err(CamError::PipelineError(
                    "LiveKit sink refused a video pad".into(),
                ));
            }
        };

        if let Err(error) = branch_src.link(&livekit_pad) {
            self.livekit_sink.release_request_pad(&livekit_pad);
            let _ = self.pipeline.remove(&branch.bin);
            return Err(CamError::PipelineError(format!(
                "failed to link camera to LiveKit: {error}"
            )));
        }

        branch.livekit_pad = Some(livekit_pad);

        if let Err(error) = branch.bin.sync_state_with_parent() {
            let _ = branch.bin.set_state(gst::State::Null);

            if let Some(livekit_pad) = branch.livekit_pad.take() {
                if let Err(unlink_error) = branch_src.unlink(&livekit_pad) {
                    log::warn!("Failed to unlink camera during rollback: {unlink_error}");
                }
                self.livekit_sink.release_request_pad(&livekit_pad);
            }

            if let Err(rollback_error) = self.pipeline.remove(&branch.bin) {
                log::error!("Failed to roll back camera branch: {rollback_error}");
            }

            return Err(CamError::PipelineError(format!(
                "failed to synchronize camera branch state: {error}"
            )));
        }

        self.branches.insert(id, branch);
        self.pipeline
            .set_state(gst::State::Playing)
            .map_err(|_| CamError::PipelineError("failed to start media pipeline".into()))?;
        Ok(())
    }

    pub fn remove_camera(&mut self, id: &CameraId) -> Result<(), CamError> {
        let mut branch = self
            .branches
            .remove(id)
            .ok_or_else(|| CamError::DeviceNotFound(format!("camera branch {}", id.as_str())))?;

        let branch_src = match branch.bin.static_pad("src") {
            Some(pad) => pad,
            None => {
                self.branches.insert(id.clone(), branch);
                return Err(CamError::PipelineError(
                    "camera branch has no src pad".into(),
                ));
            }
        };

        if let Err(error) = branch.bin.set_state(gst::State::Null) {
            self.branches.insert(id.clone(), branch);
            return Err(CamError::RemoveDeviceFailed(format!(
                "failed to stop camera branch {}: {error}",
                id.as_str()
            )));
        }

        if let Some(livekit_pad) = branch.livekit_pad.as_ref() {
            if let Err(error) = branch_src.unlink(livekit_pad) {
                if let Err(recovery_error) = branch.bin.sync_state_with_parent() {
                    log::error!(
                        "Failed to restore camera branch {} after unlink failed: {recovery_error}",
                        id.as_str()
                    );
                }

                self.branches.insert(id.clone(), branch);
                return Err(CamError::RemoveDeviceFailed(format!(
                    "failed to unlink camera branch {} from LiveKit: {error}",
                    id.as_str()
                )));
            }
        }

        if let Err(error) = self.pipeline.remove(&branch.bin) {
            if let Some(livekit_pad) = branch.livekit_pad.as_ref() {
                if let Err(recovery_error) = branch_src.link(livekit_pad) {
                    log::error!(
                        "Failed to relink camera {} after removal failed: {recovery_error}",
                        id.as_str()
                    );
                }
            }

            if let Err(recovery_error) = branch.bin.sync_state_with_parent() {
                log::error!(
                    "Failed to restore camera branch {} after removal failed: {recovery_error}",
                    id.as_str()
                );
            }

            self.branches.insert(id.clone(), branch);
            return Err(CamError::RemoveDeviceFailed(format!(
                "failed to detach camera branch {}: {error}",
                id.as_str()
            )));
        }

        if let Some(livekit_pad) = branch.livekit_pad.take() {
            self.livekit_sink.release_request_pad(&livekit_pad);
        }

        log::info!("Removed media branch for camera {}", id.as_str());
        Ok(())
    }

    #[allow(dead_code)]
    pub fn play(&self) -> Result<(), CamError> {
        self.pipeline
            .set_state(gst::State::Playing)
            .map_err(|_| CamError::PipelineError("failed to pause media pipeline".into()))?;
        Ok(())
    }

    #[allow(dead_code)]
    pub fn ready(&self) -> Result<(), CamError> {
        self.pipeline
            .set_state(gst::State::Ready)
            .map_err(|_| CamError::PipelineError("failed to pause media pipeline".into()))?;
        Ok(())
    }

    #[allow(dead_code)]
    pub fn pause(&self) -> Result<(), CamError> {
        self.pipeline
            .set_state(gst::State::Paused)
            .map_err(|_| CamError::PipelineError("failed to pause media pipeline".into()))?;
        Ok(())
    }

    pub fn stop(&self) -> Result<(), CamError> {
        self.pipeline
            .set_state(gst::State::Null)
            .map_err(|_| CamError::PipelineError("failed to stop media pipeline".into()))?;
        Ok(())
    }

    /// Applies fixed capture caps to an existing branch.
    ///
    /// A future frontend API should construct these caps from validated,
    /// structured fields instead of accepting an arbitrary caps string.
    #[allow(dead_code)]
    pub fn set_camera_caps(&mut self, id: &CameraId, caps: &gst::Caps) -> Result<(), CamError> {
        if !caps.is_fixed() {
            return Err(CamError::UnsupportedCaps(
                "requested caps must describe one fixed mode".into(),
            ));
        }

        let branch = self
            .branches
            .get_mut(id)
            .ok_or_else(|| CamError::DeviceNotFound(format!("camera branch {}", id.as_str())))?;

        if !branch.supported_caps.can_intersect(caps) {
            return Err(CamError::UnsupportedCaps(format!(
                "camera {} does not advertise {caps}",
                id.as_str()
            )));
        }

        branch.capsfilter.set_property("caps", caps);
        Ok(())
    }

    #[allow(dead_code)]
    fn build_test_branch(
        id: &CameraId,
        hardware: &CameraHardware,
    ) -> Result<CameraBranch, CamError> {
        let default_caps = gst::Caps::builder("image/jpeg")
            .field("width", 640i32)
            .field("height", 480i32)
            .field("framerate", gst::Fraction::new(30, 1))
            .build();
        if !hardware.caps.can_intersect(&default_caps) {
            return Err(CamError::UnsupportedCaps(format!(
                "camera {} does not support the default caps {default_caps}",
                id.as_str()
            )));
        }
        let source = hardware
            .device
            .create_element(Some("source"))
            .map_err(|_| {
                CamError::ElementCreationFailed("failed to create camera source".into())
            })?;
        let capsfilter = gst::ElementFactory::make("capsfilter")
            .property("caps", &default_caps)
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create capsfilter".into()))?;
        let queue = gst::ElementFactory::make("queue")
            .property("max-size-buffers", 2u32)
            .property("max-size-bytes", 0u32)
            .property("max-size-time", 0u64)
            .property_from_str("leaky", "downstream")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create queue".into()))?;
        let decoder = gst::ElementFactory::make("jpegdec")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create jpegdec".into()))?;
        let videoconvert = gst::ElementFactory::make("videoconvert")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create videoconvert".into()))?;
        let encoder_factory = gst::ElementFactory::find("v4l2h264enc")
            .or_else(|| {
                log::warn!("can't find v4l2h264enc, falling back to x264enc");
                gst::ElementFactory::find("x264enc")
            })
            .ok_or_else(|| {
                CamError::ElementCreationFailed(
                    "v4l2h264enc or x264enc could not be found (hint: install v4l2".into(),
                )
            })?;

        let encoder: gst::Element = match encoder_factory.name().as_str() {
            "v4l2h264enc" => encoder_factory
                .create()
                .build()
                .map_err(|_| CamError::ElementCreationFailed("failed to create encoder".into()))?,
            "x264enc" => {
                let bitrate_kbps: u32 = 6_000;
                encoder_factory
                    .create()
                    .property_from_str("tune", "zerolatency") // these properties are more cpu intensive, discard if required
                    .property("bitrate", bitrate_kbps) // kbit/sec, tune later
                    // .property_from_str("speed-preset", "ultrafast") // test speed-presets later
                    .build()
                    .map_err(|_| {
                        CamError::ElementCreationFailed("failed to create encoder".into())
                    })?
            }
            name => {
                log::warn!(
                    "Could not find {} encoder, falling back to default encoder",
                    name
                );
                encoder_factory.create().build().map_err(|_| {
                    CamError::ElementCreationFailed("failed to create encoder".into())
                })?
            }
        };

        let h264parse = gst::ElementFactory::make("h264parse")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to make h264 parse".into()))?;

        let valve = gst::ElementFactory::make("valve")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to make valve".into()))?;

        let fakesink = gst::ElementFactory::make("fakesink")
            .property("sync", false)
            .property("async", false)
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create fakesink".into()))?;

        let elements = [
            &source,
            &capsfilter,
            &queue,
            &decoder,
            &videoconvert,
            &encoder,
            &h264parse,
            &valve,
            &fakesink,
        ];

        let bin = gst::Bin::new();
        bin.add_many(elements)?;
        gst::Element::link_many(elements)?;
        log::info!("Successfully made camera branch for {}", hardware.name);
        Ok(CameraBranch {
            bin: bin,
            capsfilter: capsfilter,
            supported_caps: hardware.caps.clone(),
            valve: valve,
            livekit_pad: None,
        })
    }

    fn build_webrtc(id: &CameraId, hardware: &CameraHardware) -> Result<CameraBranch, CamError> {
        let default_caps = gst::Caps::builder("image/jpeg")
            .field("width", 640i32)
            .field("height", 480i32)
            .field("framerate", gst::Fraction::new(30, 1))
            .build();

        if !hardware.caps.can_intersect(&default_caps) {
            return Err(CamError::UnsupportedCaps(format!(
                "camera {} does not support the default caps {default_caps}",
                id.as_str()
            )));
        }

        let source = hardware
            .device
            .create_element(Some("source"))
            .map_err(|_| {
                CamError::ElementCreationFailed("failed to create camera source".into())
            })?;

        if source.find_property("extra-controls").is_some() {
            let controls = gst::Structure::builder("c")
                .field("focus_auto", true)
                .build();

            source.set_property("extra-controls", controls);
        } else {
            log::warn!(
                "Camera {} does not expose GStreamer's V4L2 extra-controls property",
                hardware.name
            );
        }

        let capsfilter = gst::ElementFactory::make("capsfilter")
            .property("caps", &default_caps)
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create capsfilter".into()))?;

        let queue = gst::ElementFactory::make("queue")
            .property("max-size-buffers", 2u32)
            .property("max-size-bytes", 0u32)
            .property("max-size-time", 0u64)
            .property_from_str("leaky", "downstream")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create queue".into()))?;
        let decoder = gst::ElementFactory::make("jpegdec")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create jpegdec".into()))?;
        let videoconvert = gst::ElementFactory::make("videoconvert")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to create videoconvert".into()))?;
        let encoder_factory = gst::ElementFactory::find("v4l2h264enc")
            .or_else(|| {
                log::warn!("can't find v4l2h264enc, falling back to x264enc");
                gst::ElementFactory::find("x264enc")
            })
            .ok_or_else(|| {
                CamError::ElementCreationFailed(
                    "v4l2h264enc or x264enc could not be found (hint: install v4l2".into(),
                )
            })?;

        let encoder: gst::Element = match encoder_factory.name().as_str() {
            "v4l2h264enc" => encoder_factory
                .create()
                .build()
                .map_err(|_| CamError::ElementCreationFailed("failed to create encoder".into()))?,
            "x264enc" => {
                let bitrate_kbps: u32 = 6_000;
                encoder_factory
                    .create()
                    .property_from_str("tune", "zerolatency") // these properties are more cpu intensive, discard if required
                    .property("bitrate", bitrate_kbps) // kbit/sec, tune later
                    // .property_from_str("speed-preset", "ultrafast") // test speed-presets later
                    .build()
                    .map_err(|_| {
                        CamError::ElementCreationFailed("failed to create encoder".into())
                    })?
            }
            name => {
                log::warn!(
                    "Could not find {} encoder, falling back to default encoder",
                    name
                );
                encoder_factory.create().build().map_err(|_| {
                    CamError::ElementCreationFailed("failed to create encoder".into())
                })?
            }
        };
        let h264parse = gst::ElementFactory::make("h264parse")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to make h264 parse".into()))?;

        let valve = gst::ElementFactory::make("valve")
            .build()
            .map_err(|_| CamError::ElementCreationFailed("failed to make valve".into()))?;

        let elements = [
            &source,
            &capsfilter,
            &queue,
            &decoder,
            &videoconvert,
            &encoder,
            &h264parse,
            &valve,
        ];

        let bin = Self::build_source_bin(&elements)?;
        log::info!("Successfully made camera branch for {}", hardware.name);
        Ok(CameraBranch {
            bin: bin,
            capsfilter: capsfilter,
            supported_caps: hardware.caps.clone(),
            valve: valve,
            livekit_pad: None,
        })
    }

    fn install_bus_watch(
        pipeline: &gst::Pipeline,
        event_tx: Sender<AppEvent>,
    ) -> Result<gst::bus::BusWatchGuard, CamError> {
        let bus = pipeline
            .bus()
            .ok_or_else(|| CamError::PipelineError("Failed to make bus".into()))?;

        let bus_watch_guard = bus.add_watch_local(move |_, message| {
            match message.view() {
                gst::MessageView::Error(error) => {
                    let source = error
                        .src()
                        .map(|source| source.path_string().to_string())
                        .unwrap_or_else(|| "unknown source".into());
                    let reason = error.error().to_string();

                    log::error!(
                        "GStreamer error from {source}: {reason} (debug: {:?})",
                        error.debug(),
                    );

                    let is_livekit_error = error
                        .src()
                        .and_then(|source| source.downcast_ref::<gst::Element>())
                        .and_then(|element| element.factory())
                        .is_some_and(|factory| factory.name() == "livekitwebrtcsink");

                    if is_livekit_error {
                        if event_tx
                            .send(AppEvent::Media(MediaEvent::LiveKitDisconnected { reason }))
                            .is_err()
                        {
                            return glib::ControlFlow::Break;
                        }
                    }
                }

                gst::MessageView::Eos(_) => {
                    log::warn!("Media pipeline reached EOS");
                    if event_tx
                        .send(AppEvent::Media(MediaEvent::EndOfStream))
                        .is_err()
                    {
                        return glib::ControlFlow::Break;
                    }
                }

                gst::MessageView::Warning(warning) => {
                    let source = warning
                        .src()
                        .map(|source| source.path_string().to_string())
                        .unwrap_or_else(|| "unknown source".into());

                    log::warn!(
                        "GStreamer warning from {source}: {} (debug: {:?})",
                        warning.error(),
                        warning.debug(),
                    );
                }

                _ => {}
            }

            glib::ControlFlow::Continue
        })?;
        Ok(bus_watch_guard)
    }
}

impl Drop for RoverMediaPipeline {
    fn drop(&mut self) {
        let _ = self.pipeline.set_state(gst::State::Null);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn source_bin_exposes_the_terminal_elements_unlinked_src_pad() {
        gst::init().expect("GStreamer should initialize");

        let parser = gst::ElementFactory::make("identity")
            .name("parser")
            .build()
            .expect("identity element should be available");
        let valve = gst::ElementFactory::make("valve")
            .build()
            .expect("valve element should be available");
        let elements = [&parser, &valve];

        let bin = RoverMediaPipeline::build_source_bin(&elements)
            .expect("the terminal valve src pad should be a valid ghost-pad target");

        let ghost_src = bin
            .static_pad("src")
            .expect("source bin should expose a src pad")
            .downcast::<gst::GhostPad>()
            .expect("source bin src pad should be a ghost pad");
        let target = ghost_src
            .target()
            .expect("source bin ghost pad should have a target");

        assert_eq!(
            target,
            valve
                .static_pad("src")
                .expect("valve should have a src pad")
        );
        assert!(parser.static_pad("src").unwrap().is_linked());
    }

    #[test]
    fn source_bin_streams_through_its_ghost_pad() {
        gst::init().expect("GStreamer should initialize");

        let source = gst::ElementFactory::make("videotestsrc")
            .property("num-buffers", 1i32)
            .build()
            .expect("videotestsrc should be available");
        let valve = gst::ElementFactory::make("valve")
            .build()
            .expect("valve element should be available");
        let branch_elements = [&source, &valve];
        let branch = RoverMediaPipeline::build_source_bin(&branch_elements)
            .expect("source bin should expose its terminal output");
        let sink = gst::ElementFactory::make("fakesink")
            .property("sync", false)
            .build()
            .expect("fakesink should be available");
        let pipeline = gst::Pipeline::new();

        pipeline
            .add_many([branch.upcast_ref(), &sink])
            .expect("branch and sink should attach to the pipeline");
        branch
            .link(&sink)
            .expect("the branch ghost pad should link downstream");
        pipeline
            .set_state(gst::State::Playing)
            .expect("pipeline should start");

        let message = pipeline
            .bus()
            .expect("pipeline should have a bus")
            .timed_pop_filtered(
                gst::ClockTime::from_seconds(2),
                &[gst::MessageType::Eos, gst::MessageType::Error],
            )
            .expect("pipeline should reach EOS within two seconds");

        pipeline
            .set_state(gst::State::Null)
            .expect("pipeline should stop");

        match message.view() {
            gst::MessageView::Eos(_) => {}
            gst::MessageView::Error(error) => panic!(
                "pipeline failed before EOS: {} (debug: {:?})",
                error.error(),
                error.debug()
            ),
            _ => unreachable!("message filter returned an unexpected message"),
        }
    }
}
