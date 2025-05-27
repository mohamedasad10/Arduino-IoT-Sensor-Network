# Arduino-IoT-Sensor-Network
ESP-NOW Receiver Dashboard

This project is an ESP32-based receiver that uses **ESP-NOW** protocol to receive sensor data packets from multiple ESP32 sensor nodes. The receiver acts as a Wi-Fi station and runs an asynchronous web server to display a simple dashboard for monitoring sensor readings in real-time.

---

## Features

- Receives sensor data (temperature, humidity, light intensity, reading ID) from up to 4 ESP32 sensor nodes over ESP-NOW.
- Displays real-time sensor data on a web dashboard via HTTP server.
- Uses AsyncWebServer and Server-Sent Events (SSE) for live data communication.
- Wi-Fi station mode connects to your local network to provide access to the dashboard.
- Displays sender MAC address and validates incoming data.

---

## Hardware

- ESP32 development board (receiver)
- Multiple ESP32 sensor nodes (senders) sending temperature, humidity, and light data via ESP-NOW

---

## Software Dependencies

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON)
- ESP32 Arduino core (latest)

---

## Setup Instructions

1. **Configure Wi-Fi credentials**  
   Update the following lines in `receiver.ino` with your Wi-Fi network credentials:
   ```cpp
   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_PASSWORD";
