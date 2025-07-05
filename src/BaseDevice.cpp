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
bool BaseDevice::hasEnoughSpace(BtHomeState sensor)
{
  // minimum space is needed for the short name, but if there is spare room then we can use the full name
  int remainingBytes = MAX_MEASUREMENT_SIZE - _sensorDataIdx;
  bool result = (remainingBytes >= sensor.byteCount + 1); // include sensor indicator byte
  return result;
}

/// @brief Add a state or step value to the sensor data packet.
/// @param sensor
/// @param state
/// @param steps
/// @return
bool BaseDevice::addState(BtHomeState sensor, uint8_t state)
{
  if (!hasEnoughSpace(sensor))
  {
    return false;
  }

  _sensorData[_sensorDataIdx++] = sensor.id;
  _sensorData[_sensorDataIdx++] = state;
  return true;
}

/// @brief Add a state or step value to the sensor data packet.
/// @param sensor
/// @param state
/// @param steps
/// @return
bool BaseDevice::addState(BtHomeState sensor, uint8_t state, uint8_t steps)
{
  if (addState(sensor, state))
  {
    _sensorData[_sensorDataIdx++] = steps;
  }
  return false;
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
bool BaseDevice::addRaw(uint8_t sensorId, uint8_t *value, uint8_t size)
{
  if ((_sensorDataIdx + size + 1) > MAX_MEASUREMENT_SIZE)
  {
    return false;
  }

  _sensorData[_sensorDataIdx++] = sensorId;
  _sensorData[_sensorDataIdx++] = static_cast<byte>(size & 0xff);
  for (uint8_t i = 0; i < size; i++)
  {
    _sensorData[_sensorDataIdx++] = static_cast<byte>(value[i] & 0xff);
  }
  return true;
}

size_t BaseDevice::getAdvertisementData(uint8_t *buffer)
{
  size_t idx = 0; // Tracks the next write position in buffer

  // 1. Flags
  buffer[idx++] = FLAG1;
  buffer[idx++] = FLAG2;
  buffer[idx++] = FLAG3;


  // prefer long name
  uint8_t completeNameLength = strnlen(_completeName, MAX_LENGTH_COMPLETE_NAME);
  bool canFitLongName = idx + completeNameLength + TYPE_INDICATOR_SIZE <= MAX_ADVERTISEMENT_SIZE;
  if (canFitLongName)
  {
    buffer[idx++] = completeNameLength + TYPE_INDICATOR_SIZE; // Length of this AD field
    buffer[idx++] = COMPLETE_NAME;          // 0x09 for complete name
    memcpy(&buffer[idx], _completeName, completeNameLength);
    idx += completeNameLength;
  }

  uint8_t shortNameLength = strnlen(_shortName, MAX_LENGTH_SHORT_NAME);
  bool canFitShortName =  idx + shortNameLength + TYPE_INDICATOR_SIZE <= MAX_ADVERTISEMENT_SIZE;
  if (canFitShortName)
  {
    buffer[idx++] = shortNameLength + TYPE_INDICATOR_SIZE;
    buffer[idx++] = SHORT_NAME; // 0x08 for short name
    memcpy(&buffer[idx], _shortName, shortNameLength);
    idx += shortNameLength;
  }

  // 3. Service Data
  size_t serviceDataStart = idx;          // Remember where service data starts
  buffer[idx++] = 3 + TYPE_INDICATOR_SIZE + _sensorDataIdx; // Packet length =  Service Data length (UUID(2) + header(1) + sensorData)
  buffer[idx++] = SERVICE_DATA;           // 0x16
  buffer[idx++] = UUID1;                  // 0xD2
  buffer[idx++] = UUID2;                  // 0xFC

  // Header (no encryption vs trigger based)
  if (_triggerDevice)
    buffer[idx++] = NO_ENCRYPT_TRIGGER_BASE;
  else
    buffer[idx++] = NO_ENCRYPT;

  // Sensor Data
  for (size_t i = 0; i < _sensorDataIdx; i++)
    buffer[idx++] = _sensorData[i];

  return idx; // Number of bytes written to buffer
}
