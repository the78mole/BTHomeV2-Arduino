#include "definitions.h"
#include <Arduino.h>
#include <data_types.h>
static const size_t MAX_PAYLOAD_SIZE = 31;


class BaseDevice {
  public:
    BaseDevice( const char* shortName,  const char* completeName, bool isTriggerBased);
    size_t getAdvertisementData(uint8_t *buffer);
    void resetMeasurement();
    bool addState(BtHomeState,  uint8_t state);
    bool addState(BtHomeState sensor, uint8_t state, uint8_t steps);
    bool addUnsignedInteger(BtHomeType sensor, uint64_t value);
    bool addSignedInteger(BtHomeType sensor, int64_t value);
    bool addFloat(BtHomeType sensor, float value);
    bool addRaw(uint8_t sensor, uint8_t *value, uint8_t size);

  private:
    bool pushBytes(uint64_t value2, BtHomeType sensor);
    uint8_t _maximumMeasurementBytes;
    uint8_t _sensorDataIdx = 0;    
    byte _sensorData[BLE_ADVERT_MAX_LEN] = {0};
    char _shortName[MAX_LENGTH_SHORT_NAME];
    char _completeName[MAX_LENGTH_COMPLETE_NAME];
    bool hasEnoughSpace(BtHomeState sensor);
    template <typename T>
    bool addInteger(BtHomeType sensor, T value);
    bool _triggerDevice;
};
