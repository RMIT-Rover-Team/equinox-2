//
// Created by Nick Waites on 21/6/2026.
//

#include "SciencePayload.h"

SciencePayload::SciencePayload() {
}

SciencePayload::~SciencePayload() {
}

void SciencePayload::set_heater_state(uint8_t temperature) {
}

void SciencePayload::set_heater_state(bool state) {
}

void SciencePayload::motor_cycle() {
}

void SciencePayload::set_servo_position(int device_id, float cm) {
}

void SciencePayload::set_servo_velocity(int device_id, float cm_per_second) {
}

void SciencePayload::set_microscope_swivel(int device_id, float degrees) {
}

void SciencePayload::stop_all() {
}

int SciencePayload::get_heater_state() {
}

int SciencePayload::get_heater_current_temperature() {
}

float SciencePayload::get_servo_position(int device_id) {
}

float SciencePayload::get_servo_velocity(int device_id) {
}

float SciencePayload::get_microscope_swivel(int device_id) {
}
