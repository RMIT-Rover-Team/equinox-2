use crate::{
    config::LiveKitConfig,
    device::{CameraHardware, CameraId},
    error::CamError,
    events::{AppEvent, MediaEvent},
};
use gstreamer::prelude::*;
use gstreamer::{self as gst, bus::BusWatchGuard};
use std::{
    cell::{Cell, RefCell},
    collections::HashMap,
    rc::Rc,
    sync::mpsc::Sender,
    time::{Duration, Instant},
};

const INITIAL_PUBLISHER_REPLACEMENT_DELAY: Duration = Duration::from_secs(1);
const MAX_PUBLISHER_REPLACEMENT_DELAY: Duration = Duration::from_secs(30);
const STABLE_PUBLISHER_TIME: Duration = Duration::from_secs(60);
const PUBLISHER_STATE_CHANGE_TIMEOUT: gst::ClockTime = gst::ClockTime::from_seconds(5);

enum MediaBusAction {
    ReplacePublisher,
    Notify(MediaEvent),
    Ignore,
}

struct PublisherRecoveryState {
    pending: Cell<bool>,
    retry_delay: Cell<Duration>,
    stable_since: Cell<Option<Instant>>,
}

impl PublisherRecoveryState {
    fn new() -> Self {
        Self {
            pending: Cell::new(false),
            retry_delay: Cell::new(INITIAL_PUBLISHER_REPLACEMENT_DELAY),
            stable_since: Cell::new(None),
        }
    }

    fn schedule(&self, now: Instant) -> Option<Duration> {
        if self.pending.replace(true) {
            return None;
        }

        if self
            .stable_since
            .get()
            .is_some_and(|stable_since| now.duration_since(stable_since) >= STABLE_PUBLISHER_TIME)
        {
            self.retry_delay.set(INITIAL_PUBLISHER_REPLACEMENT_DELAY);
        }
        self.stable_since.set(None);

        let delay = self.retry_delay.get();
        self.retry_delay
            .set(delay.saturating_mul(2).min(MAX_PUBLISHER_REPLACEMENT_DELAY));
        Some(delay)
    }

    fn finish(&self) {
        self.pending.set(false);
    }

    fn recovered(&self, now: Instant) {
        self.pending.set(false);
        self.stable_since.set(Some(now));
    }

    fn escalated(&self) {
        self.pending.set(true);
        self.stable_since.set(None);
    }
}

struct CameraBranch {
    bin: gst::Bin,
    capsfilter: gst::Element,
    supported_caps: gst::Caps,
    valve: gst::Element,
}

#[derive(Clone)]
struct PublisherLink {
    branch_src: gst::Pad,
    sink_pad: gst::Pad,
    valve: glib::WeakRef<gst::Element>,
}

struct Publisher {
    sink: gst::Element,
    links: HashMap<String, PublisherLink>,
}

pub struct RoverMediaPipeline {
    pipeline: gst::Pipeline,
    branches: HashMap<CameraId, CameraBranch>,
    bus_watch: BusWatchGuard,
    publisher: Rc<RefCell<Publisher>>,
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

        let livekit_sink = Self::build_livekit_sink(config)?;
        pipeline
            .add(&livekit_sink)
            .map_err(|_| CamError::PipelineError("failed to add LiveKit sink".into()))?;

        let publisher = Rc::new(RefCell::new(Publisher {
            sink: livekit_sink,
            links: HashMap::new(),
        }));
        let bus_watch = Self::install_bus_watch(&pipeline, &publisher, config.clone(), event_tx)?;

        pipeline
            .set_state(gst::State::Ready)
            .map_err(|_| CamError::PipelineError("failed to make media pipeline ready".into()))?;

