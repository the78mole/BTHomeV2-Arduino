#ifndef BT_HOME_DATA_TYPES_H
#define BT_HOME_DATA_TYPES_H

#pragma once
#include <Arduino.h>
#include <vector>

struct BtHomeType
{
    uint8_t id;  // Type ID
    float scale; // Multiplier to apply before serializing
    uint8_t byteCount;
    bool signed_value; // true if value is signed, false if unsigned
};

const BtHomeType temperature_int8 = {0x57, 1.0f, 1, true};
const BtHomeType temperature_int8_scale_0_35 = {0x58, 0.35f, 1, true};
const BtHomeType temperature_int16_scale_0_1 = {0x45, 0.1f, 2, true};
const BtHomeType temperature_int16_scale_0_01 = {0x02, 0.01f, 2, true};

const BtHomeType count_uint8 = {0x09, 1.0f, 1, false};
const BtHomeType count_uint16 = {0x3D, 1.0f, 2, false};
const BtHomeType count_uint32 = {0x3E, 1.0f, 4, true};
const BtHomeType count_int8 = {0x59, 1.0f, 1, true};
const BtHomeType count_int16 = {0x5A, 1.0f, 2, true};
const BtHomeType count_int32 = {0x5B, 1.0f, 4, true};

const BtHomeType voltage_0_001 = {0x0C, 0.001f, 2, false};
const BtHomeType voltage_0_1 = {0x4A, 0.1f, 2, false};

const BtHomeType battery_percentage = {0x01, 1.0f, 1, false};

const BtHomeType distance_millimetre = {0x40, 1.0f, 2, false};
const BtHomeType distance_metre = {0x41, 0.1f, 2, false};

const BtHomeType acceleration = {0x51, 0.001f, 2, false};
const BtHomeType channel = {0x60, 1.0f, 1, false};
const BtHomeType co2 = {0x12, 1.0f, 2, false};
const BtHomeType conductivity = {0x56, 1.0f, 2, false};

const BtHomeType current_uint16 = {0x43, 0.001f, 2, false};
const BtHomeType current_int16 = {0x5D, 0.001f, 2, true};
const BtHomeType dewpoint = {0x08, 0.01f, 2, true};
const BtHomeType direction = {0x5E, 0.01f, 2, false};
const BtHomeType duration_uint24 = {0x42, 0.001f, 3, false};
const BtHomeType energy_uint32 = {0x4D, 0.001f, 4, true};
const BtHomeType energy_uint24 = {0x0A, 0.001f, 3, false};
const BtHomeType gas_uint24 = {0x4B, 0.001f, 3, false};
const BtHomeType gas_uint32 = {0x4C, 0.001f, 4, true};
const BtHomeType gyroscope = {0x52, 0.001f, 2, false};
const BtHomeType humidity_uint16 = {0x03, 0.01f, 2, false};
const BtHomeType humidity_uint8 = {0x2E, 1.0f, 1, false};
const BtHomeType illuminance = {0x05, 0.01f, 3, false};
const BtHomeType mass_kg = {0x06, 0.01f, 2, false};
const BtHomeType mass_lb = {0x07, 0.01f, 2, false};
const BtHomeType moisture_uint16 = {0x14, 0.01f, 2, false};
const BtHomeType moisture_uint8 = {0x2F, 1.0f, 1, false};
const BtHomeType pm2_5 = {0x0D, 1.0f, 2, false};
const BtHomeType pm10 = {0x0E, 1.0f, 2, false};
const BtHomeType power_uint24 = {0x0B, 0.01f, 3, false};
const BtHomeType power_int32 = {0x5C, 0.01f, 4, true};
;
const BtHomeType precipitation = {0x5F, 0.1f, 2, false};
const BtHomeType pressure = {0x04, 0.01f, 3, false};
const BtHomeType rotation = {0x3F, 0.1f, 2, true};
const BtHomeType speed = {0x44, 0.01f, 2, false};
const BtHomeType timestamp = {0x50, 1.0f, 5, false};
const BtHomeType tvoc = {0x13, 1.0f, 2, false};

const BtHomeType volume_uint32 = {0x4E, 0.001f, 4, true};
const BtHomeType volume_uint16_scale_0_1 = {0x47, 0.1f, 2, false};
const BtHomeType volume_uint16_scale_1 = {0x48, 1.0f, 2, false};
const BtHomeType volume_storage = {0x55, 0.001f, 4, true};
const BtHomeType volume_flow_rate = {0x49, 0.001f, 2, false};
const BtHomeType UV_index = {0x46, 0.1f, 1, false};
const BtHomeType water_litre = {0x4F, 0.001f, 4, true};
// text (0x53) requires custom serialization
// raw (0x54) and text (0x53) require custom serialization, not included here

#endif // BT_HOME_DATA_TYPES_H