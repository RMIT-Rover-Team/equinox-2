use gstreamer as gst;

mod device;
mod discovery;
mod error;

use discovery::{DeviceDiscovery, DiscoveryEvent};

fn main() -> anyhow::Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    gst::init()?;

    let main_loop = glib::MainLoop::new(None, false);

    log::info!("Starting device discovery...");

    let _discovery = DeviceDiscovery::start(|event| match event {
        DiscoveryEvent::Added(device) => {
            log::info!("Device added: {:?}", device);
        }
        DiscoveryEvent::Removed(device) => {
            log::info!("Device removed: {:?}", device);
        }
    })?;

    main_loop.run();
    Ok(())
}
