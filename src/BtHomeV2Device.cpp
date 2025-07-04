#include "BtHomeV2Device.h"

void BtHomeV2Device::clearMeasurementData()
{
    return _baseDevice.resetMeasurement();
}

/// @brief Builds an outgoing wrapper for the current measurement data.
/// @param payload
/// @return
size_t BtHomeV2Device::getAdvertisementData(uint8_t *buffer)
{
    return _baseDevice.getAdvertisementData(buffer);
}

BtHomeV2Device::BtHomeV2Device(const char *shortName, const char *completeName, bool isTriggerDevice) : _baseDevice(shortName, completeName, isTriggerDevice) // Initialize with default device name and trigger-based device flag
{
}

bool BtHomeV2Device::addDistanceMetres(float metres)
{
    return _baseDevice.addFloat(distance_metre, metres);
}

bool BtHomeV2Device::addDistanceMillimetres(float millimetres)
{
    return _baseDevice.addFloat(distance_millimetre, millimetres);
}

bool BtHomeV2Device::addTemperature(float degreesCelsius, TemperatureRangeResolution rangeResolution)
{

    switch (rangeResolution)
    {
    case RANGE_44_RESOLUTION_0_35:
    {
        return _baseDevice.addFloat(temperature_int8_scale_0_35, degreesCelsius);
    }
    case RANGE_127_RESOLUTION_1:
    {
        return _baseDevice.addFloat(temperature_int8, degreesCelsius);
    }
    case RANGE_3276_RESOLUTION_0_1:
    {
        return _baseDevice.addFloat(temperature_int16_scale_0_1, degreesCelsius);
    }
    case RANGE_327_RESOLUTION_0_01:
    {
        return _baseDevice.addFloat(temperature_int16_scale_0_01, degreesCelsius);
    }
    }
    return false;
}

bool BtHomeV2Device::addCount_0_4294967295(uint32_t count)
{
    return _baseDevice.addUnsignedInteger(count_uint32, count);
}

bool BtHomeV2Device::addCount_0_255(uint8_t count)
{
    Serial.print("Adding count 0-255: ");
    Serial.println(count);
    return _baseDevice.addUnsignedInteger(count_uint8, count);
}
bool BtHomeV2Device::addCount_0_65535(uint16_t count)
{
    return _baseDevice.addUnsignedInteger(count_uint16, count);
}
bool BtHomeV2Device::addCount_neg128_127(int8_t count)
{
    return _baseDevice.addSignedInteger(count_int8, static_cast<uint64_t>(count));
}
bool BtHomeV2Device::addCount_neg32768_32767(int16_t count)
{
    return _baseDevice.addSignedInteger(count_int16, static_cast<uint64_t>(count));
}
bool BtHomeV2Device::addCount_neg2147483648_2147483647(int32_t count)
{
    return _baseDevice.addSignedInteger(count_int32, static_cast<uint64_t>(count));
}

bool BtHomeV2Device::addHumidity_0_01(float humidityPercent)
{
    return _baseDevice.addFloat(humidity_uint16, humidityPercent);
}

bool BtHomeV2Device::addHumidity_0_1(float humidityPercent)
{
    return _baseDevice.addFloat(humidity_uint8, humidityPercent);
}

bool BtHomeV2Device::addVoltage(float voltage, VoltageRangeResolution rangeResolution)
{

    switch (rangeResolution)
    {
    case RANGE_65_RESOLUTION_0_001:
    {
        return _baseDevice.addFloat(voltage_0_001, voltage);
    }
    case RANGE_65535_RESOLUTION_0_1:
    {
        return _baseDevice.addFloat(voltage_0_1, voltage);
    }
    }
    return false;
}

bool BtHomeV2Device::addText(const char text[])
{
    return _baseDevice.addRaw(0x53, (uint8_t *)text, strlen(text));
}

bool BtHomeV2Device::addTime(uint32_t secondsSinceEpoch)
{
    return _baseDevice.addUnsignedInteger(timestamp, secondsSinceEpoch);
}

bool BtHomeV2Device::addRaw(uint8_t *bytes, uint8_t size)
{
    return _baseDevice.addRaw(0x54, bytes, size);
}

bool BtHomeV2Device::addBatteryPercentage(uint8_t batteryPercentage)
{
    return _baseDevice.addUnsignedInteger(battery_percentage, batteryPercentage);
}

bool BtHomeV2Device::setBatteryState(BATTERY_STATE batteryState)
{
    return _baseDevice.addState(battery_state, batteryState);
}

