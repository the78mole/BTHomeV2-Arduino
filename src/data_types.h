#ifndef BT_HOME_DATA_TYPES_H
#define BT_HOME_DATA_TYPES_H

#pragma once
#include <Arduino.h>
#include <vector>

// You can use struct inheritance in C++ to inherit properties.
// For example, BtHomeType can inherit from BtHomeState if you want to reuse the 'id' field:

struct BtHomeState
{
    uint8_t id;
    uint8_t byteCount;
};

struct BtHomeType : public BtHomeState
{
    float scale; // Multiplier to apply before serializing
    bool signed_value; // true if value is signed, false if unsigned

    BtHomeType(uint8_t id, float scale, uint8_t byteCount, bool signed_value)
    {
        this->id = id;
        this->scale = scale;
        this->byteCount = byteCount;
        this->signed_value = signed_value;
    }
};

// Now BtHomeType has 'id' from BtHomeState, plus its own fields.


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


const BtHomeState battery_state = {0x15,1}; // Battery state, 1 byte, 0 = normal, 1 = low
const BtHomeState battery_charging = {0x16, 1};
const BtHomeState carbon_monoxide = {0x17, 1};
const BtHomeState cold = {0x18, 1};
const BtHomeState connectivity = {0x19, 1};
const BtHomeState door = {0x1A, 1};
const BtHomeState garage_door = {0x1B, 1};
const BtHomeState gas = {0x1C, 1};
const BtHomeState generic_boolean = {0x0F, 1};
const BtHomeState heat = {0x1D, 1};
const BtHomeState light = {0x1E, 1};
const BtHomeState lock = {0x1F, 1};
const BtHomeState moisture = {0x20, 1};
const BtHomeState motion = {0x21, 1};
const BtHomeState moving = {0x22, 1};
const BtHomeState occupancy = {0x23, 1};
const BtHomeState opening = {0x11, 1};
const BtHomeState plug = {0x24, 1};
const BtHomeState power = {0x10, 1};
const BtHomeState presence = {0x25, 1};
const BtHomeState problem = {0x26, 1};
const BtHomeState running = {0x27, 1};
const BtHomeState safety = {0x28, 1};
const BtHomeState smoke = {0x29, 1};
const BtHomeState sound = {0x2A, 1};
const BtHomeState tamper = {0x2B, 1};
const BtHomeState vibration = {0x2C, 1};
const BtHomeState window = {0x2D, 1};

enum Battery_Sensor_Status 
{
    Battery_Normal = 0,    // False = Normal
    Battery_Low    = 1     // True  = Low
};

enum Battery_Charging_Sensor_Status 
{
    Battery_Not_Charging = 0,  // False = Not Charging
    Battery_Charging     = 1   // True  = Charging
};

enum Carbon_Monoxide_Sensor_Status 
{
    Carbon_Monoxide_Not_Detected = 0,  // False = Not detected
    Carbon_Monoxide_Detected     = 1   // True  = Detected
};

enum Cold_Sensor_Status 
{
    Cold_Normal = 0,  // False = Normal
    Cold_Cold   = 1   // True  = Cold
};

enum Connectivity_Sensor_Status 
{
    Connectivity_Disconnected = 0,  // False = Disconnected
    Connectivity_Connected    = 1   // True  = Connected
};

enum Door_Sensor_Status 
{
    Door_Closed = 0,  // False = Closed
    Door_Open   = 1   // True  = Open
};

enum Garage_Door_Sensor_Status 
{
    Garage_Door_Closed = 0,  // False = Closed
    Garage_Door_Open   = 1   // True  = Open
};

enum Gas_Sensor_Status 
{
    Gas_Clear    = 0,  // False = Clear
    Gas_Detected = 1   // True  = Detected
};

enum Generic_Sensor_Status 
{
    Generic_Off = 0,   // False = Off
    Generic_On  = 1    // True  = On
};

enum Heat_Sensor_Status 
{
    Heat_Normal = 0,  // False = Normal
    Heat_Hot    = 1   // True  = Hot
};

enum Light_Sensor_Status 
{
    Light_No_Light  = 0,  // False = No light
    Light_Detected  = 1   // True  = Light detected
};

enum Lock_Sensor_Status 
{
    Lock_Locked   = 0,  // False = Locked
    Lock_Unlocked = 1   // True  = Unlocked
};

enum Moisture_Sensor_Status 
{
    Moisture_Dry = 0,   // False = Dry
    Moisture_Wet = 1    // True  = Wet
};

enum Motion_Sensor_Status 
{
    Motion_Clear    = 0,  // False = Clear
    Motion_Detected = 1   // True  = Detected
};

enum Moving_Sensor_Status 
{
    Moving_Not_Moving = 0,  // False = Not moving
    Moving_Moving     = 1   // True  = Moving
};

enum Occupancy_Sensor_Status 
{
    Occupancy_Clear    = 0,  // False = Clear
    Occupancy_Detected = 1   // True  = Detected
};

enum Opening_Sensor_Status 
{
    Opening_Closed = 0,  // False = Closed
    Opening_Open   = 1   // True  = Open
};

enum Plug_Sensor_Status 
{
    Plug_Unplugged = 0,  // False = Unplugged
    Plug_Plugged_In = 1  // True  = Plugged in
};

enum Power_Sensor_Status 
{
    Power_Off = 0,  // False = Off
    Power_On  = 1   // True  = On
};

enum Presence_Sensor_Status 
{
    Presence_Away = 0,  // False = Away
    Presence_Home = 1   // True  = Home
};

enum Problem_Sensor_Status 
{
    Problem_OK      = 0,  // False = OK
    Problem_Problem = 1   // True  = Problem
};

enum Running_Sensor_Status 
{
    Running_Not_Running = 0,  // False = Not Running
    Running_Running     = 1   // True  = Running
};

enum Safety_Sensor_Status 
{
    Safety_Unsafe = 0,  // False = Unsafe
    Safety_Safe   = 1   // True  = Safe
};

enum Smoke_Sensor_Status 
{
    Smoke_Clear    = 0,  // False = Clear
    Smoke_Detected = 1   // True  = Detected
};

enum Sound_Sensor_Status 
{
    Sound_Clear    = 0,  // False = Clear
    Sound_Detected = 1   // True  = Detected
};

enum Tamper_Sensor_Status 
{
    Tamper_Off = 0,  // False = Off
    Tamper_On  = 1   // True  = On
};

enum Vibration_Sensor_Status 
{
    Vibration_Clear    = 0,  // False = Clear
    Vibration_Detected = 1   // True  = Detected
};

enum Window_Sensor_Status 
{
    Window_Closed = 0,  // False = Closed
    Window_Open   = 1   // True  = Open
};


#endif // BT_HOME_DATA_TYPES_H