        Ok(Self {
            pipeline,
            branches: HashMap::new(),
            bus_watch: bus_watch,
            publisher,
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

        let livekit_sink = self.publisher.borrow().sink.clone();
        let livekit_pad = match livekit_sink.request_pad_simple("video_%u") {
            Some(pad) => pad,
            None => {
                let _ = self.pipeline.remove(&branch.bin);
                return Err(CamError::PipelineError(
                    "LiveKit sink refused a video pad".into(),
                ));
            }
        };

        if let Err(error) = branch_src.link(&livekit_pad) {
            livekit_sink.release_request_pad(&livekit_pad);
            let _ = self.pipeline.remove(&branch.bin);
            return Err(CamError::PipelineError(format!(
                "failed to link camera to LiveKit: {error}"
            )));
        }

        branch.bin.sync_state_with_parent().map_err(|error| {
            Self::rollback_camera_branch(
                &self.pipeline,
                &livekit_sink,
                &mut branch,
                &branch_src,
                &livekit_pad,
            );

            CamError::PipelineError(format!("failed to synchronize camera branch: {error}"))
        })?;

        self.pipeline
            .set_state(gst::State::Playing)
            .map_err(|error| {
                Self::rollback_camera_branch(
                    &self.pipeline,
                    &livekit_sink,
                    &mut branch,
                    &branch_src,
                    &livekit_pad,
                );

                CamError::PipelineError(format!("failed to start media pipeline: {error}"))
            })?;

        self.publisher.borrow_mut().links.insert(
            id.as_str().to_owned(),
            PublisherLink {
                branch_src,
                sink_pad: livekit_pad,
                valve: branch.valve.downgrade(),
            },
        );
        self.branches.insert(id, branch);

        Ok(())
    }

    fn rollback_camera_branch(
        pipeline: &gst::Pipeline,
        livekit_sink: &gst::Element,
        branch: &mut CameraBranch,
        branch_src: &gst::Pad,
        livekit_pad: &gst::Pad,
    ) {
        let _ = branch.bin.set_state(gst::State::Null);

        let _ = branch_src.unlink(livekit_pad);
        livekit_sink.release_request_pad(livekit_pad);

        if let Err(error) = pipeline.remove(&branch.bin) {
            log::error!("Failed to remove camera branch during rollback: {error}");
        }
    }

    pub fn remove_camera(&mut self, id: &CameraId) -> Result<(), CamError> {
        let branch = self
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
        let (livekit_sink, livekit_pad) = {
            let publisher = self.publisher.borrow();
            let link = match publisher.links.get(id.as_str()) {
                Some(link) => link,
                None => {
                    self.branches.insert(id.clone(), branch);
                    return Err(CamError::RemoveDeviceFailed(format!(
                        "camera branch {} has no publisher link",
                        id.as_str()
                    )));
                }
            };
            (publisher.sink.clone(), link.sink_pad.clone())
        };

        if let Err(error) = branch.bin.set_state(gst::State::Null) {
            self.branches.insert(id.clone(), branch);
            return Err(CamError::RemoveDeviceFailed(format!(
                "failed to stop camera branch {}: {error}",
                id.as_str()
            )));
        }

        if let Err(error) = branch_src.unlink(&livekit_pad) {
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

        if let Err(error) = self.pipeline.remove(&branch.bin) {
            if let Err(recovery_error) = branch_src.link(&livekit_pad) {
                log::error!(
                    "Failed to relink camera {} after removal failed: {recovery_error}",
                    id.as_str()
                );
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

        livekit_sink.release_request_pad(&livekit_pad);
        self.publisher.borrow_mut().links.remove(id.as_str());

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
            valve,
        })
    }

    fn install_bus_watch(
        pipeline: &gst::Pipeline,
        publisher: &Rc<RefCell<Publisher>>,
        config: LiveKitConfig,
        event_tx: Sender<AppEvent>,
    ) -> Result<gst::bus::BusWatchGuard, CamError> {
        let bus = pipeline
            .bus()
            .ok_or_else(|| CamError::PipelineError("Failed to make bus".into()))?;

        let pipeline = pipeline.downgrade();
        let publisher = Rc::downgrade(publisher);
        let publisher_recovery = Rc::new(PublisherRecoveryState::new());

        let bus_watch_guard = bus.add_watch_local(move |_, message| {
            let Some(pipeline) = pipeline.upgrade() else {
                return glib::ControlFlow::Break;
            };
            let Some(publisher) = publisher.upgrade() else {
                return glib::ControlFlow::Break;
            };
            let livekit_sink = publisher.borrow().sink.clone();

            match Self::classify_event(message, &livekit_sink) {
                MediaBusAction::ReplacePublisher => {
                    Self::schedule_publisher_replacement(
                        &pipeline,
                        &publisher,
                        config.clone(),
                        Rc::clone(&publisher_recovery),
                        event_tx.clone(),
                    );
                    glib::ControlFlow::Continue
                }
                MediaBusAction::Notify(event) => {
                    if event_tx.send(AppEvent::Media(event)).is_err() {
                        glib::ControlFlow::Break
                    } else {
                        glib::ControlFlow::Continue
                    }
                }
                MediaBusAction::Ignore => glib::ControlFlow::Continue,
            }
        })?;
        Ok(bus_watch_guard)
    }

    fn classify_event(message: &gst::Message, livekit_sink: &gst::Element) -> MediaBusAction {
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

                let is_livekit_error = error.src().is_some_and(|source| {
                    source == livekit_sink.upcast_ref::<gst::Object>()
                        || source.has_as_ancestor(livekit_sink)
                });

                if is_livekit_error {
                    MediaBusAction::ReplacePublisher
                } else {
                    MediaBusAction::Ignore
                }
            }

            gst::MessageView::Eos(_) => {
                log::warn!("Media pipeline reached EOS");
                MediaBusAction::Notify(MediaEvent::PipelineEos)
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
                MediaBusAction::Ignore
            }

            _ => {
                log::trace!("Ignoring GStreamer bus message: {:?}", message.type_());
                MediaBusAction::Ignore
            }
        }
    }

