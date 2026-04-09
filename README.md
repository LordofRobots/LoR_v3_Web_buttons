# LoR Core V3 Web Interface Robot Control

## Description

This project turns a LoR Core V3 into a self-hosted robot control system with a browser-based driving interface.

The ESP32 creates its own Wi-Fi network, hosts a web page, and communicates with that page over WebSocket for low-latency control. The interface provides a virtual joystick, four function buttons, and live telemetry so the robot can be driven from a phone, tablet, or computer without installing an app.

![shared image](https://github.com/user-attachments/assets/7931de68-f467-4f1c-a51e-371bb26e73d8 =200x) 

![shared image (1)](https://github.com/user-attachments/assets/dc25ae7c-d5e0-4230-a659-3268b49c197b =200x)

![shared image (2)](https://github.com/user-attachments/assets/32459e3b-1849-464d-825d-4a4756543ef7 =100x)

![shared image (3)](https://github.com/user-attachments/assets/1a7c693a-d3f2-482c-b158-56194946af73 =100x)

---

## Environment Setup

### Arduino IDE Libraries

    - "Async TCP": by ESP32Async
    - "ESP Async Webserver": by ESP32Async
    - "FastLED": by Daniel Garcia
    - "ESP32Servo" specifically Version 3.0.7: by Kevin Harrington, John K. Bennett

### ESP32 Board Package

Add to Arduino IDE under File → Preferences → "external board manerger url" :
https://dl.espressif.com/dl/package_esp32_index.json

Install:
- esp32 by Espressif Systems

### Board Selection

Tools → Board → ESP32 Arduino → ESP32 Dev Module

---

## User Guide

### 1. Set custom Wi-Fi SSID and Password

the robot hosts it's own AP wifi network you can connect to.

Update in code:

static const char *AP_SSID = "My Robot";
static const char *AP_PASS = "mypassword";

### 2. Upload Firmware

- Select Board: ESP32 Dev Module
- Select correct COM port
- Upload

### 3. Connect to Robot

- Open Wi-Fi settings
- Connect to your robot network

### 4. Open Interface

on any browser on either PC of Mobile, Go to:
http://10.0.0.1

Chrome recommended.

---

## Features

### Joystick
- Full XY control
- Auto stop on release
- Continuous command streaming

### Buttons (A/B/C/D)
- Trigger custom robot functions

### Telemetry
- Battery voltage + graph
- RSSI signal strength
- Lag (ms)
- Command age
- Heap memory
- Input states (A/B/C/D/SW)
- Uptime
- Connected stations
- System info

### UI
- Responsive layout (mobile + desktop)
- Light/Dark theme toggle
- WebSocket connection status

---

## Notes

- No internet required
- Fixed IP: 10.0.0.1
- Works on any modern browser
