/*
Untested with Home Assistant as I don't have a BLE 5 receiver.
Verified through a BLE Scanner app. 

BLE5 Implementation with Long Range adjustments (low data rate)
You will need a BLE5 Capable Receiver for these messages.
It's also possible to extend the packet size to 1650 bytes, although recommended to keep 
below 255 bytes for compatibility with BLE scanners.

Increasing the TX power past 9dB does nothing as it's capped.

You can manually change MAX_ADVERTISEMENT_SIZE in the library if wanted.
*/


#include "esp_gap_ble_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_sleep.h"


#include <BtHomeV2Device.h>

#define ADV_DURATION 500       // 500 × 10ms = 5 seconds
#define SLEEP_DURATION_SEC 60  // Deep sleep for 60 seconds


bool isAdvertising = false;
volatile bool advStopped = false;

// ---- GAP callback handler ----
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  if (event == ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT) {
    advStopped = true;
  }
}

RTC_DATA_ATTR uint8_t counter = 0;
uint8_t advertisementData[MAX_ADVERTISEMENT_SIZE];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nWakeup. Starting BLE Long-Range Advertisement...");

  BtHomeV2Device device("LONG_RNG", "A long range example using BLE 5", false);

  device.addBatteryPercentage(77);
  device.addCount_0_4294967295(counter++);


  uint8_t size = device.getAdvertisementData(advertisementData);

  // ---- Initialize Bluetooth Controller ----
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
    Serial.println("Failed to init controller");
    return;
  }
  if (esp_bt_controller_enable(ESP_BT_MODE_BLE) != ESP_OK) {
    Serial.println("Failed to enable controller");
    return;
  }

  // ---- Initialize Bluedroid stack ----
  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to init bluedroid");
    return;
  }
  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return;
  }

  // ---- Register GAP callback ----
  if (esp_ble_gap_register_callback(gap_event_handler) != ESP_OK) {
    Serial.println("Failed to register GAP callback");
    return;
  }

  // ---- Set Max TX Power ----
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);  // P9 is the highest for ESP. Over that is capped.

  // ---- Extended Advertising Parameters ----
  esp_ble_gap_ext_adv_params_t adv_params = {
    .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_NONCONN_NONSCANNABLE_UNDIRECTED, 
    .interval_min = 0x0800,
    .interval_max = 0x0800,
    .channel_map = ADV_CHNL_ALL,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .primary_phy = ESP_BLE_GAP_PHY_CODED,
    .secondary_phy = ESP_BLE_GAP_PHY_CODED,
    .sid = 0,
    .scan_req_notif = false
  };

  if (esp_ble_gap_ext_adv_set_params(0, &adv_params) != ESP_OK) {
    Serial.println("Failed to set adv params");
    return;
  }

  if (esp_ble_gap_config_ext_adv_data_raw(0, size, advertisementData) != ESP_OK) {
    Serial.println("Failed to set adv data");
    return;
  }

  // ---- Start Extended Advertising ----
  esp_ble_gap_ext_adv_t start_params = {
    .instance = 0,
    .duration = ADV_DURATION,  // 5 seconds
    .max_events = 0
  };

  if (esp_ble_gap_ext_adv_start(1, &start_params) != ESP_OK) {
    Serial.println("Failed to start advertising");
    return;
  }

  Serial.println("Advertising for 5 seconds using Coded PHY...");
  isAdvertising = true;

  delay((ADV_DURATION * 10) + 500);  // Wait for advertising to complete

  shutdownBLE();  // Cleanly shut down BLE

  // ---- Enter Deep Sleep ----
  Serial.printf("Sleeping for %d seconds...\n", SLEEP_DURATION_SEC);
  esp_deep_sleep(SLEEP_DURATION_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

void shutdownBLE() {
  esp_err_t err;

  if (isAdvertising) {
    uint8_t adv_ids[1] = { 0 };  // Stop instance 0
    advStopped = false;          // Reset event flag

    err = esp_ble_gap_ext_adv_stop(1, adv_ids);
    if (err != ESP_OK) {
      Serial.printf("adv_stop error: %s\n", esp_err_to_name(err));
    } else {
      // Wait for the GAP event (max 1 second)
      uint32_t timeout = millis() + 1000;
      while (!advStopped && millis() < timeout) {
        delay(10);
      }
      Serial.println("Advertising stopped (confirmed by event).");
    }
    delay(100);
  }

  err = esp_bluedroid_disable();
  if (err != ESP_OK) Serial.printf("bluedroid_disable error: %s\n", esp_err_to_name(err));
  delay(100);

  err = esp_bluedroid_deinit();
  if (err != ESP_OK) Serial.printf("bluedroid_deinit error: %s\n", esp_err_to_name(err));
  delay(100);

  err = esp_bt_controller_disable();
  if (err != ESP_OK) Serial.printf("controller_disable error: %s\n", esp_err_to_name(err));
  delay(100);

  // err = esp_bt_controller_deinit(); // BUG - seems to crash when deinit() is called
  // if (err != ESP_OK) Serial.printf("controller_deinit error: %s\n", esp_err_to_name(err));

  Serial.println("BLE shutdown complete.");
}

void loop() {
  // Nothing here — deep sleep handles the cycle
}
