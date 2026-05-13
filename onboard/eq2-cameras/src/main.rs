// #![warn(missing_docs)]

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
use stream_registry::StreamRegistry;

use crate::camera_types::DiscoveryEvent;

/// Main loop for cameras.
/// 
/// Handles all the logic and the glib MainLoop
fn main() -> anyhow::Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();
    gst::init()?;

    let main_loop = glib::MainLoop::new(None, false);
    let device_registry = Arc::new(Mutex::new(DeviceRegistry::new()));
    let stream_registry = Arc::new(Mutex::new(StreamRegistry::new()));
    
    let mut discovery = DeviceDiscovery::new(device_registry.clone());

    let dev_reg = device_registry.clone();
    let str_reg = stream_registry.clone();

    log::info!("Starting device discovery...");
    discovery.start(move |event| {
        match event {
            DiscoveryEvent::Added(uid) => {
                log::info!("New device detected: {}", uid);
                
                let mut s_reg = str_reg.lock();
                let d_reg = dev_reg.lock();
                
                if let Some(device) = d_reg.get_device(&uid) {
                    if let Err(e) = s_reg.create_stream(&uid, &device) {
                        log::error!("Failed to start stream for {}: {}", uid, e);
                    }
                }
            }
            DiscoveryEvent::Removed(uid) => {
                log::info!("Device removed: {}", uid);
                str_reg.lock().remove_stream(&uid);
            }
        }
     })?;
    
    main_loop.run();

    discovery.stop();

    Ok(())
}