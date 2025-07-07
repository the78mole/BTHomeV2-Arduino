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

  resetMeasurement();
}

BaseDevice::BaseDevice(const char *shortName, const char *completeName, bool isTriggerBased,
                       uint8_t encryptionKey[ENCRYPTION_KEY_LENGTH], const uint8_t macAddress[BLE_MAC_ADDRESS_LENGTH])
    : BaseDevice(shortName, completeName, isTriggerBased)
{
  _useEncryption = true;

  memcpy(_macAddress, macAddress, BLE_MAC_ADDRESS_LENGTH);
  mbedtls_ccm_init(&this->_encryptCTX);
  mbedtls_ccm_setkey(&this->_encryptCTX, MBEDTLS_CIPHER_ID_AES, encryptionKey, ENCRYPTION_KEY_LENGTH * 8);
}

/// @brief Clear the measurement data.
void BaseDevice::resetMeasurement()
{
  _sensorDataIdx = 0;
  memset(_sensorData, 0, MAX_MEASUREMENT_SIZE);
}

/// @brief Check that there is enough space in the sensor data packet for the given size.
/// @details The sensor data packet has a maximum length defined by MEASUREMENT_MAX_LEN.
/// @param size
/// @return Returns true if there is enough space for the given size, false otherwise.
bool BaseDevice::hasEnoughSpace(BtHomeState sensor)
{
  return hasEnoughSpace(sensor.byteCount + TYPE_INDICATOR_SIZE);
}

bool BaseDevice::hasEnoughSpace(uint8_t size)
{
  // the index is at the next entry point, so there is one byte extra
  static const uint8_t CURRENT_BYTE = 1;
  static const uint8_t ENCRYPTION_ADDITIONAL_BYTES = 8;
  int remainingBytes = (MAX_MEASUREMENT_SIZE - _sensorDataIdx) + CURRENT_BYTE - (_useEncryption ? ENCRYPTION_ADDITIONAL_BYTES: 0);
  return remainingBytes >= size;
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
    return true;
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
  static const size_t RAW_HEADER_BYTE_SIZE = 2;

  if (!hasEnoughSpace(size + RAW_HEADER_BYTE_SIZE))
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

size_t BaseDevice::getAdvertisementData(uint8_t buffer[MAX_ADVERTISEMENT_SIZE])
{
  size_t idx = 0; // Tracks the next write position in buffer

  // 1. Flags
  buffer[idx++] = FLAG1;
  buffer[idx++] = FLAG2;
  buffer[idx++] = FLAG3;

  // 3. Service Data
  static const uint8_t UUID_AND_HEADER_LENGTH = 3;
  buffer[idx++] = UUID_AND_HEADER_LENGTH + TYPE_INDICATOR_SIZE + _sensorDataIdx; // Packet length =  Service Data length (UUID(2) + header(1) + sensorData)
  buffer[idx++] = SERVICE_DATA;                                                  // 0x16
  buffer[idx++] = UUID1;                                                         // 0xD2
  buffer[idx++] = UUID2;                                                         // 0xFC

  uint8_t indicatorByte = FLAG_VERSION;

  if (_useEncryption)
  {
    indicatorByte |= FLAG_ENCRYPT;
  }

  if (_triggerDevice)
  {
    indicatorByte |= FLAG_TRIGGER;
  }

  buffer[idx++] = indicatorByte;

  if (_useEncryption)
  {
    uint8_t ciphertext[MAX_ADVERTISEMENT_SIZE];
    uint8_t encryptionTag[MIC_LEN];
    uint8_t nonce[NONCE_LEN];
    uint8_t *countPtr = (uint8_t *)(&this->_encryptCount);

    nonce[0] = _macAddress[5];
    nonce[1] = _macAddress[4];
    nonce[2] = _macAddress[3];
    nonce[3] = _macAddress[2];
    nonce[4] = _macAddress[1];
    nonce[5] = _macAddress[0];
    nonce[6] = UUID1;
    nonce[7] = UUID2;
    nonce[8] = FLAG_VERSION | FLAG_ENCRYPT; // TODO: should this be the same as the indicator byte?
    memcpy(&nonce[9], countPtr, 4);

    // encrypt sensorData
    mbedtls_ccm_encrypt_and_tag(&this->_encryptCTX, this->_sensorDataIdx, nonce, NONCE_LEN, 0, 0,
                                &this->_sensorData[0], &ciphertext[0], encryptionTag,
                                MIC_LEN);
    for (uint8_t i = 0; i < _sensorDataIdx; i++)
    {
      buffer[idx++] = ciphertext[i];
    }
    // writeCounter
    buffer[idx++] = nonce[9];
    buffer[idx++] = nonce[10];
    buffer[idx++] = nonce[11];
    buffer[idx++] = nonce[12];
    this->_encryptCount++;
    // writeMIC
    buffer[idx++] = encryptionTag[0];
    buffer[idx++] = encryptionTag[1];
    buffer[idx++] = encryptionTag[2];
    buffer[idx++] = encryptionTag[3];
  }
  else
  {
    // Sensor Data
    for (size_t i = 0; i < _sensorDataIdx; i++)
      buffer[idx++] = _sensorData[i];
  }
  
  // prefer long name
  size_t completeNameLength = strnlen(_completeName, MAX_LENGTH_COMPLETE_NAME);
  bool canFitLongName = idx + TYPE_INDICATOR_SIZE + completeNameLength + 1 <= MAX_ADVERTISEMENT_SIZE;
  if (canFitLongName)
  {
    buffer[idx++] = completeNameLength + TYPE_INDICATOR_SIZE;
    buffer[idx++] = COMPLETE_NAME;
    memcpy(&buffer[idx], _completeName, completeNameLength);
    idx += completeNameLength;
  }

  size_t shortNameLength = strnlen(_shortName, MAX_LENGTH_SHORT_NAME);
  bool canFitShortName = idx + TYPE_INDICATOR_SIZE + shortNameLength + 1 <= MAX_ADVERTISEMENT_SIZE;
  if (canFitShortName)
  {
    buffer[idx++] = shortNameLength + TYPE_INDICATOR_SIZE;
    buffer[idx++] = SHORT_NAME; // 0x08 for short name
    memcpy(&buffer[idx], _shortName, shortNameLength);
    idx += shortNameLength;
  }

#ifdef BTHOME_DEBUG
  Serial.print("Advertisement: ");
  for (size_t i = 0; i < idx; i++)
  {
    if (buffer[i] < 16)
      Serial.print('0');
    Serial.print(buffer[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.print(" (size: ");
  Serial.print(idx);
  Serial.println(" bytes)");
#endif

  return idx; // Number of bytes written to buffer
}
