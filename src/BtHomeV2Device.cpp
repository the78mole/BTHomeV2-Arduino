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

bool BtHomeV2Device::addBatteryPercentage(uint8_t batteryPercentage)
{
    return _baseDevice.addUnsignedInteger(battery_percentage, batteryPercentage);
}

bool BtHomeV2Device::setBatteryState(BATTERY_STATE batteryState)
{
    return _baseDevice.addState(battery_state, batteryState);
}