bool BtHomeV2Device::setBatteryChargingState(Battery_Charging_Sensor_Status batteryChargingState)
{
    return _baseDevice.addState(battery_charging, batteryChargingState);
}

bool BtHomeV2Device::setCarbonMonoxideState(Carbon_Monoxide_Sensor_Status carbonMonoxideState)
{
    return _baseDevice.addState(carbon_monoxide, carbonMonoxideState);
}

bool BtHomeV2Device::setColdState(Cold_Sensor_Status coldState)
{
    return _baseDevice.addState(cold, coldState);
}

bool BtHomeV2Device::setConnectivityState(Connectivity_Sensor_Status connectivityState)
{
    return _baseDevice.addState(connectivity, connectivityState);
}

bool BtHomeV2Device::setDoorState(Door_Sensor_Status doorState)
{
    return _baseDevice.addState(door, doorState);
}

bool BtHomeV2Device::setGarageDoorState(Garage_Door_Sensor_Status garageDoorState)
{
    return _baseDevice.addState(garage_door, garageDoorState);
}

bool BtHomeV2Device::setGasState(Gas_Sensor_Status gasState)
{
    return _baseDevice.addState(gas, gasState);
}

bool BtHomeV2Device::setGenericState(Generic_Sensor_Status genericState)
{
    return _baseDevice.addState(generic_boolean, genericState);
}

bool BtHomeV2Device::setHeatState(Heat_Sensor_Status heatState)
{
    return _baseDevice.addState(heat, heatState);
}

bool BtHomeV2Device::setLightState(Light_Sensor_Status lightState)
{
    return _baseDevice.addState(light, lightState);
}

bool BtHomeV2Device::setLockState(Lock_Sensor_Status lockState)
{
    return _baseDevice.addState(lock, lockState);
}

bool BtHomeV2Device::setMoistureState(Moisture_Sensor_Status moistureState)
{
    return _baseDevice.addState(moisture, moistureState);
}

bool BtHomeV2Device::setMotionState(Motion_Sensor_Status motionState)
{
    return _baseDevice.addState(motion, motionState);
}

bool BtHomeV2Device::setMovingState(Moving_Sensor_Status movingState)
{
    return _baseDevice.addState(moving, movingState);
}

bool BtHomeV2Device::setOccupancyState(Occupancy_Sensor_Status occupancyState)
{
    return _baseDevice.addState(occupancy, occupancyState);
}

bool BtHomeV2Device::setOpeningState(Opening_Sensor_Status openingState)
{
    return _baseDevice.addState(opening, openingState);
}

bool BtHomeV2Device::setPlugState(Plug_Sensor_Status plugState)
{
    return _baseDevice.addState(plug, plugState);
}

bool BtHomeV2Device::setPowerState(Power_Sensor_Status powerState)
{
    return _baseDevice.addState(power, powerState);
}

bool BtHomeV2Device::setPresenceState(Presence_Sensor_Status presenceState)
{
    return _baseDevice.addState(presence, presenceState);
}

bool BtHomeV2Device::setProblemState(Problem_Sensor_Status problemState)
{
    return _baseDevice.addState(problem, problemState);
}

bool BtHomeV2Device::setRunningState(Running_Sensor_Status runningState)
{
    return _baseDevice.addState(running, runningState);
}

bool BtHomeV2Device::setSafetyState(Safety_Sensor_Status safetyState)
{
    return _baseDevice.addState(safety, safetyState);
}

bool BtHomeV2Device::setSmokeState(Smoke_Sensor_Status smokeState)
{
    return _baseDevice.addState(smoke, smokeState);
}

bool BtHomeV2Device::setSoundState(Sound_Sensor_Status soundState)
{
    return _baseDevice.addState(sound, soundState);
}

bool BtHomeV2Device::setTamperState(Tamper_Sensor_Status tamperState)
{
    return _baseDevice.addState(tamper, tamperState);
}

bool BtHomeV2Device::setVibrationState(Vibration_Sensor_Status vibrationState)
{
    return _baseDevice.addState(vibration, vibrationState);
}

bool BtHomeV2Device::setWindowState(Window_Sensor_Status windowState)
{
    return _baseDevice.addState(window, windowState);
}

bool BtHomeV2Device::eventButton(Button_Event_Status buttonEvent)
{
    return _baseDevice.addState(button, buttonEvent);
}

bool BtHomeV2Device::eventDimmer(Dimmer_Event_Status dimmerEvent)
{
    return _baseDevice.addState(dimmer, dimmerEvent);
}
