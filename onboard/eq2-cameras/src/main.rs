use std::sync::{Arc, Mutex};

use glib::{GString};
use gstreamer::{bus::BusWatchGuard, prelude::*};

use gstreamer as gst;
struct GstApp {
    pipeline: gst::Pipeline,
    monitor: gst::DeviceMonitor,
    _monitor_guard: BusWatchGuard,
    _pipeline_guard: BusWatchGuard,
    devices: Arc<Mutex<Vec<gst::Device>>>,
}

fn monitor_bus_function(_bus: &gst::Bus, message: &gst::Message, devices: &Arc<Mutex<Vec<gst::Device>>>) -> glib::ControlFlow {
    use gst::MessageView;

    match message.view() {
        MessageView::DeviceAdded(msg) => {
            let device = msg.device();
            let name = device.display_name();
            println!("Device added: {}", name);

            let mut list = devices.lock().unwrap();
            list.push(device);
        }
        MessageView::DeviceRemoved(msg) => {
            let device = msg.device();
            let name = device.display_name();
            println!("Device removed: {}", name);
        }
        _ => (),
    }
    glib::ControlFlow::Continue
}

fn pipeline_bus_function(_bus: &gst::Bus, message: &gst::Message) -> glib::ControlFlow {
    use gst::MessageView;
    match message.view() {
        MessageView::Error(err) => {
            eprintln!("Error from {:?}: {}", err.src().map(|s| s.path_string()), err.error());
        }
        MessageView::Eos(_) => println!("End of stream"),
        _ => (),
    }

    glib::ControlFlow::Continue
}

impl GstApp {
    fn new(pipeline_name: &str) -> Self {
        let devices: Arc<Mutex<Vec<gst::Device>>> = Arc::new(Mutex::new(Vec::new()));
        let devices_for_bus = Arc::clone(&devices);
        

        let pipeline = gst::Pipeline::with_name(pipeline_name);
        let pipeline_bus = pipeline.bus().expect("Could not retrieve bus from pipeline");

        let monitor = gst::DeviceMonitor::new();
        monitor.add_filter(Some("Video/Source"), None).expect("Could not find Video/Source");
        let monitor_bus = monitor.bus();

        let monitor_guard = monitor_bus.add_watch(move |bus, msg| {
            monitor_bus_function(bus,msg, &devices_for_bus)
        }).expect("Failed to watch monitor bus");

        let pipeline_guard = pipeline_bus.add_watch(|bus, msg| {
            pipeline_bus_function(bus, msg)
        }).expect("Failed to watch pipeline bus");

        monitor.start().expect("Failed to start monitor");

        GstApp {
            pipeline,
            monitor,
            _monitor_guard: monitor_guard,
            _pipeline_guard: pipeline_guard,
            devices,
        }
    } 
}

fn main() {
    // Print GStreamer Version
    let (major, minor, micro, nano) = gst::version();
    println!("Using GStreamer {}.{}.{}.{}", major, minor, micro, nano);

    gst::init().expect("Failed to init");

    let gst_app: GstApp = GstApp::new("camera_pipeline");

    let source = gst::ElementFactory::make("videotestsrc").name("source").build().expect("No source");
    let filter = gst::ElementFactory::make("identity").name("filter").build().expect("No filter");
    let sink = gst::ElementFactory::make("autovideosink").name("sink").build().expect("No sink");

    gst_app.pipeline.add_many(&[&source, &filter, &sink]).expect("Unable to add elements to pipeline");
    gst::Element::link_many(&[&source, &filter, &sink]).unwrap();
    
    gst_app.pipeline.set_state(gst::State::Playing).expect("Unable to set pipeline to Playing");

    let shared_devices = Arc::clone(&gst_app.devices);
    glib::timeout_add_seconds(5, move || {
        let list = shared_devices.lock().unwrap();
        println!("--- Periodic Report: {} cameras currently connected ---", list.len());
        glib::ControlFlow::Continue
    });

    println!("Running! Plug/Unplug a camera to see the list update.");

    // 6. Start the Main Loop (The program stays here until you hit Ctrl+C)
    let main_loop = glib::MainLoop::new(None, false);
    main_loop.run();


    // let bus: gst::Bus;
    // let _guard = bus.add_watch(bus_function)?;

    // let source = gst::ElementFactory::make("filesrc").name("source")
    //     .build()
    //     .expect("Could not create source");
    
    
    // if gst::
    // let mut silent: i32 = 0;
    // let silent_long = "silent\0";
    // let silent_desc = "Silent mode\0";

    // let entries = vec![
    //     GOptionEntry {.
    //         long_name: silent_long.as_ptr() as *const i8,
    //         short_name: 's' as i8,
    //         flags: 0,
    //         arg: G_OPTION_ARG_NONE,
    //         arg_data: &mut silent as *mut i32 as *mut _,
    //         description: silent_desc.as_ptr() as *const i8,
    //         arg_description: ptr::null(),
    //     },
    //     // Null terminator entry
    //     GOptionEntry {
    //         long_name: ptr::null(),
    //         short_name: 0,
    //         flags: 0,
    //         arg: 0,
    //         arg_data: ptr::null_mut(),
    //         description: ptr::null(),
    //         arg_description: ptr::null(),
    //     },
    // ];
    
    // if let Some(factory) = gst::ElementFactory::find("fakesrc") {
    //     let element = factory.create().name("source").build()
    //     .expect("Failed to create element");
    
    //     let name: GString = element.name();
    //     println!("The name of the element is '{}'.", name);

    //     let klass = factory
    //         .metadata(gst::ELEMENT_METADATA_KLASS)
    //         .unwrap_or("Unknown");
    
    //     let description = factory
    //         .metadata(gst::ELEMENT_METADATA_DESCRIPTION)
    //         .unwrap_or("No description");
    //         println!(
    //             "The '{}' element is a member of the category {}.\nDescription: {}",
    //             name, klass, description
    //     );
    // } else {
    //     eprintln!("Error: fakesrc plugin not found!");
    // }

    // let source = gst::ElementFactory::make("fakesrc").name("source").build().expect("Could not find fakesrc (plugin might be missing");
    // let filter = gst::ElementFactory::make("identity").name("filter").build().expect("Could not find identity (plugin might be missing");
    // let sink = gst::ElementFactory::make("fakesink").name("sink").build().expect("Could not find fakesink (plugin might be missing");

    // let pipeline = gst::Pipeline::new();

    // pipeline.add_many(&[&source, &filter, &sink]).expect("Add many failed. idk why.");
    // source.link(&sink).unwrap();
}
