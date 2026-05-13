//! Module to store GST devices.
//! 
//! Makes 2 HashMaps, one mapping device details to `api.v4l2.cap.bus_info` as a 
//! key, and the other mapping the 1st hashmap's keys to a GST Device. The reason 
//! why there are 2 hashmaps is to retrieve the uid in a case where it is unobtainable.
//! 
//! The 2nd HashMap is specifically used in device_discovery, where upon unplugging a camera the 
//! device's properties can not be read, in which the GST Device object can be used as a key instead.
//! This comes at the cost of using more space, but realistically who even cares.

use gstreamer as gst;
use gstreamer::prelude::*;
use std::collections::HashMap;
use crate::camera_types::{CamError, CameraHardware};

pub struct DeviceRegistry {
    registry: HashMap<String, CameraHardware>,
    device_to_uid: HashMap<gst::Device, String>,
}

impl DeviceRegistry {
    /// Initialises new DeviceRegistry
    /// 
    /// It makes 2 new hashmaps according to the struct, one mapping uids to registry and the other mapping device objects to uid
    pub fn new() -> Self {
        Self {
            registry: HashMap::new(),
            device_to_uid: HashMap::new(),
        }
    }

    pub fn get_device(&self, uid: &str) -> Option<gst::Device> {
        self.registry.get(uid).map(|hw| hw.device.clone())
    }

    /// Adds device to registry
    /// 
    /// Adds a device to the registry map, with the key based off the uid (api.v4l2.cap.bus_info property).
    /// The function takes in a device object and farts out nothing if successful. 
    /// 
    /// # Arguments
    /// * `device` - A GST Device: `gst::Device`.
    /// # Issues
    /// When the property is not found, there is currently no fallback, which needs to be eventually implemented.
    /// It also only works on linux operating-systems currently due to relying on V4l2.
    /// All devices should come with a uid, due to it currently being linked to the USB Input, but in future if cameras
    /// are to be specced with hardware identifiers, we should opt to use those instead.
    ///
    pub fn handle_device_add(&mut self, device: &gst::Device) -> Result<String, CamError> {
        let props = device.properties().ok_or_else(|| {
            CamError::PropertyNotFound("Device has no properties".to_string())
        })?;
        
        let uid: String = props.get("api.v4l2.cap.bus_info")
            .map_err(|_| CamError::PropertyNotFound("api.v4l2.cap.bus_info".into()))?;
        
        let path: String = props.get("api.v4l2.path")
            .map_err(|_| CamError::PropertyNotFound("api.v4l2.path".into()))?;

        let name = device.display_name().to_string();
        let caps = device.caps().ok_or_else(|| {
            CamError::PropertyNotFound("Device has no caps".into())
        })?;

        log::info!("Adding camera: {} ({})", name, uid);
        
        self.device_to_uid.insert(device.clone(), uid.clone());
        self.registry.insert(uid.clone(), CameraHardware { 
            uid: uid.clone(), name, path, device: device.clone(), caps 
        });
        
        Ok(uid)
    }

    /// Removes device from registry
    /// 
    /// Removes a device from the registry map, based off the camera device.
    pub fn handle_device_remove(&mut self, device: &gst::Device) -> Result<String, CamError> {
        if let Some(uid) = self.device_to_uid.remove(device) {
            self.registry.remove(&uid);
            log::info!("Removed camera: {}", uid);
            Ok(uid)
        } else {
            Err(CamError::DeviceNotFound("Device handle not in registry".into()))
        }
    }
}