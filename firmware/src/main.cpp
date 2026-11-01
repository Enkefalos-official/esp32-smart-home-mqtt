/*
 * ESP32 Smart Home Node
 * ---------------------
 * Reads temperature/humidity (DHT22) and motion (PIR), controls a relay,
 * and talks to any MQTT broker. Auto-registers with Home Assistant via
 * MQTT Discovery.
 *
 * Author: Akram Hssaine (Enkefalos)
 * License: MIT
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#include "config.h"   // copy include/config.example.h -> include/config.h

// ---------- Hardware ----------
DHT dht(PIN_DHT, DHT22);

// ---------- Networking ----------
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// ---------- Topics ----------
String baseTopic;      // home/<device_id>
String stateTopic;     // home/<device_id>/state
String relaySetTopic;  // home/<device_id>/relay/set
String availTopic;     // home/<device_id>/status

// ---------- State ----------
bool relayOn = false;
bool motion = false;
unsigned long lastPublish = 0;
unsigned long lastReconnectAttempt = 0;

// ------------------------------------------------------------------
void setRelay(bool on) {
  relayOn = on;
  digitalWrite(PIN_RELAY, on ? RELAY_ACTIVE_LEVEL : !RELAY_ACTIVE_LEVEL);
}

void publishState() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  JsonDocument doc;
  if (!isnan(t)) doc["temperature"] = roundf(t * 10) / 10.0;
  if (!isnan(h)) doc["humidity"] = roundf(h * 10) / 10.0;
  doc["motion"] = motion ? "ON" : "OFF";
  doc["relay"] = relayOn ? "ON" : "OFF";
  doc["rssi"] = WiFi.RSSI();
  doc["uptime_s"] = millis() / 1000;

  char buf[256];
  size_t n = serializeJson(doc, buf);
  mqtt.publish(stateTopic.c_str(), (const uint8_t*)buf, n, true);
  Serial.printf("[pub] %s\n", buf);
}

// Publishes one Home Assistant discovery config message.
void discover(const char* component, const char* key, const char* name,
              const char* deviceClass, const char* unit,
              const char* valueTemplate, bool commandable = false) {
  JsonDocument doc;
  String uid = String(DEVICE_ID) + "_" + key;
  doc["name"] = name;
  doc["unique_id"] = uid;
  doc["state_topic"] = stateTopic;
  doc["availability_topic"] = availTopic;
  doc["value_template"] = valueTemplate;
  if (deviceClass) doc["device_class"] = deviceClass;
  if (unit) doc["unit_of_measurement"] = unit;
  if (commandable) {
    doc["command_topic"] = relaySetTopic;
    doc["payload_on"] = "ON";
    doc["payload_off"] = "OFF";
  }
  if (strcmp(component, "binary_sensor") == 0) {
    doc["payload_on"] = "ON";
    doc["payload_off"] = "OFF";
  }
  JsonObject dev = doc["device"].to<JsonObject>();
  dev["identifiers"][0] = DEVICE_ID;
  dev["name"] = DEVICE_NAME;
  dev["manufacturer"] = "Enkefalos";
  dev["model"] = "ESP32 Smart Home Node";

  String topic = String("homeassistant/") + component + "/" + uid + "/config";
  char buf[640];
  size_t n = serializeJson(doc, buf);
  mqtt.publish(topic.c_str(), (const uint8_t*)buf, n, true);
}

void publishDiscovery() {
  discover("sensor", "temp", "Temperature", "temperature", "°C",
           "{{ value_json.temperature }}");
  discover("sensor", "hum", "Humidity", "humidity", "%",
           "{{ value_json.humidity }}");
  discover("binary_sensor", "motion", "Motion", "motion", nullptr,
           "{{ value_json.motion }}");
  discover("switch", "relay", "Relay", nullptr, nullptr,
           "{{ value_json.relay }}", true);
}

void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();
  msg.toUpperCase();
  Serial.printf("[sub] %s -> %s\n", topic, msg.c_str());

  if (relaySetTopic == topic) {
    if (msg == "ON") setRelay(true);
    else if (msg == "OFF") setRelay(false);
    else if (msg == "TOGGLE") setRelay(!relayOn);
    publishState();
  }
}

// ------------------------------------------------------------------
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_ID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to %s", WIFI_SSID);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected" : " failed");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  }
}

bool connectMqtt() {
  Serial.print("MQTT connecting... ");
  bool ok = mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASSWORD,
                         availTopic.c_str(), 1, true, "offline");
  if (!ok) {
    Serial.printf("failed (rc=%d)\n", mqtt.state());
    return false;
  }
  Serial.println("ok");
  mqtt.publish(availTopic.c_str(), "online", true);
  mqtt.subscribe(relaySetTopic.c_str());
  publishDiscovery();
  publishState();
  return true;
}

// ------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n== ESP32 Smart Home Node ==");

  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_PIR, INPUT);
  setRelay(false);
  dht.begin();

  baseTopic = String("home/") + DEVICE_ID;
  stateTopic = baseTopic + "/state";
  relaySetTopic = baseTopic + "/relay/set";
  availTopic = baseTopic + "/status";

  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(768);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }

  if (!mqtt.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      connectMqtt();
    }
    return;
  }
  mqtt.loop();

  // Publish immediately when motion state changes.
  bool m = digitalRead(PIN_PIR) == HIGH;
  if (m != motion) {
    motion = m;
    publishState();
  }

  if (millis() - lastPublish > PUBLISH_INTERVAL_MS) {
    lastPublish = millis();
    publishState();
  }
}
