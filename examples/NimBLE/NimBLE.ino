#include <BtHomeV2Device.h>
#include "NimBLEDevice.h"

void setup()
{
  Serial.begin(115200);
  delay(50);
  Serial.println("Starting BTHomeV2-Arduino Example...");
}
bool buttonOn = false;

void sendAdvertisement(uint8_t advertisementData[], size_t size)
{
  NimBLEDevice::init("");
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData pAdvData = BLEAdvertisementData();
  std::vector<uint8_t> data(advertisementData, advertisementData + size);

  pAdvData.addData(data);
  pAdvertising->setAdvertisementData(pAdvData);
  pAdvertising->setConnectableMode(0);
  Serial.println("Starting advertising...");
  pAdvertising->start();
  delay(1000);
  pAdvertising->stop();
  Serial.println("Stopping advertising...");
}

void loop()
{

  // Short or long name sent based on packet size
  BtHomeV2Device btHome("short_name", "My longer device name", false);

  // Add humidity
  btHome.addHumidityPercent_Resolution_0_01(50.55f);

  // Add Temperature
  btHome.addTemperature_neg127_to_127_Resolution_1(-22.0f);

  // Set Battery State
  btHome.setBatteryState(BATTERY_STATE_LOW);

  // Set battery percentage
  btHome.addBatteryPercentage(22);

  uint8_t advertisementData[MAX_PAYLOAD_SIZE];
  size_t size = btHome.getAdvertisementData(advertisementData);

  // Set the advertisement data on BLE
  sendAdvertisement(advertisementData, size); // <- this method isn't part of the library. Look at the examples
  Serial.println("Advertising data sent.");

  delay(5000);
}
