#![warn(missing_docs)]

use gstreamer as gst;
use std::sync::Arc;
use parking_lot::Mutex;
use glib;

mod camera_types;
mod device_registry;
mod device_discovery;
mod stream_registry;

use device_registry::DeviceRegistry;
use device_discovery::DeviceDiscovery;

/// Main loop for cameras.
/// 
/// Handles all the logic and the glib MainLoop
fn main() -> anyhow::Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();
    gst::init()?;

    let main_loop = glib::MainLoop::new(None, false);
    let device_registry = Arc::new(Mutex::new(DeviceRegistry::new()));
    let mut discovery = DeviceDiscovery::new(device_registry.clone());

    log::info!("Starting device discovery...");
    discovery.start(|count| {
        log::info!("Registry Changed! Current device count: {}", count);
    })?;
    
    main_loop.run();

    discovery.stop();

    Ok(())
}