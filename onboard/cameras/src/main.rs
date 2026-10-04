use config::LiveKitConfig;
use gstreamer as gst;
use std::{sync::mpsc, time::Duration};

mod app;
mod config;
mod device;
mod discovery;
mod error;
mod events;
mod media;

use app::CameraApp;
use discovery::DeviceDiscovery;
use events::AppEvent;

fn main() -> anyhow::Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    gst::init()?;

    let main_loop = glib::MainLoop::new(None, false);

    let config = LiveKitConfig {
        ws_url: std::env::var("LIVEKIT_WS_URL")?,
        auth_token: std::env::var("LIVEKIT_AUTH_TOKEN")?,
    };
    let (event_tx, event_rx) = mpsc::channel::<AppEvent>();
    let mut app = CameraApp::start(&config, event_tx.clone())?;

    log::info!("Starting device discovery...");

    let _discovery = DeviceDiscovery::start(event_tx)?;
    let _event_pump = glib::timeout_add_local(Duration::from_millis(50), move || {
        while let Ok(event) = event_rx.try_recv() {
            if let Err(error) = app.handle_event(event) {
                log::error!("Failed to handle application event: {error}");
            }
        }

        if let Err(error) = app.poll() {
            log::error!("Failed to recover camera media: {error}");
        }

        glib::ControlFlow::Continue
    });

    main_loop.run();
    Ok(())
}
