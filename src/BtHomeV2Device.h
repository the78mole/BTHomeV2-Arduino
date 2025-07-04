#ifndef BT_HOME_H
#define BT_HOME_H

// https://bthome.io/format/

#include <Arduino.h>
#include "BaseDevice.h"

/**
 * @file BTHome.h
 * @brief BTHome v2 sensor data setter methods with summaries.
 */

static const size_t MAX_PAYLOAD_SIZE = 31;

/// @brief Temperature range resolution options for BTHome temperature data
enum TemperatureRangeResolution
{
    /// @brief Temperature range -127 to 127  degrees Celsius, resolution 1 degree Celsius - 1 byte
    RANGE_127_RESOLUTION_1,
    /// @brief Temperature range -57 to 57 degrees Celsius, resolution 0.35 degrees Celsius - 1 byte
    RANGE_44_RESOLUTION_0_35,
    /// @brief Temperature range -327.68 to 327.67 degrees Celsius, resolution 0.1 degrees Celsius - 2 bytes
    RANGE_327_RESOLUTION_0_01,
    /// @brief Temperature range -3276.8 to 3276.7 degrees Celsius, resolution 0.01 degrees Celsius - 2 bytes
    RANGE_3276_RESOLUTION_0_1
};

/// @brief Voltage range resolution options for BTHome Voltage data
enum VoltageRangeResolution
{
    /// @brief Voltage range 0 to 65.535 volts, resolution 0.001 volts - 2 bytes
    RANGE_65_RESOLUTION_0_001,
    /// @brief Voltage range 0 to 6553.5 volts, resolution 0.1 volts - 2 bytes
    RANGE_65535_RESOLUTION_0_1
};

/// @brief Battery state options
enum BATTERY_STATE
{
    BATTERY_STATE_NORMAL = 0,
    BATTERY_STATE_LOW = 1
};

class BtHomeV2Device
{
public:
    /// @brief
    /// @param shortName Short name of the device - sent when space is limited. Max 12 characters
    /// @param completeName  Full name of the device - sent when space is available
    /// @param isTriggerDevice - If the device sends data when triggered
    BtHomeV2Device(const char *shortName, const char *completeName, bool isTriggerDevice);
    size_t getAdvertisementData(uint8_t *buffer);
    void clearMeasurementData();

    /**
     * @brief Set the voltage value in the packet.
     * @param voltageVolts Voltage in volts.
     */
    bool addVoltage(float voltage, VoltageRangeResolution rangeResolution);

    /**
     * @brief Set a generic count value in the packet.
     * @param count Arbitrary count (e.g., event count).
     */
    bool addCount_0_4294967295(uint32_t count);
    bool addCount_0_255(uint8_t count);
    bool addCount_0_65535(uint16_t count);
    bool addCount_neg128_127(int8_t count);
    bool addCount_neg32768_32767(int16_t count);
    bool addCount_neg2147483648_2147483647(int32_t count);

    /**
     * @brief Set the distance measurement value in the packet.
     * @param distanceMetres Distance in metres.
     */
    bool addDistanceMetres(float distanceMetres);

    /**
     * @brief Set the distance measurement value in the packet.
     * @param distanceMillimetres Distance in metres.
     */
    bool addDistanceMillimetres(float distanceMillimetres);

    /**
     * @brief Set the battery level value in the packet.
     * @param batteryPercentOrMillivolts Battery level as an unsigned 8-bit value (e.g., percentage or mV depending on implementation).
     */
    bool addBatteryPercentage(uint8_t batteryPercentage);

    bool addText(const char text[]);

    bool setBatteryState(BATTERY_STATE batteryState);
    bool setBatteryChargingState(Battery_Charging_Sensor_Status batteryChargingState);
    bool setCarbonMonoxideState(Carbon_Monoxide_Sensor_Status carbonMonoxideState);
    bool setColdState(Cold_Sensor_Status coldState);
    bool setConnectivityState(Connectivity_Sensor_Status connectivityState);
    bool setDoorState(Door_Sensor_Status doorState);
    bool setGarageDoorState(Garage_Door_Sensor_Status garageDoorState);
    bool setGasState(Gas_Sensor_Status gasState);
    bool setGenericState(Generic_Sensor_Status genericState);
    bool setHeatState(Heat_Sensor_Status heatState);
    bool setLightState(Light_Sensor_Status lightState);
    bool setLockState(Lock_Sensor_Status lockState);
    bool setMoistureState(Moisture_Sensor_Status moistureState);
    bool setMotionState(Motion_Sensor_Status motionState);
    bool setMovingState(Moving_Sensor_Status movingState);
    bool setOccupancyState(Occupancy_Sensor_Status occupancyState);
    bool setOpeningState(Opening_Sensor_Status openingState);
    bool setPlugState(Plug_Sensor_Status plugState);
    bool setPowerState(Power_Sensor_Status powerState);
    bool setPresenceState(Presence_Sensor_Status presenceState);
    bool setProblemState(Problem_Sensor_Status problemState);
    bool setRunningState(Running_Sensor_Status runningState);
    bool setSafetyState(Safety_Sensor_Status safetyState);
    bool setSmokeState(Smoke_Sensor_Status smokeState);
    bool setSoundState(Sound_Sensor_Status soundState);
    bool setTamperState(Tamper_Sensor_Status tamperState);
    bool setVibrationState(Vibration_Sensor_Status vibrationState);
    bool setWindowState(Window_Sensor_Status windowState);

