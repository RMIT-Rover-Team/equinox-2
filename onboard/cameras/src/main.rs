use config::LiveKitConfig;
use gstreamer as gst;

mod app;
mod config;
mod device;
mod discovery;
mod error;
mod media;

use app::CameraApp;
use discovery::DeviceDiscovery;

fn main() -> anyhow::Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    gst::init()?;

    let main_loop = glib::MainLoop::new(None, false);

    let config = LiveKitConfig {
        ws_url: std::env::var("LIVEKIT_WS_URL")?,
        auth_token: std::env::var("LIVEKIT_AUTH_TOKEN")?,
    };
    let mut app = CameraApp::start(&config)?;

    log::info!("Starting device discovery...");

    let _discovery = DeviceDiscovery::start(move |event| {
        if let Err(error) = app.handle_discovery(event) {
            log::error!("Failed to handle discovery event: {error}");
        }
    })?;

    main_loop.run();
    Ok(())
}
