use crate::error::CamError;
use gstreamer as gst;
use gstreamer::prelude::*;
use std::collections::{HashMap, hash_map::Entry};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct CameraId(String);

impl CameraId {
    pub fn as_str(&self) -> &str {
        &self.0
    }
}

#[allow(dead_code)]
pub struct CameraHardware {
    pub name: String,
    pub path: String,
    pub device: gst::Device,
    pub caps: gst::Caps,
}

pub struct DeviceCatalog {
    devices: HashMap<CameraId, CameraHardware>,
    device_to_id: HashMap<gst::Device, CameraId>,
}

impl DeviceCatalog {
    pub fn new() -> Self {
        Self {
            devices: HashMap::new(),
            device_to_id: HashMap::new(),
        }
    }

    // TODO: check cameras with same model have different device.serial values
    pub fn add(&mut self, device: gst::Device) -> Result<CameraId, CamError> {
        let properties = device
            .properties()
            .ok_or_else(|| CamError::PropertyNotFound("device properties".into()))?;

        let identity = properties
            .get::<String>("device.serial")
            .or_else(|_| properties.get::<String>("device.bus_path"))
            .or_else(|_| properties.get::<String>("v4l2.device.bus_info"))
            .map_err(|_| CamError::PropertyNotFound("stable device identity".into()))?;

        let path = properties
            .get::<String>("device.path")
            .map_err(|_| CamError::PropertyNotFound("device.path".into()))?;

        let caps = device
            .caps()
            .ok_or_else(|| CamError::PropertyNotFound("device caps".into()))?;

        let id = CameraId(identity);

        let hardware = CameraHardware {
            name: device.display_name().to_string(),
            path,
            device: device.clone(),
            caps,
        };

        match self.devices.entry(id.clone()) {
            Entry::Occupied(_) => Err(CamError::DuplicateDevice(id.as_str().to_owned())),
            Entry::Vacant(entry) => {
                entry.insert(hardware);
                self.device_to_id.insert(device, id.clone());
                Ok(id)
            }
        }
    }

    pub fn id_for_device(&self, device: &gst::Device) -> Result<CameraId, CamError> {
        self.device_to_id
            .get(device)
            .cloned()
            .ok_or_else(|| CamError::DeviceNotFound("device is not registered".into()))
    }

    pub fn get(&self, id: &CameraId) -> Option<&CameraHardware> {
        self.devices.get(id)
    }

    pub fn remove(&mut self, id: &CameraId) -> Option<CameraHardware> {
        let hardware = self.devices.remove(id)?;
        self.device_to_id.remove(&hardware.device);

        Some(hardware)
    }
}
