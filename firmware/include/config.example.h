#pragma once
// Copy this file to config.h and fill in your values.
// config.h is git-ignored so your credentials never get committed.

// ---------- Wi-Fi ----------
#define WIFI_SSID      "your-wifi-name"
#define WIFI_PASSWORD  "your-wifi-password"

// ---------- MQTT broker ----------
#define MQTT_HOST      "192.168.1.10"   // e.g. Mosquitto on a Raspberry Pi
#define MQTT_PORT      1883
#define MQTT_USER      ""               // leave empty if anonymous
#define MQTT_PASSWORD  ""

// ---------- Device ----------
#define DEVICE_ID      "esp32-livingroom"
#define DEVICE_NAME    "Living Room Node"
#define PUBLISH_INTERVAL_MS 30000

// ---------- Pins ----------
#define PIN_DHT    4    // DHT22 data
#define PIN_PIR    27   // HC-SR501 output
#define PIN_RELAY  26   // relay module IN
#define RELAY_ACTIVE_LEVEL LOW   // most relay boards are active-LOW
