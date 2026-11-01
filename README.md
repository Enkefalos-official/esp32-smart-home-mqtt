# ESP32 Smart Home Node

An open-source IoT node that turns a ~$10 ESP32 into a smart-home device:
it reads **temperature, humidity and motion**, controls a **relay**, and talks
to any **MQTT** broker. It registers itself with **Home Assistant** automatically
through MQTT Discovery, so there is no YAML to write.

![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange)
![MQTT](https://img.shields.io/badge/protocol-MQTT-660066)
![Home Assistant](https://img.shields.io/badge/Home%20Assistant-discovery-41BDF5)
![License: MIT](https://img.shields.io/badge/license-MIT-green)

## Features

- DHT22 temperature and humidity, published every 30 s (configurable)
- PIR motion sensor, published instantly on change
- Relay control over MQTT: `ON`, `OFF`, `TOGGLE`
- Home Assistant auto-discovery (sensors, motion binary sensor, relay switch)
- Last-will "offline" status, so dashboards know when a node drops
- Auto-reconnect for Wi-Fi and MQTT
- Python CLI to monitor every node and send commands from a terminal

## Architecture

```
 [DHT22] [PIR] [Relay]
     \     |     /
      [ ESP32 node ] --Wi-Fi--> [ MQTT broker (Mosquitto) ] <--> [ Home Assistant ]
                                          ^
                                          |
                                 [ tools/mqtt_monitor.py ]
```

## MQTT topics

| Topic                         | Direction | Payload                                        |
|-------------------------------|-----------|------------------------------------------------|
| `home/<id>/state`             | out       | JSON: temperature, humidity, motion, relay, rssi |
| `home/<id>/status`            | out       | `online` / `offline` (retained, last will)     |
| `home/<id>/relay/set`         | in        | `ON` / `OFF` / `TOGGLE`                        |
| `homeassistant/.../config`    | out       | Discovery configs (retained)                   |

Example state message:

```json
{"temperature":23.4,"humidity":51.2,"motion":"OFF","relay":"ON","rssi":-58,"uptime_s":3721}
```

## Hardware

- ESP32 DevKit (any ESP32-WROOM board)
- DHT22 sensor
- HC-SR501 PIR motion sensor
- 1-channel 5V relay module
- Jumper wires and a breadboard

See [docs/wiring.md](docs/wiring.md) for pinout and safety notes.

## Getting started

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI).
2. Copy the config and fill in your Wi-Fi and broker details:
   ```bash
   cd firmware
   cp include/config.example.h include/config.h
   ```
3. Build and flash:
   ```bash
   pio run -t upload && pio device monitor
   ```
4. Watch your nodes from any computer:
   ```bash
   pip install paho-mqtt
   python tools/mqtt_monitor.py --host <broker-ip>
   python tools/mqtt_monitor.py --host <broker-ip> --relay esp32-livingroom TOGGLE
   ```

No broker yet? On a Raspberry Pi or Linux machine:
`sudo apt install mosquitto mosquitto-clients`.

## Roadmap

- [ ] OTA firmware updates
- [ ] Deep-sleep battery mode
- [ ] TLS for remote brokers
- [ ] Web config portal (no hard-coded credentials)
- [ ] More sensors: BME280, soil moisture, gas (MQ-2)

## License

MIT © 2026 Akram Hssaine
