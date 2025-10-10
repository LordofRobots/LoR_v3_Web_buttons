# LoR_Core_WebInterface 

Self-hosted Wi-Fi AP + HTTP control UI for MiniBot running on LoR Core V3.
Drive uses ESP32Servo (90° = stop). LEDs use FastLED on WS2812B.

**Version:** AUG 27, 2025

---

## Purpose

* Bring-up and test MiniBot without external router or app.
* Expose a simple web page at `http://10.0.0.1` to drive the robot.
* Provide keyboard and on-screen controls.
* Offer stub hooks for A..D functions.

---

## Quick start — user walkthrough

1. **Power on** LoR Core V3.
2. On your phone/PC, **join Wi-Fi**:

   * SSID: `MiniBot_v3`
   * PASS: `password`
     *(change both in code before deployment)*
3. Open a browser to **`http://10.0.0.1`**.
4. Use the UI:

   * **Drive:** Forward / Left / Stop / Right / Back.
   * **Speed:** toggle Low/High.
   * **Functions:** A / B / C / D (placeholders).
5. Optional desktop keys:

   * **Drive:** Arrow keys or **W/A/S/D**
   * **Speed:** **L** = Low, **H** = High
   * **Functions:** **1–4** or Numpad **1–4**
6. LED cues:

   * Blue = boot, White = ready, Red = page/stop, Green = command,
   * Yellow = Low speed, Purple = High speed, Cyan = A..D.

---

## IDE setup and libraries

### Arduino IDE

* Arduino IDE **2.x**
* **ESP32 board support** installed (Arduino core for ESP32 v3.x).
* **Board:** *ESP32 Dev Module* (or your specific Core V3 target).
* **Baud:** 115200.

### Libraries (Arduino)

* **ESP32Servo** (madhephaestus)
* **FastLED**
  *(Other headers like `esp_http_server.h`, `esp_wifi.h`, `WiFi.h`, `ESPmDNS.h` come with the ESP32 core.)*

### PlatformIO (optional)

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
  madhephaestus/ESP32Servo
  FastLED/FastLED
monitor_speed = 115200
```

---

## Build and flash

1. Open the sketch.
2. Edit **SSID/PASS** in:

   ```cpp
   const String ssid = "MiniBot_v3";
   const String password = "password";
   ```
3. Connect the board by USB.
4. Select the correct port and board.
5. Upload.
6. Open Serial Monitor at **115200**. You should see `AP IP: 10.0.0.1`.

---

## How to use (operator)

* Connect to the robot AP and open `http://10.0.0.1`.
* Hold a drive button to move. Release → **Stop** is sent automatically.
* Switch **Low/High** for precision or faster response.
* A..D buttons are available for custom actions as your build requires.

**HTTP endpoints**

| Route            | Method | Purpose               |
| ---------------- | ------ | --------------------- |
| `/`              | GET    | Serves the control UI |
| `/action?go=CMD` | GET    | Executes a command    |

**Valid `CMD` values:** `forward,left,right,backward,stop,low,high,functionA,functionB,functionC,functionD`

---

## Code walkthrough

### 1) Configuration and pins

* **LEDs:** WS2812B on `LED_PIN=33`, `LED_COUNT=4`, FastLED `GRB`.
* **Motors:** `IO_PINS[1..12]` define 12 slots. `Servo MotorOutput[13]` with **90° = stop**.
* **Inputs:** User buttons (A..D, SW) and `VIN_SENSE` are defined for future use.

### 2) Motor profiles

* `MotorType` + `MotorTypeConfig[]` set PWM freq and pulse range per device (e.g., `N20Plus`, `MG90_CR`, `STD_SERVO`).
* `ConfigureMotorOutput(slot, type, startupDeg)` attaches a slot with the profile and sets start angle.

### 3) Drive layer

* `percentToServoAngle(pct)` maps **[-100..+100]** to **[0..180]**.
* Tank mixing:

  * Left group = slots **1..6**
  * Right group = slots **7..12**
* `Motor_Control(Lpct, Rpct)` writes both groups. `Motor_STOP()` writes 90° to all.

### 4) Command bindings

* `functionForward/Backward/Left/Right/Stop()` call the drive layer using the current `driveSpeed`.
* `functionA..D()` are stub hooks.

### 5) LED helpers

* `LED_SetSolid(r,g,b)` plus named wrappers (Red, Green, Blue, White, Yellow, Cyan, Purple, Off).
* Used as minimal UI feedback during requests and state changes.

### 6) Embedded Web UI

* `INDEX_HTML` is served from flash.
* Buttons call `/action?go=...`.
* Keyboard listeners send the same commands.
* Pointer down → command; pointer up → `stop`.

### 7) HTTP server

* ESP-IDF httpd.
* `index_handler` serves the page.
* `cmd_handler` parses `go=` and switches on the command. Unknown → **Stop**.
* CORS header `Access-Control-Allow-Origin: *` is set for `/action`.

### 8) Wi-Fi AP

* Static IP **10.0.0.1** via `WiFi.softAPConfig()`.
* AP via `WiFi.softAP(ssid, password)`.
* mDNS `robot.local` is advertised (client support varies).

### 9) setup()/loop()

* Disables brownout detector via `WRITE_PERI_REG(...)` to match original behavior.
* Initializes Serial, inputs, LEDs, motors, Wi-Fi, and server.
* `loop()` is empty; system is event-driven by HTTP handlers.

---

## Configuration points

* **Speeds:**

  ```cpp
  int highSpeed = 90;   // percent
  int lowSpeed  = 60;
  ```
* **Per-slot motor types:**

  ```cpp
  for (int s = 1; s <= 11; ++s) ConfigureMotorOutput(s, N20Plus, 90);
  ConfigureMotorOutput(12, MG90_CR, 90);
  ```
* **LEDs:**

  ```cpp
  #define LED_PIN 33
  #define LED_COUNT 4
  #define COLOR_ORDER GRB
  #define CHIPSET WS2812B
  ```

---

## Safety

* Lift the robot or remove wheels during first power-on.
* Verify servo direction and neutral at **90°** before full power.
* Confirm Wi-Fi is isolated for public events if needed.

---

## Troubleshooting

* **Cannot load page:** confirm joined to `MiniBot_v3`. Use `http://10.0.0.1`.
* **No AP visible:** check Serial for boot logs and brownout messages.
* **Controls lag:** limit multiple key presses; one press sends one command until keyup.

---

## License

Apache License 2.0
A permissive license whose main conditions require preservation of copyright and license notices. 
Contributors provide an express grant of patent rights. Licensed works, modifications, and larger works may be distributed under different terms and without source code.
