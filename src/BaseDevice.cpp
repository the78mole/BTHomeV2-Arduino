#include "Arduino.h"
#include "BaseDevice.h"

/// @brief
/// @param shortName - Short name of the device - sent when space is limited. Max 12 characters.
/// @param completeName - Full name of the device - sent when space is available.
/// @param isTriggerBased
BaseDevice::BaseDevice(const char *shortName, const char *completeName, bool isTriggerBased)
    : _triggerDevice(isTriggerBased)
{

  strncpy(_shortName, shortName, MAX_LENGTH_SHORT_NAME);
  _shortName[MAX_LENGTH_SHORT_NAME] = '\0';

  strncpy(_completeName, completeName, MAX_LENGTH_COMPLETE_NAME);
  _completeName[MAX_LENGTH_COMPLETE_NAME] = '\0';

  // 27 bytes left after flags and other headers
  _maximumMeasurementBytes = 27 - strlen(_shortName) - 2; // 2 bytes for the name id and size byte
}

/// @brief Clear the measurement data.
void BaseDevice::resetMeasurement()
{
  _sensorDataIdx = 0;
}

/// @brief Check that there is enough space in the sensor data packet for the given size.
/// @details The sensor data packet has a maximum length defined by MEASUREMENT_MAX_LEN.
/// @param size
/// @return Returns true if there is enough space for the given size, false otherwise.
bool BaseDevice::hasEnoughSpace(BtHomeType sensor)
{
  // minimum space is needed for the short name, but if there is spare room then we can use the full name
  uint8_t shortNameSize = strlen(_shortName) + 2; // 1 byte for the name id and one for the size byte
  return (_sensorDataIdx + sensor.byteCount + 1) <= _maximumMeasurementBytes;
}

/// @brief Add a state or step value to the sensor data packet.
/// @param sensor
/// @param state
/// @param steps
/// @return
bool BaseDevice::addState(BtHomeType sensor, uint8_t state, uint8_t steps)
{
  if ((_sensorDataIdx + 2 + (steps > 0 ? 1 : 0)) <= _maximumMeasurementBytes)
  {
    _sensorData[_sensorDataIdx] = sensor.id;
    _sensorDataIdx++;
    _sensorData[_sensorDataIdx] = static_cast<byte>(state & 0xff);
    _sensorDataIdx++;
    if (steps > 0)
    {
      _sensorData[_sensorDataIdx] = static_cast<byte>(steps & 0xff);
      _sensorDataIdx++;
    }
  }
  else
  {
    return addState(sensor, state, steps);
  }
  return true;
}

bool BaseDevice::addUnsignedInteger(BtHomeType sensor, uint64_t value)
{
  return addInteger(sensor, value);
}

bool BaseDevice::addSignedInteger(BtHomeType sensor, int64_t value)
{
  return addInteger(sensor, value);
}

template <typename T>
bool BaseDevice::addInteger(BtHomeType sensor, T value)
{
  if (!hasEnoughSpace(sensor))
  {
    return false;
  }
  auto scaledValue = static_cast<T>(static_cast<double>(value) / sensor.scale);
  return pushBytes(scaledValue, sensor);
}

/// @brief Float data
/// @param sensor
/// @param value
/// @return
bool BaseDevice::addFloat(BtHomeType sensor, float value)
{
  if (!hasEnoughSpace(sensor))
  {
    return false;
  }

  float factor = sensor.scale;
  float scaledValue = value / factor;
  return pushBytes(static_cast<uint64_t>(scaledValue), sensor);
}

bool BaseDevice::pushBytes(uint64_t value2, BtHomeType sensor)
{
  _sensorData[_sensorDataIdx] = sensor.id;
  _sensorDataIdx++;
  for (uint8_t i = 0; i < sensor.byteCount; i++)
  {
    _sensorData[_sensorDataIdx] = static_cast<byte>((value2 >> (8 * i)) & 0xff);
    _sensorDataIdx++;
  }
  return true;
}

/// @brief TEXT and RAW data
/// @param sensor
/// @param value
/// @param size
/// @return
bool BaseDevice::addRaw(BtHomeType sensor, uint8_t *value, uint8_t size)
{
  if ((_sensorDataIdx + size + 1) > _maximumMeasurementBytes)
  // TODO: see if this can be moved to the hasEnoughSpace function
  {
    return false;
  }

  _sensorData[_sensorDataIdx] = sensor.id;
  _sensorDataIdx++;
  _sensorData[_sensorDataIdx] = static_cast<byte>(size & 0xff);
  _sensorDataIdx++;
  for (uint8_t i = 0; i < size; i++)
  {
    _sensorData[_sensorDataIdx] = static_cast<byte>(value[i] & 0xff);
    _sensorDataIdx++;
  }
  return true;
}

size_t BaseDevice::getAdvertisementData(uint8_t *buffer)
{
    /**
   * 02 01 06                             ← Flags
   * 0B (length) -- 09 (name indicator) -- 44 49 59 2D 73 65 6E 73 6F 72  ← Complete Local Name: “DIY-sensor”
   * 0A (length) -- 16 (payload indicator) -- D2 FC 40 02 C4 09 03 BF 13    ← BTHome Service Data
   *
   * Or another example:
   * 02 01 06                                     │ Flags
   * 05 08 44 49 59 2D                            │ Length=5, AD type=0x08, "DIY-"
   * 0B 09 44 49 59 2D 73 65 6E 73 6F 72          │ Length=11, AD type=0x09, "DIY-sensor"
   * 0A 16 D2 FC 40 ...                           │ BTHome service data
   */
  size_t idx = 0; // Tracks the next write position in buffer

  // 1. Flags
  buffer[idx++] = FLAG1;
  buffer[idx++] = FLAG2;
  buffer[idx++] = FLAG3;

  // 2. Name (Complete or Short)
  uint8_t completeNameLength = strlen(_completeName);
  uint8_t shortNameLength = strlen(_shortName);

  bool canFitLongName = (completeNameLength - shortNameLength) + _sensorDataIdx <= _maximumMeasurementBytes;

  if (canFitLongName) {
    buffer[idx++] = completeNameLength + 1; // Length of this AD field
    buffer[idx++] = COMPLETE_NAME;          // 0x09 for complete name
    memcpy(&buffer[idx], _completeName, completeNameLength);
    idx += completeNameLength;
  } else {
    buffer[idx++] = shortNameLength + 1;
    buffer[idx++] = SHORT_NAME;             // 0x08 for short name
    memcpy(&buffer[idx], _shortName, shortNameLength);
    idx += shortNameLength;
  }

  // 3. Service Data
  size_t serviceDataStart = idx; // Remember where service data starts
  buffer[idx++] = 3 + 1 + _sensorDataIdx; // Service Data length (UUID(2) + header(1) + sensorData)
  buffer[idx++] = SERVICE_DATA;            // 0x16
  buffer[idx++] = UUID1;                   // 0xD2
  buffer[idx++] = UUID2;                   // 0xFC

  // Header (no encryption vs trigger based)
  if (_triggerDevice)
    buffer[idx++] = NO_ENCRYPT_TRIGGER_BASE;
  else
    buffer[idx++] = NO_ENCRYPT;

  // Sensor Data
  for (size_t i = 0; i < _sensorDataIdx; i++)
    buffer[idx++] = _sensorData[i];

  // (No need to separately "add length" at the end; done above)

  // Optionally: print total length for debugging
  Serial.print("Advertisement total length: ");
  Serial.println(idx);

  return idx; // Number of bytes written to buffer
}