    /**
     * @brief Set the temperature value in the packet.
     * @param degreesCelsius Temperature in degrees Celsius.
     * @param rangeResolution The temperature range and resolution.
     */
    bool addTemperature(float degreesCelsius, TemperatureRangeResolution rangeResolution);

    /**
     * @brief Set the humidity value with a resolution of 0.01% in the packet. 2 bytes.
     * @param humidityPercent Relative humidity in percent.
     */
    bool addHumidity_0_01(float humidityPercent);

    /**
     * @brief Set the humidity value with a resolution of 0.1% in the packet. 1 byte.
     * @param humidityPercent Relative humidity in percent.
     */
    bool addHumidity_0_1(float humidityPercent);

    // /**
    //  * @brief Set the illuminance (light level) value in the packet.
    //  * @param illuminanceLux Ambient light in lux.
    //  */
    // bool addIlluminance(float illuminanceLux);

    // /**
    //  * @brief Set the button state in the packet.
    //  * @param buttonState Button state as an unsigned 8-bit value (e.g., pressed/released).
    //  */
    // bool addButton(uint8_t buttonState);

    // /**
    //  * @brief Set the soil moisture value in the packet.
    //  * @param moisturePercent Moisture level as a float (e.g., percentage).
    //  */
    // bool addMoisture(float moisturePercent);

    // /**
    //  * @brief Set the conductivity value in the packet.
    //  * @param conductivityMicroSiemensCm Electrical conductivity in µS/cm.
    //  */
    // bool addConductivity(uint16_t conductivityMicroSiemensCm);

    // /**
    //  * @brief Set the power (wattage) value in the packet.
    //  * @param powerWatts Power in watts.
    //  */
    // bool addPower(float powerWatts);

    // /**
    //  * @brief Set the energy consumption value in the packet.
    //  * @param energyWattHours Energy in watt-hours.
    //  */
    // bool addEnergy(float energyWattHours);

    // /**
    //  * @brief Set the current value in the packet.
    //  * @param currentAmperes Current in amperes.
    //  */
    // bool addCurrent(float currentAmperes);

    // /**
    //  * @brief Set the particulate matter (PM2.5) value in the packet.
    //  * @param pm2_5MicrogramsM3 PM2.5 concentration in µg/m³.
    //  */
    // bool addPM2_5(uint16_t pm2_5MicrogramsM3);

    // /**
    //  * @brief Set the particulate matter (PM10) value in the packet.
    //  * @param pm10MicrogramsM3 PM10 concentration in µg/m³.
    //  */
    // bool addPM10(uint16_t pm10MicrogramsM3);

    // /**
    //  * @brief Set the CO₂ concentration value in the packet.
    //  * @param co2PPM Carbon dioxide concentration in ppm.
    //  */
    // bool addCO2(uint16_t co2PPM);

    // /**
    //  * @brief Set the total volatile organic compounds (TVOC) value in the packet.
    //  * @param tvocPPB TVOC concentration in ppb.
    //  */
    // bool addTVOC(uint16_t tvocPPB);

    // /**
    //  * @brief Set the atmospheric pressure value in the packet.
    //  * @param pressureHpa Pressure in hPa.
    //  */
    // bool addPressure(float pressureHpa);

    // /**
    //  * @brief Set the weight measurement value in the packet.
    //  * @param weightKg Weight in kilograms or grams depending on implementation.
    //  */
    // bool addWeight(float weightKg);

    // /**
    //  * @brief Set the volume measurement value in the packet.
    //  * @param volumeLitres Volume in litres or cubic metres depending on implementation.
    //  */
    // bool addVolume(float volumeLitres);

    // /**
    //  * @brief Set a duration value in the packet.
    //  * @param durationSeconds Duration in seconds or milliseconds depending on implementation.
    //  */
    // bool addDuration(uint32_t durationSeconds);

    // /**
    //  * @brief Set the speed measurement value in the packet.
    //  * @param speedMetresPerSecond Speed in metres per second or km/h.
    //  */
    // bool addSpeed(float speedMetresPerSecond);

    // /**
    //  * @brief Set the ultraviolet (UV) index value in the packet.
    //  * @param uvIndex UV index (unitless).
    //  */
    // bool addUV(float uvIndex);

    // /**
    //  * @brief Set a pulse count or heart rate value in the packet.
    //  * @param pulseBPM Pulse count or heart rate in beats per minute.
    //  */
    // bool addPulse(uint32_t pulseBPM);

    // /**
    //  * @brief Set a generic floating-point value in the packet.
    //  * @param valueFloat Arbitrary float value.
    //  */
    // bool addGenericFloat(float valueFloat);

    // /**
    //  * @brief Set a generic unsigned integer value in the packet.
    //  * @param valueUint Arbitrary unsigned integer value.
    //  */
    // bool addGenericUint(uint32_t valueUint);

    // /**
    //  * @brief Set a generic signed integer value in the packet.
    //  * @param valueInt Arbitrary signed integer value.
    //  */
    // bool addGenericInt(int32_t valueInt);

private:
    BaseDevice _baseDevice;
};

#endif // BT_HOME_H
