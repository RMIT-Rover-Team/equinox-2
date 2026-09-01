use crate::{
    config::LiveKitConfig,
    device::{CameraHardware, CameraId},
    error::CamError,
};
use gstreamer::prelude::*;
use gstreamer::{self as gst, bus::BusWatchGuard};
use std::collections::HashMap;

struct CameraBranch {
    bin: gst::Bin,
    capsfilter: gst::Element,
    supported_caps: gst::Caps,
    livekit_pad: Option<gst::Pad>,
}

pub struct RoverMediaPipeline {
    pipeline: gst::Pipeline,
    branches: HashMap<CameraId, CameraBranch>,
    _bus_watch: BusWatchGuard,
    livekit_sink: gst::Element,
}

impl RoverMediaPipeline {
    pub fn start(config: &LiveKitConfig) -> Result<Self, CamError> {
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

        let bus_watch = Self::install_bus_watch(&pipeline)?;

        pipeline
            .set_state(gst::State::Ready)
            .map_err(|_| CamError::PipelineError("failed to make media pipeline ready".into()))?;

        Ok(Self {
            pipeline,
            branches: HashMap::new(),
            _bus_watch: bus_watch,
            livekit_sink,
        })
    }

    pub fn add_camera(&mut self, id: CameraId, hardware: &CameraHardware) -> Result<(), CamError> {
        self.pipeline
            .set_state(gst::State::Ready)
            .map_err(|_| CamError::PipelineError("failed to pause media pipeline".into()))?;

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
        self.pipeline
            .set_state(gst::State::Ready)
            .map_err(|_| CamError::PipelineError("failed to pause media pipeline".into()))?;

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

        self.pipeline
            .set_state(gst::State::Playing)
            .map_err(|_| CamError::PipelineError("failed to start media pipeline".into()))?;

        log::info!("Removed media branch for camera {}", id.as_str());
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
            &fakesink,
        ];

        let bin = gst::Bin::new();
        bin.add_many(elements)?;
        gst::Element::link_many(elements)?;
        log::info!("Successfully made camera branch for {}", hardware.name);
        Ok(CameraBranch {
            bin,
            capsfilter,
            supported_caps: hardware.caps.clone(),
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

        let elements = [
            &source,
            &capsfilter,
            &queue,
            &decoder,
            &videoconvert,
            &encoder,
            &h264parse,
        ];

        let bin = gst::Bin::new();
        bin.add_many(elements)?;
        gst::Element::link_many(elements)?;

        let parser_src = h264parse
            .static_pad("src")
            .ok_or_else(|| CamError::PipelineError("h264parse has no src pad".into()))?;

        let ghost_src = gst::GhostPad::builder_with_target(&parser_src)?
            .name("src")
            .build();

        bin.add_pad(&ghost_src)?;
        log::info!("Successfully made camera branch for {}", hardware.name);
        Ok(CameraBranch {
            bin,
            capsfilter,
            supported_caps: hardware.caps.clone(),
            livekit_pad: None,
        })
    }

    fn install_bus_watch(pipeline: &gst::Pipeline) -> Result<gst::bus::BusWatchGuard, CamError> {
        let bus = pipeline
            .bus()
            .ok_or_else(|| CamError::PipelineError("Failed to make bus".into()))?;

        let bus_watch_guard = bus.add_watch_local(|_, message| {
            match message.view() {
                gst::MessageView::Error(error) => {
                    let source = error
                        .src()
                        .map(|source| source.path_string().to_string())
                        .unwrap_or_else(|| "unknown source".into());

                    log::error!(
                        "GStreamer error from {source}: {} (debug: {:?})",
                        error.error(),
                        error.debug(),
                    );
                }

                gst::MessageView::Eos(_) => {
                    log::warn!("Media pipeline reached EOS");
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
