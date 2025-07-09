/*
Example of using a DHT11 to measure humidity and temperature on a FireBeetle2 ESP32 C6
*/
#include <BtHomeV2Device.h>
#include <ArduinoBLE.h>
#include <DHT11.h>
#define SLEEP_DURATION_SECONDS 30
#define BATTERY_MEASUREMENT_PIN 0
#define DHT_DATA_PIN A4
#define LED_PIN 8

RTC_DATA_ATTR uint64_t counter = 0;


void setup() {
  Serial.begin(115200);
  
  // turn on LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  if (!BLE.begin()) {
    // If BLE doesn't start, then just go to sleep
    Serial.println("Failed to initialize BLE!");
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SECONDS * 1000000);
    esp_deep_sleep_start();
  }

  // set up the buffer for the advertisement data
  uint8_t advertisementData[MAX_ADVERTISEMENT_SIZE];
  uint8_t size = 0;

  // first advertisement with battery percentage
  BtHomeV2Device device("SUPER", "SUPER", false);
  device.addCount_0_4294967295(counter++);
  size = device.getAdvertisementData(advertisementData);
  sendAdvertisement(advertisementData, size);
  device.clearMeasurementData(); // clear the buffer before the next advertisement

  DHT11 dht11(DHT_DATA_PIN); // Initialise the DHT code

  int temperature = 0;
  int humidity = 0;

  int result = dht11.readTemperatureHumidity(temperature, humidity);

  if (result == 0) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\tHumidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println(DHT11::getErrorString(result));
  }


  device.addTemperature_neg327_to_327_Resolution_0_01(temperature);
  device.addHumidityPercent_Resolution_1(humidity);

  size = device.getAdvertisementData(advertisementData);
  sendAdvertisement(advertisementData, size);

  // turn off BLE
  BLE.end();

  // turn off LED
  digitalWrite(LED_PIN, LOW);

  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SECONDS * 1000000);
  esp_deep_sleep_start();
}

void sendAdvertisement(uint8_t advertisementData[], size_t size) {
  BLEAdvertisingData advData;
  advData.setRawData(advertisementData, size);
  BLE.setAdvertisingData(advData);
  BLE.advertise();
  Serial.println("Raw advertising started!");
  delay(1000);
  BLE.stopAdvertise();
  Serial.println("Raw advertising ended!");
}

void loop() {
}
