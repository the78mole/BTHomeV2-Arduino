/*
Example of using a DHT11 to measure humidity and temperature on a FireBeetle2 ESP32 C6
*/
#include <BtHomeV2Device.h>
#include <ArduinoBLE.h>
#include <DHT11.h>
#define SLEEP_DURATION_SECONDS 180
#define BATTERY_MEASUREMENT_PIN 0
#define DHT_POWER_PIN 19
#define DHT_DATA_PIN 20

const float V_CUTOFF = 3000;  // 0% batteryPercentage
const float V_FULL = 4200;    // 100% batteryPercentage

/**  This code should work with most devices.
FireBeetle 2 Notes
- Add the boards to the IDE - refer to https://wiki.dfrobot.com/SKU_DFR1075_FireBeetle_2_Board_ESP32_C6
- Enable the USB CDC build flag under the tools menu before uploading
- Hold the boot button and press reset to get into bootloader mode.
- If the usb keeps connecting and then disconnecting, put it in bootloader mode.
 */
RTC_DATA_ATTR uint64_t counter = 0;

void publishSensorReading(int analogMillivolts) {

  // Fixed battery percentage calculation
  float batteryPercentage = ((analogMillivolts - V_CUTOFF) * 100.0) / (V_FULL - V_CUTOFF);
  batteryPercentage = constrain(batteryPercentage, 0.0, 100.0);

  Serial.print("Battery percentage: ");
  Serial.print(batteryPercentage, 1);
  Serial.println("%");

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
  BtHomeV2Device device("BEETLE", "Firebeetle Sensor", false);
  device.addBatteryPercentage(batteryPercentage);
  float volts = analogMillivolts / 1000.0f;
  device.addVoltage_0_to_65_resolution_0_001(volts);

  // DURING DEVELOPMENT -> check if the last entry can fit
  if (!device.addCount_0_4294967295(counter++)) {
    Serial.println("Failed to add count");
  }

  size = device.getAdvertisementData(advertisementData);
  sendBluetoothAdvertisement(advertisementData, size);
  device.clearMeasurementData();  // clear the buffer before the next advertisement

  DHT11 dht11(DHT_DATA_PIN);  // Initialise the DHT code

  int temperature = 0;
  int humidity = 0;

  int result = dht11.readTemperatureHumidity(temperature, humidity);

  // power off sensor
  digitalWrite(DHT_POWER_PIN, LOW);

  if (result == 0) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\tHumidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    device.setRunningState(Running_Sensor_Status_Running);  // Running == DHT working
    device.addTemperature_neg327_to_327_Resolution_0_01(temperature);
    device.addHumidityPercent_Resolution_1(humidity);
  } else {
    Serial.println(DHT11::getErrorString(result));
    device.setRunningState(Running_Sensor_Status_Not_Running);
  }

  size = device.getAdvertisementData(advertisementData);
  sendBluetoothAdvertisement(advertisementData, size);
}


void setup() {
  Serial.begin(115200);

  // turn on sensor as it needs time to wake up
  pinMode(DHT_POWER_PIN, OUTPUT);
  digitalWrite(DHT_POWER_PIN, HIGH);

  // turn on LED
  pinMode(D13, OUTPUT);

  digitalWrite(D13, HIGH);

  analogReadResolution(12);
  int analogMillivolts = analogReadMilliVolts(BATTERY_MEASUREMENT_PIN) * 2;

  delay(50);
  Serial.print("BAT millivolts value = ");
  Serial.print(analogMillivolts);
  Serial.println("mV");


  if (analogMillivolts < 3300) {
    Serial.print("BAT millivolts too low! Skipping advertisement ");
    Serial.print(analogMillivolts);
  } else {
    publishSensorReading(analogMillivolts);
  }
  
  // turn off LED
  digitalWrite(D13, LOW);

  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SECONDS * 1000000);
  esp_deep_sleep_start();
}

void sendBluetoothAdvertisement(uint8_t advertisementData[], size_t size) {
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
  Serial.println("Should not see this!");
}
