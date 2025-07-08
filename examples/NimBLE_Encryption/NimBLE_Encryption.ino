#include <Arduino.h>
#include <BtHomeV2Device.h>
#include "NimBLEDevice.h"

// Bind key: aab3147d9822c05fe14a0c3b77d68e55
const uint8_t key[16] = {
  0xAA, 0xB3, 0x14, 0x7D, 0x98, 0x22, 0xC0, 0x5F,
  0xE1, 0x4A, 0x0C, 0x3B, 0x77, 0xD6, 0x8E, 0x55
};

uint8_t _macAddress[6];
BtHomeV2Device *btHome = nullptr;
float _metres = 1.5f;

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("Starting BTHomeV2-Arduino Encryption Example...");

  NimBLEDevice::init("");
  const uint8_t* mac = NimBLEDevice::getAddress().getVal();
  memcpy(_macAddress, mac, 6);

  Serial.print("Device MAC: ");
  for (int i = 0; i < 6; i++) {
    if (i) Serial.print(":");
    Serial.print(_macAddress[i], HEX);
  }
  Serial.println();

  btHome = new BtHomeV2Device("short_name", "My longer device name", false, key, _macAddress);
}

void sendAdvertisement(uint8_t advertisementData[], size_t size) {
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

void loop() {
  btHome->clearMeasurementData();
  btHome->addDistanceMetres(_metres++);

  uint8_t advertisementData[MAX_ADVERTISEMENT_SIZE];
  size_t size = btHome->getAdvertisementData(advertisementData);

  sendAdvertisement(advertisementData, size);
  Serial.println("Advertising data sent.");

  delay(5000); 
}