    fn schedule_publisher_replacement(
        pipeline: &gst::Pipeline,
        publisher: &Rc<RefCell<Publisher>>,
        config: LiveKitConfig,
        recovery: Rc<PublisherRecoveryState>,
        event_tx: Sender<AppEvent>,
    ) {
        let Some(delay) = recovery.schedule(Instant::now()) else {
            log::debug!("A LiveKit publisher replacement is already scheduled");
            return;
        };

        log::warn!(
            "LiveKit publisher failed; replacing in {:.1}s",
            delay.as_secs_f32()
        );

        let pipeline = pipeline.downgrade();
        let publisher = Rc::downgrade(publisher);
        glib::timeout_add_local_once(delay, move || {
            let Some(pipeline) = pipeline.upgrade() else {
                recovery.finish();
                return;
            };
            let Some(publisher) = publisher.upgrade() else {
                recovery.finish();
                return;
            };

            match Self::replace_publisher(&pipeline, &publisher, &config) {
                Ok(()) => {
                    recovery.recovered(Instant::now());
                    log::info!("Replaced the LiveKit publisher");
                }
                Err(CamError::PipelineRebuildError(reason)) => {
                    recovery.escalated();
                    log::error!("Publisher recovery left an inconsistent pipeline: {reason}");
                    if event_tx
                        .send(AppEvent::Media(MediaEvent::PipelineFailed { reason }))
                        .is_err()
                    {
                        log::error!("Failed to request a full media-pipeline rebuild");
                    }
                }
                Err(error) => {
                    recovery.finish();
                    log::error!("Failed to replace the LiveKit publisher: {error}");
                    Self::schedule_publisher_replacement(
                        &pipeline, &publisher, config, recovery, event_tx,
                    );
                }
            }
        });
    }

    fn replace_publisher(
        pipeline: &gst::Pipeline,
        publisher: &Rc<RefCell<Publisher>>,
        config: &LiveKitConfig,
    ) -> Result<(), CamError> {
        Self::replace_publisher_with(pipeline, publisher, "video_%u", || {
            Self::build_livekit_sink(config)
        })
    }

