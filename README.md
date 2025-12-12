# PC Status In LCD via MQTT

<img src="assets/LCD-Display.png" alt="LCD Display"/>   

This project consists of a Python script running on a PC that collects system statistics (CPU usage, memory usage, uptime) and publishes them via MQTT. An ESP32 microcontroller subscribes to the MQTT topic and displays the data on an I2C LCD screen.

## Features

- Real-time PC monitoring: CPU percentage, RAM percentage, uptime
- MQTT communication between PC and ESP32
- LCD display on ESP32 showing CPU/RAM and uptime
- Automatic data publishing every 1 second

## Requirements

### PC Side

- Python 3.6+
- Libraries: `paho-mqtt`, `psutil`
- Mosquitto MQTT broker installed and running

### ESP32 Side

- ESP32-S3 board
- I2C LCD (16x2, address 0x27 or 0x3F)
- Arduino IDE with ESP32 board support
- Libraries: `WiFi`, `Wire`, `LiquidCrystal_I2C`, `PubSubClient`, `ArduinoJson`

## Setup

### PC Setup

1. Install Python dependencies:

   ```bash
   git clone https://github.com/mojahid2021/PC-Status-In-LCD.git
   cd PC-Status-In-LCD
   ```

   ```bash
   python3 -m venv venv
   source venv/bin/activate
   ```

   ```bash
   pip install -r requirements.txt
   ```

   Or manually:

   ```bash
   pip install paho-mqtt psutil
   ```

2. Install and start Mosquitto MQTT broker:

   ```bash
   sudo apt update
   sudo apt install mosquitto mosquitto-clients
   sudo systemctl start mosquitto
   sudo systemctl enable mosquitto
   ```

3. Verify broker is running:

   ```bash
   ss -tlnp | grep 1883
   ```

### ESP32 Setup

1. Install Arduino IDE and add ESP32 board support:
   - Go to File > Preferences > Additional Boards Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools > Board > Boards Manager > Install ESP32

2. Install required libraries in Arduino IDE:
   - LiquidCrystal I2C by Frank de Brabander
   - PubSubClient by Nick O'Leary
   - ArduinoJson by Benoit Blanchon

3. Update WiFi credentials in `sketch_dec12a.ino`:

   ```cpp
   const char* WIFI_SSID = "your_wifi_name";
   const char* WIFI_PASS = "your_wifi_password";
   ```

4. Update MQTT server IP if different:

   ```cpp
   const char* MQTT_SERVER = "192.168.0.xxx";  // PC's IP
   ```

5. Upload the code to ESP32-S3.

## Running

1. Start the PC script:

   ```bash
   python3 pc_mqtt_stats.py
   ```

2. The ESP32 will automatically connect to WiFi and MQTT, then display:
   - Line 1: CPU and RAM percentages (e.g., "CPU:15 RAM:68")
   - Line 2: Uptime in HH:MM:SS (e.g., "Up: 01:23:45")

## Troubleshooting

- **ESP32 not displaying data**: Check Serial Monitor for connection errors. Ensure WiFi credentials and MQTT IP are correct.
- **MQTT connection fails**: Verify Mosquitto is running and ESP32 is on the same network.
- **LCD not working**: Confirm I2C address (0x27 or 0x3F) and wiring (SDA=8, SCL=9 for ESP32-S3).
- **No data received**: Check PC script output for "MQTT connected" and "Published".

## Files

- `pc_mqtt_stats.py`: Python script for PC stats collection and MQTT publishing
- `sketch_dec12a.ino`: Arduino code for ESP32 MQTT subscription and LCD display
- `README.md`: This documentation