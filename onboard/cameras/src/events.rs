use gstreamer as gst;

#[derive(Debug)]
pub enum AppEvent {
    Discovery(DiscoveryEvent),
    Media(MediaEvent),
}

#[derive(Debug)]
pub enum DiscoveryEvent {
    Added(gst::Device),
    Removed(gst::Device),
}

#[derive(Debug)]
pub enum MediaEvent {
    LiveKitDisconnected { reason: String },
    EndOfStream,
}