    fn replace_publisher_with<F>(
        pipeline: &gst::Pipeline,
        publisher: &Rc<RefCell<Publisher>>,
        pad_template: &str,
        build_replacement: F,
    ) -> Result<(), CamError>
    where
        F: FnOnce() -> Result<gst::Element, CamError>,
    {
        let (old_sink, old_links) = {
            let publisher = publisher.borrow();
            (publisher.sink.clone(), publisher.links.clone())
        };
        let (_, pipeline_state, pending_pipeline_state) =
            pipeline.state(Some(gst::ClockTime::ZERO));
        let target_state = if pending_pipeline_state == gst::State::VoidPending {
            pipeline_state
        } else {
            pending_pipeline_state
        };
        let replacement = build_replacement()?;
        pipeline.add(&replacement).map_err(|error| {
            CamError::PipelineError(format!("failed to add replacement publisher: {error}"))
        })?;

        let valves: Vec<_> = old_links
            .values()
            .filter_map(|link| link.valve.upgrade())
            .collect();
        for valve in &valves {
            valve.set_property("drop", true);
        }

        if let Err(error) = old_sink.set_state(gst::State::Null) {
            return Err(Self::rollback_publisher_error(
                pipeline,
                &old_sink,
                &replacement,
                &old_links,
                &HashMap::new(),
                format!("failed to stop old LiveKit publisher: {error}"),
            ));
        }

        let mut replacement_links = HashMap::new();
        for (id, old_link) in &old_links {
            if let Err(error) = old_link.branch_src.unlink(&old_link.sink_pad) {
                return Err(Self::rollback_publisher_error(
                    pipeline,
                    &old_sink,
                    &replacement,
                    &old_links,
                    &replacement_links,
                    format!("failed to unlink camera {id} from old publisher: {error}"),
                ));
            }

            let Some(replacement_pad) = replacement.request_pad_simple(pad_template) else {
                return Err(Self::rollback_publisher_error(
                    pipeline,
                    &old_sink,
                    &replacement,
                    &old_links,
                    &replacement_links,
                    format!("replacement publisher refused a pad for camera {id}"),
                ));
            };

            if let Err(error) = old_link.branch_src.link(&replacement_pad) {
                replacement.release_request_pad(&replacement_pad);
                return Err(Self::rollback_publisher_error(
                    pipeline,
                    &old_sink,
                    &replacement,
                    &old_links,
                    &replacement_links,
                    format!("failed to link camera {id} to replacement publisher: {error}"),
                ));
            }

            replacement_links.insert(
                id.clone(),
                PublisherLink {
                    branch_src: old_link.branch_src.clone(),
                    sink_pad: replacement_pad,
                    valve: old_link.valve.clone(),
                },
            );
        }

        if let Err(error) = replacement.sync_state_with_parent() {
            return Err(Self::rollback_publisher_error(
                pipeline,
                &old_sink,
                &replacement,
                &old_links,
                &replacement_links,
                format!("failed to start replacement publisher: {error}"),
            ));
        }

        let (state_result, current_state, pending_state) =
            replacement.state(Some(PUBLISHER_STATE_CHANGE_TIMEOUT));
        if let Err(error) = state_result {
            return Err(Self::rollback_publisher_error(
                pipeline,
                &old_sink,
                &replacement,
                &old_links,
                &replacement_links,
                format!("replacement publisher failed to reach {target_state:?}: {error}"),
            ));
        }
        if current_state != target_state || pending_state != gst::State::VoidPending {
            return Err(Self::rollback_publisher_error(
                pipeline,
                &old_sink,
                &replacement,
                &old_links,
                &replacement_links,
                format!(
                    "replacement publisher did not reach {target_state:?} within {}s (current: {current_state:?}, pending: {pending_state:?})",
                    PUBLISHER_STATE_CHANGE_TIMEOUT.seconds()
                ),
            ));
        }

        if let Err(error) = pipeline.remove(&old_sink) {
            return Err(Self::rollback_publisher_error(
                pipeline,
                &old_sink,
                &replacement,
                &old_links,
                &replacement_links,
                format!("failed to remove old publisher: {error}"),
            ));
        }

        for link in old_links.values() {
            old_sink.release_request_pad(&link.sink_pad);
        }
        {
            let mut publisher = publisher.borrow_mut();
            publisher.sink = replacement;
            publisher.links = replacement_links;
        }
        for valve in &valves {
            valve.set_property("drop", false);
        }

        Ok(())
    }

