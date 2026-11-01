#!/usr/bin/env python3
"""
mqtt_monitor.py - live terminal monitor and controller for ESP32 Smart Home nodes.

Usage:
    pip install paho-mqtt
    python mqtt_monitor.py --host 192.168.1.10
    python mqtt_monitor.py --host 192.168.1.10 --relay esp32-livingroom ON

Author: Akram Hssaine (Enkefalos) - MIT License
"""
import argparse
import json
import time
from datetime import datetime

import paho.mqtt.client as mqtt


def on_connect(client, userdata, flags, reason_code, properties=None):
    print(f"[{now()}] connected to broker (code {reason_code})")
    client.subscribe("home/+/state")
    client.subscribe("home/+/status")


def on_message(client, userdata, msg):
    device = msg.topic.split("/")[1]
    payload = msg.payload.decode(errors="replace")
    if msg.topic.endswith("/status"):
        print(f"[{now()}] {device:<20} is {payload.upper()}")
        return
    try:
        s = json.loads(payload)
    except json.JSONDecodeError:
        print(f"[{now()}] {device:<20} raw: {payload}")
        return
    print(
        f"[{now()}] {device:<20} "
        f"T={s.get('temperature', '--'):>5}°C  "
        f"H={s.get('humidity', '--'):>5}%  "
        f"motion={s.get('motion', '--'):<3}  "
        f"relay={s.get('relay', '--'):<3}  "
        f"rssi={s.get('rssi', '--')}dBm"
    )


def now():
    return datetime.now().strftime("%H:%M:%S")


def main():
    p = argparse.ArgumentParser(description="Monitor/control ESP32 smart home nodes over MQTT")
    p.add_argument("--host", default="localhost")
    p.add_argument("--port", type=int, default=1883)
    p.add_argument("--user")
    p.add_argument("--password")
    p.add_argument("--relay", nargs=2, metavar=("DEVICE_ID", "ON|OFF|TOGGLE"),
                   help="send a relay command and exit")
    a = p.parse_args()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    if a.user:
        client.username_pw_set(a.user, a.password)

    if a.relay:
        device, cmd = a.relay
        client.connect(a.host, a.port, 30)
        client.loop_start()
        info = client.publish(f"home/{device}/relay/set", cmd.upper(), qos=1)
        info.wait_for_publish()
        print(f"sent {cmd.upper()} to {device}")
        time.sleep(0.2)
        client.loop_stop()
        client.disconnect()
        return

    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(a.host, a.port, 30)
    print("Listening... Ctrl+C to quit")
    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nbye")


if __name__ == "__main__":
    main()
