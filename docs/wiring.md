# Wiring

| Component            | Pin on module | ESP32 pin | Notes                                  |
|----------------------|---------------|-----------|----------------------------------------|
| DHT22                | VCC           | 3V3       |                                        |
|                      | DATA          | GPIO 4    | 10 kΩ pull-up to 3V3 (if not on board) |
|                      | GND           | GND       |                                        |
| HC-SR501 PIR         | VCC           | VIN (5V)  | PIR needs 5V supply                    |
|                      | OUT           | GPIO 27   | Output is 3.3V, safe for ESP32         |
|                      | GND           | GND       |                                        |
| 1-channel relay      | VCC           | VIN (5V)  |                                        |
|                      | IN            | GPIO 26   | Active-LOW on most boards              |
|                      | GND           | GND       |                                        |

```
            +-----------------+
   DHT22 ---| GPIO 4          |
     PIR ---| GPIO 27   ESP32 |
   Relay ---| GPIO 26         |
            | 3V3  VIN  GND   |
            +-----------------+
```

## Safety

Switching mains voltage (110/230 V) with the relay is dangerous. Use an enclosed
relay module, keep low-voltage and mains wiring separated, and get help from
someone qualified if you have never worked with mains before. For testing, switch
a 5–12 V LED strip or fan instead.
