#include <BtHomeV2Device.h>
#include <ArduinoBLE.h>
#define SLEEP_DURATION_SECONDS 10
#define BATTERY_MEASUREMENT_PIN 0

const float V_CUTOFF = 3000; // 0% SoC
const float V_FULL = 4200;   // 100% SoC

/**  This code should work with most devices.
FireBeetle 2 Notes
- Add the boards to the IDE - refer to https://wiki.dfrobot.com/SKU_DFR1075_FireBeetle_2_Board_ESP32_C6
- Enable the USB CDC build flag under the tools menu before uploading
- Hold the boot button and press reset to get into bootloader mode.
- If the usb keeps connecting and then disconnecting, put it in bootloader mode.
 */
RTC_DATA_ATTR uint64_t counter = 0;

void setup()
{
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // turn on LED
  pinMode(D13, OUTPUT);
  digitalWrite(D13, HIGH);

  // set the resolution to 12 bits (0-4096)
  //  read the analog / millivolts value for pin 0:
  analogReadResolution(12);
  int analogValue = analogRead(BATTERY_MEASUREMENT_PIN);
  int analogMillivolts = analogReadMilliVolts(BATTERY_MEASUREMENT_PIN) * 2;

  delay(50);
  // print out the values you read:
  Serial.print("ADC analog value = ");
  Serial.println(analogValue);
  Serial.print("ADC millivolts value = ");
  Serial.print(analogMillivolts);
  Serial.println("mV");
  // Please adjust the calculation coefficient according to the actual measurement.
  Serial.print("BAT millivolts value = ");
  Serial.print(analogMillivolts);
  Serial.println("mV");
  Serial.println("--------------");

  float soc = (analogMillivolts - V_CUTOFF) / (V_FULL - V_CUTOFF) * 100.0;

  // Clamp between 0 and 100
  if (soc < 0.0)
    soc = 0.0;
  if (soc > 100.0)
    soc = 100.0;

  if (!BLE.begin())
  {
    // If BLE doesn't start, then just go to sleep
    Serial.println("Failed to initialize BLE!");
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SECONDS * 1000000);
    esp_deep_sleep_start();
  }

  uint8_t advertisementData[MAX_ADVERTISEMENT_SIZE];
  uint8_t size = 0;

  // first advertisement with battery percentage
  BtHomeV2Device device("BEETLE", "My Firebeetle V2", false);
  device.addBatteryPercentage(soc);
  size = device.getAdvertisementData(advertisementData);
  sendAdvertisement(advertisementData, size);

  // second advertisement with the voltage
  float volts = analogMillivolts / 1000;
  device.addVoltage_0_to_65_resolution_0_001(volts);
  device.addVoltage_0_to_6550_resolution_0_1(volts);
  device.addCount_0_4294967295(counter++);

  size = device.getAdvertisementData(advertisementData);
  sendAdvertisement(advertisementData, size);

  // turn off BLE
  BLE.end();

  // turn off LED
  digitalWrite(D13, LOW);

  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SECONDS * 1000000);
  esp_deep_sleep_start();
}

void sendAdvertisement(uint8_t advertisementData[], size_t size)
{
  BLEAdvertisingData advData;
  advData.setRawData(advertisementData, size);
  BLE.setAdvertisingData(advData);
  BLE.advertise();
  Serial.println("Raw advertising started!");
  delay(1000);
  BLE.stopAdvertise();
  Serial.println("Raw advertising ended!");
}

void loop()
{
}