    fn rollback_publisher_error(
        pipeline: &gst::Pipeline,
        old_sink: &gst::Element,
        replacement: &gst::Element,
        old_links: &HashMap<String, PublisherLink>,
        replacement_links: &HashMap<String, PublisherLink>,
        original_error: String,
    ) -> CamError {
        match Self::rollback_publisher_replacement(
            pipeline,
            old_sink,
            replacement,
            old_links,
            replacement_links,
        ) {
            Ok(()) => CamError::PipelineError(original_error),
            Err(rollback_error) => CamError::PipelineRebuildError(format!(
                "{original_error}; publisher rollback failed: {rollback_error}"
            )),
        }
    }

    fn rollback_publisher_replacement(
        pipeline: &gst::Pipeline,
        old_sink: &gst::Element,
        replacement: &gst::Element,
        old_links: &HashMap<String, PublisherLink>,
        replacement_links: &HashMap<String, PublisherLink>,
    ) -> Result<(), String> {
        let mut errors = Vec::new();

        if let Err(error) = replacement.set_state(gst::State::Null) {
            errors.push(format!("failed to stop replacement publisher: {error}"));
        }

        for (id, link) in replacement_links {
            if link.branch_src.peer().as_ref() == Some(&link.sink_pad)
                && let Err(error) = link.branch_src.unlink(&link.sink_pad)
            {
                errors.push(format!(
                    "failed to unlink camera {id} from replacement publisher: {error}"
                ));
            }
            replacement.release_request_pad(&link.sink_pad);
        }

        match old_sink.parent() {
            Some(parent) if parent == pipeline.clone().upcast::<gst::Object>() => {}
            None => {
                if let Err(error) = pipeline.add(old_sink) {
                    errors.push(format!("failed to restore old publisher: {error}"));
                }
            }
            Some(parent) => errors.push(format!(
                "old publisher belongs to unexpected parent {}",
                parent.path_string()
            )),
        }

        for (id, link) in old_links {
            match link.branch_src.peer() {
                Some(peer) if peer == link.sink_pad => {}
                None => {
                    if let Err(error) = link.branch_src.link(&link.sink_pad) {
                        errors.push(format!(
                            "failed to restore camera {id} to old publisher: {error}"
                        ));
                    }
                }
                Some(peer) => errors.push(format!(
                    "camera {id} remains linked to unexpected pad {}",
                    peer.path_string()
                )),
            }
        }

        if replacement.parent().is_some()
            && let Err(error) = pipeline.remove(replacement)
        {
            errors.push(format!("failed to remove replacement publisher: {error}"));
        }

        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors.join("; "))
        }
    }

    fn build_livekit_sink(config: &LiveKitConfig) -> Result<gst::Element, CamError> {
        let sink = gst::ElementFactory::make("livekitwebrtcsink")
            .build()
            .map_err(|_| {
                CamError::ElementCreationFailed("failed to create livekitwebrtcsink".into())
            })?;

        let child_proxy = sink
            .clone()
            .dynamic_cast::<gst::ChildProxy>()
            .map_err(|_| CamError::MonitorError("failed to make child proxy".into()))?;

        child_proxy.set_child_property("signaller::ws-url", config.ws_url.as_str());
        child_proxy.set_child_property("signaller::auth-token", config.auth_token.as_str());
        Ok(sink)
    }
}

impl Drop for RoverMediaPipeline {
    fn drop(&mut self) {
        let _ = self.pipeline.set_state(gst::State::Null);
    }
}
