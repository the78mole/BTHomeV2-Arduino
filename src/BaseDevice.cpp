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
                       uint8_t const *const key, const uint8_t macAddress[BLE_MAC_ADDRESS_LENGTH], uint32_t counter)
    : BaseDevice(shortName, completeName, isTriggerBased)
{
  _useEncryption = true;
  _counter = counter;
  
  memcpy(bindKey, key, sizeof(uint8_t) * BIND_KEY_LEN);
  // Print the encryption key as a string for BTHome (hex, no spaces, lowercase)
  Serial.print("Encryption Key (BTHome format): ");
  for (size_t i = 0; i < ENCRYPTION_KEY_LENGTH; i++)
  {
    if (bindKey[i] < 0x10)
      Serial.print('0');
    Serial.print(bindKey[i], HEX);
  }

  Serial.println();
  memcpy(_macAddress, macAddress, BLE_MAC_ADDRESS_LENGTH);

  Serial.println("Setting up the keys");
  delay(50);

  mbedtls_ccm_init(&this->_encryptCTX);
  mbedtls_ccm_setkey(&this->_encryptCTX, MBEDTLS_CIPHER_ID_AES, bindKey, ENCRYPTION_KEY_LENGTH * 8);

  Serial.println("Keys set up");
  delay(50);
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

  int remainingBytes = (MAX_MEASUREMENT_SIZE - _sensorDataIdx) + CURRENT_BYTE - (_useEncryption ? ENCRYPTION_ADDITIONAL_BYTES : 0);
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

  return pushBytes(state, sensor);

}

/// @brief Add a state or step value to the sensor data packet.
/// @param sensor
/// @param state
/// @param steps
/// @return
bool BaseDevice::addState(BtHomeState sensor, uint8_t state, uint8_t steps)
{
    if (!hasEnoughSpace(sensor))
  {
    return false;
  }

  uint16_t stepState = ((uint16_t)steps <<8 | state);
  return pushBytes(stepState, sensor);
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

bool BaseDevice::pushBytes(uint64_t value2, BtHomeState sensor)
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
  char serviceData[MAX_ADVERTISEMENT_SIZE];
  uint8_t serviceDataIndex = 0;

  serviceData[serviceDataIndex++] = SERVICE_DATA; // DO NOT CHANGE -- Service Data - 16-bit UUID
  serviceData[serviceDataIndex++] = UUID1;        // DO NOT CHANGE -- UUID
  serviceData[serviceDataIndex++] = UUID2;        // DO NOT CHANGE -- UUID

  char indicatorByte = FLAG_VERSION;

  if (_triggerDevice)
  {
    indicatorByte |= FLAG_TRIGGER;
  }

  if (_useEncryption)
  {
    indicatorByte |= FLAG_ENCRYPT;
  }

  serviceData[serviceDataIndex++] = indicatorByte;

  // The encryption
  if (_useEncryption)
  {

    uint8_t ciphertext[MAX_ADVERTISEMENT_SIZE];
    uint8_t encryptionTag[MIC_LEN];
    uint8_t nonce[NONCE_LEN];
    uint8_t *countPtr = (uint8_t *)(&this->_counter);

    nonce[0] = _macAddress[5];
    nonce[1] = _macAddress[4];
    nonce[2] = _macAddress[3];
    nonce[3] = _macAddress[2];
    nonce[4] = _macAddress[1];
    nonce[5] = _macAddress[0];
    nonce[6] = UUID1;
    nonce[7] = UUID2;
    nonce[8] = FLAG_VERSION | FLAG_ENCRYPT;
    memcpy(&nonce[9], countPtr, 4);

    mbedtls_ccm_encrypt_and_tag(&_encryptCTX, _sensorDataIdx, nonce, NONCE_LEN, 0, 0,
                                &_sensorData[0], &ciphertext[0], encryptionTag,
                                MIC_LEN);

    for (uint8_t i = 0; i < _sensorDataIdx; i++)
    {
      serviceData[serviceDataIndex++] = ciphertext[i];
    }
    // writeCounter
    serviceData[serviceDataIndex++] = nonce[9];
    serviceData[serviceDataIndex++] = nonce[10];
    serviceData[serviceDataIndex++] = nonce[11];
    serviceData[serviceDataIndex++] = nonce[12];
    this->_counter++;
    // writeMIC
    serviceData[serviceDataIndex++] = encryptionTag[0];
    serviceData[serviceDataIndex++] = encryptionTag[1];
    serviceData[serviceDataIndex++] = encryptionTag[2];
    serviceData[serviceDataIndex++] = encryptionTag[3];
  }
  else
  {
    for (uint8_t i = 0; i < _sensorDataIdx; i++)
    {
      serviceData[serviceDataIndex++] = _sensorData[i]; // Add the sensor data to the Service Data
    }
  }

  uint8_t bufferDataIndex = 0;
  // head
  buffer[bufferDataIndex++] = FLAG1;
  buffer[bufferDataIndex++] = FLAG2;
  buffer[bufferDataIndex++] = FLAG3;
  byte sd_length = serviceDataIndex;     // Generate the length of the Service Data
  buffer[bufferDataIndex++] = sd_length; // Add the length of the Service Data

  for (size_t i = 0; i < serviceDataIndex; i++)
  {
    buffer[bufferDataIndex++] = serviceData[i];
    /* code */
  }

#define CURRENT_BYTE 1

  // prefer long name
  size_t completeNameLength = strnlen(_completeName, MAX_LENGTH_COMPLETE_NAME);
  bool canFitLongName = bufferDataIndex + completeNameLength + TYPE_INDICATOR_SIZE + CURRENT_BYTE <= MAX_ADVERTISEMENT_SIZE;
  if (canFitLongName)
  {
    buffer[bufferDataIndex++] = completeNameLength + TYPE_INDICATOR_SIZE;
    buffer[bufferDataIndex++] = COMPLETE_NAME;
    memcpy(&buffer[bufferDataIndex], _completeName, completeNameLength);
    bufferDataIndex += completeNameLength;
  }

  size_t shortNameLength = strnlen(_shortName, MAX_LENGTH_SHORT_NAME);
  bool canFitShortName = bufferDataIndex + TYPE_INDICATOR_SIZE + shortNameLength + CURRENT_BYTE <= MAX_ADVERTISEMENT_SIZE;
  if (canFitShortName)
  {
    buffer[bufferDataIndex++] = shortNameLength + TYPE_INDICATOR_SIZE;
    buffer[bufferDataIndex++] = SHORT_NAME; // 0x08 for short name
    memcpy(&buffer[bufferDataIndex], _shortName, shortNameLength);
    bufferDataIndex += shortNameLength;
  }
  return bufferDataIndex;
}

void BaseDevice::writeEncryptedPayload(uint8_t serviceDataBuffer[MAX_ADVERTISEMENT_SIZE], uint8_t *index)
{
}
