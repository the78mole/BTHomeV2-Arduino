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
                       uint8_t const *const key, const uint8_t macAddress[BLE_MAC_ADDRESS_LENGTH])
    : BaseDevice(shortName, completeName, isTriggerBased)
{
  _useEncryption = true;

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

/*PLAIN SENSOR DATA: 10 01 10 01
NONCE: E0 5A 1B 6C 2E 72 D2 FC 41 01 00 00 00
KEY: 23 1D 39 C1 D7 CC 1A B1 AE E2 24 CD 09 6D B9 32
MAC: 72 2E 6C 1B 5A E0
MAC (reversed for nonce): E0 5A 1B 6C 2E 72
COUNTER: 1 01 00 00 00
CIPHERTEXT: CF 24 4F 9F
ENCRYPTION TAG: B4 30 0F B4
FULL ADVERTISEMENT: 02 01 06 10 16 D2 FC 41 CF 24 4F 9F 01 00 00 00 B4 30 0F B4 */

size_t BaseDevice::getAdvertisementData(uint8_t buffer[MAX_ADVERTISEMENT_SIZE])
{
  std::string payloadData = "";
  std::string serviceData = "";
  uint8_t i;

  // head
  payloadData += FLAG1;
  payloadData += FLAG2;
  payloadData += FLAG3;

  serviceData += SERVICE_DATA; // DO NOT CHANGE -- Service Data - 16-bit UUID
  serviceData += UUID1;        // DO NOT CHANGE -- UUID
  serviceData += UUID2;        // DO NOT CHANGE -- UUID
  // The encryption
  if (_useEncryption)
  {
    if (_triggerDevice)
      serviceData += ENCRYPT_TRIGGER_BASE;
    else
      serviceData += ENCRYPT;

    uint8_t ciphertext[MAX_ADVERTISEMENT_SIZE];
    uint8_t encryptionTag[MIC_LEN];
    uint8_t nonce[NONCE_LEN];
    uint8_t *countPtr = (uint8_t *)(&this->_counter);
    const uint8_t *addrs = _macAddress;

    nonce[0] = addrs[5];
    nonce[1] = addrs[4];
    nonce[2] = addrs[3];
    nonce[3] = addrs[2];
    nonce[4] = addrs[1];
    nonce[5] = addrs[0];
    nonce[6] = UUID1;
    nonce[7] = UUID2;
    nonce[8] = ENCRYPT;
    memcpy(&nonce[9], countPtr, 4);

    // ======= PRINT THE BUFFERS =======
    Serial.print("PLAIN SENSOR DATA: ");
    for (uint8_t k = 0; k < _sensorDataIdx; k++)
    {
      if (_sensorData[k] < 16)
        Serial.print('0');
      Serial.print(_sensorData[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    Serial.print("NONCE: ");
    for (uint8_t k = 0; k < NONCE_LEN; k++)
    {
      if (nonce[k] < 16)
        Serial.print('0');
      Serial.print(nonce[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    Serial.print("KEY: ");
    for (uint8_t k = 0; k < BIND_KEY_LEN; k++)
    {
      if (bindKey[k] < 16)
        Serial.print('0');
      Serial.print(bindKey[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    // ==================================

    // Print MAC (in normal and reversed order)
    Serial.print("MAC: ");
    for (int k = 0; k < 6; k++)
    {
      if (addrs[k] < 16)
        Serial.print('0');
      Serial.print(addrs[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    Serial.print("MAC (reversed for nonce): ");
    for (int k = 5; k >= 0; k--)
    {
      if (addrs[k] < 16)
        Serial.print('0');
      Serial.print(addrs[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    // Print counter value as bytes
    Serial.print("COUNTER: ");

    Serial.print(this->_counter);
    Serial.print(" ");

    for (uint8_t k = 0; k < 4; k++)
    {
      uint8_t b = countPtr[k];
      if (b < 16)
        Serial.print('0');
      Serial.print(b, HEX);
      Serial.print(' ');
    }
    Serial.println();

    mbedtls_ccm_encrypt_and_tag(&_encryptCTX, _sensorDataIdx, nonce, NONCE_LEN, 0, 0,
                                &_sensorData[0], &ciphertext[0], encryptionTag,
                                MIC_LEN);

    Serial.print("CIPHERTEXT: ");
    for (uint8_t k = 0; k < _sensorDataIdx; k++)
    {
      if (ciphertext[k] < 16)
        Serial.print('0');
      Serial.print(ciphertext[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    Serial.print("ENCRYPTION TAG: ");
    for (uint8_t k = 0; k < MIC_LEN; k++)
    {
      if (encryptionTag[k] < 16)
        Serial.print('0');
      Serial.print(encryptionTag[k], HEX);
      Serial.print(' ');
    }
    Serial.println();

    // ==================================

    for (i = 0; i < _sensorDataIdx; i++)
    {
      serviceData += ciphertext[i];
    }
    // writeCounter
    serviceData += nonce[9];
    serviceData += nonce[10];
    serviceData += nonce[11];
    serviceData += nonce[12];
    // this->_counter++;
    // writeMIC
    serviceData += encryptionTag[0];
    serviceData += encryptionTag[1];
    serviceData += encryptionTag[2];
    serviceData += encryptionTag[3];
  }
  else
  {
    if (_triggerDevice)
      serviceData += NO_ENCRYPT_TRIGGER_BASE;
    else
      serviceData += NO_ENCRYPT;
    for (i = 0; i < _sensorDataIdx; i++)
    {
      serviceData += _sensorData[i]; // Add the sensor data to the Service Data
    }
  }

  byte sd_length = serviceData.length(); // Generate the length of the Service Data
  payloadData += sd_length;              // Add the length of the Service Data
  payloadData += serviceData;            // Finalize the packet

  // ----- Copy payloadData into buffer -----
  size_t payloadLen = payloadData.length();
  for (size_t j = 0; j < payloadLen; j++)
  {
    buffer[j] = payloadData[j];
  }
  return payloadLen;
}

void BaseDevice::writeEncryptedPayload(uint8_t serviceDataBuffer[MAX_ADVERTISEMENT_SIZE], uint8_t *index)
{
}

// // prefer long name
// size_t completeNameLength = strnlen(_completeName, MAX_LENGTH_COMPLETE_NAME);
// bool canFitLongName = idx + TYPE_INDICATOR_SIZE + completeNameLength + 1 <= MAX_ADVERTISEMENT_SIZE;
// if (canFitLongName)
// {
//   buffer[idx++] = completeNameLength + TYPE_INDICATOR_SIZE;
//   buffer[idx++] = COMPLETE_NAME;
//   memcpy(&buffer[idx], _completeName, completeNameLength);
//   idx += completeNameLength;
// }

// size_t shortNameLength = strnlen(_shortName, MAX_LENGTH_SHORT_NAME);
// bool canFitShortName = idx + TYPE_INDICATOR_SIZE + shortNameLength + 1 <= MAX_ADVERTISEMENT_SIZE;
// if (canFitShortName)
// {
//   buffer[idx++] = shortNameLength + TYPE_INDICATOR_SIZE;
//   buffer[idx++] = SHORT_NAME; // 0x08 for short name
//   memcpy(&buffer[idx], _shortName, shortNameLength);
//   idx += shortNameLength;
// }
