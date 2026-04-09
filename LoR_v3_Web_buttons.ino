/* UPDATED: APRIL 08 2026
================================================================================
  LORD of ROBOTS — LoR_Core_WebInterface (AsyncWebServer + WebSocket)
  Purpose: Self-hosted Wi-Fi AP + WebSocket joystick control + telemetry stream
  Target:  LoR Core V3 (ESP32)

  REQUIRED LIBRARIES
  ------------------------------------------------------------------------------
  Install these in Arduino IDE before compiling:

    - "Async TCP" by ESP32Async
    - "ESP Async Webserver" by ESP32Async
    - "ESP32Servo" by Kevin Harrington, John K. Bennett
    - "FastLED" by Daniel Garcia

  REQUIRED BOARD PACKAGE
  ------------------------------------------------------------------------------
  In Arduino IDE:
    File → Preferences → Additional Boards Manager URLs
    Add:
      https://dl.espressif.com/dl/package_esp32_index.json

  Then:
    Tools → Board → Boards Manager
    Install:
      "esp32" by Espressif Systems
  
  UI
  ------------------------------------------------------------------------------
    - SSID: "Minibot Web Interface" or change to as you see fit
    - IP:   10.0.0.1 (static)
    - AsyncWebServer + WebSocket (/ws) (libraries by ESP32Async)
    - Joystick (x,y normalized [-1..1]) drives tank mix
    - Buttons A/B/C/D send function events
    - Telemetry pushed periodically:
        vin, vin_raw, rssi_dbm, lag estimate handled client-side,
        active-low buttons A/B/C/D + switch (external pullups),
        uptime, cmd_age, stations, heap, system info.

  SYSTEM OVERVIEW
  ------------------------------------------------------------------------------
  This firmware creates a standalone robot control system using:

    1. ESP32 in Access Point mode
    2. Browser-hosted control UI
    3. WebSocket-based real-time command + telemetry link
    4. Servo-style output control for motors / actuators
    5. LED state feedback
    6. Telemetry and fail-safe stopping logic

================================================================================
/*
================================================================================
  LORD of ROBOTS — LoR_Core_WebInterface (AsyncWebServer + WebSocket)
  Purpose: Self-hosted Wi-Fi AP + WebSocket joystick control + telemetry stream
  Target:  LoR Core V3 (ESP32)

  REQUIRED LIBRARIES
  ------------------------------------------------------------------------------
  Install these in Arduino IDE before compiling:

    - "Async TCP" Version 3.4.10 by ESP32Async
    - "ESP Async Webserver" Version 3.10.3 by ESP32Async
    - "ESP32Servo" Version 3.0.7 by Kevin Harrington, John K. Bennett
    - "FastLED" Version 3.10.3 by Daniel Garcia

  REQUIRED BOARD PACKAGE
  ------------------------------------------------------------------------------
  In Arduino IDE:
    File → Preferences → Additional Boards Manager URLs
    Add:
      https://dl.espressif.com/dl/package_esp32_index.json

  Then:
    Tools → Board → Boards Manager
    Install:
      "esp32" by Espressif Systems
  
  UI
  ------------------------------------------------------------------------------
    - SSID: "Minibot Web Interface" or change to as you see fit
    - IP:   10.0.0.1 (static)
    - AsyncWebServer + WebSocket (/ws) (libraries by ESP32Async)
    - Joystick (x,y normalized [-1..1]) drives tank mix
    - Buttons A/B/C/D send function events
    - Telemetry pushed periodically:
        vin, vin_raw, rssi_dbm, lag estimate handled client-side,
        active-low buttons A/B/C/D + switch (external pullups),
        uptime, cmd_age, stations, heap, system info.

  SYSTEM OVERVIEW
  ------------------------------------------------------------------------------
  This firmware creates a standalone robot control system using:

    1. ESP32 in Access Point mode
    2. Browser-hosted control UI
    3. WebSocket-based real-time command + telemetry link
    4. Servo-style output control for motors / actuators
    5. LED state feedback
    6. Telemetry and fail-safe stopping logic

================================================================================
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESP32Servo.h>
#include <FastLED.h>
#include "index_html.h"

// ------------------------------
// VERSION
// ------------------------------
static const char *FW_VERSION = "LoR Core V3 Web Interface FW - APRIL 2026";

// ------------------------------
// Wi-Fi AP config (static 10.0.0.1)
// ------------------------------
static const char *AP_SSID = "Minibot v3 Web Interface";  // CHANGE TO YOUR NAME OF CHOICE
static const char *AP_PASS = "password";

IPAddress AP_IP(10, 0, 0, 1);
IPAddress AP_GW(10, 0, 0, 1);
IPAddress AP_SN(255, 255, 255, 0);

// ------------------------------
// LoR Core V3 pins
// ------------------------------
const uint8_t AUX_PINS[9] = { 0, 5, 18, 23, 19, 22, 21, 1, 3 };
const uint8_t IO_PINS[13] = { 0, 32, 25, 26, 27, 14, 12, 13, 15, 2, 4, 22, 21 };

// ------------------------------
// User inputs (active-low with external pull-ups)
// ------------------------------
#define User_BTN_A 35
#define User_BTN_B 39
#define User_BTN_C 38
#define User_BTN_D 37
#define User_SW 36

// ------------------------------
// VIN sense
// ------------------------------
#define VIN_SENSE 34
#define VOLT_SLOPE 0.0063492
#define VOLT_OFFSET 1.079

// ------------------------------
// LEDs
// ------------------------------
#define LED_PIN 33
#define LED_COUNT 4
#define BRIGHTNESS 255
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
CRGB leds[LED_COUNT];

// ------------------------------
// Serial debug state
// ------------------------------
static const uint32_t DEBUG_PRINT_PERIOD_MS = 200;
static uint32_t g_lastDebugPrintMs = 0;

static float g_dbgJoyX = 0.0f;
static float g_dbgJoyY = 0.0f;
static bool g_dbgJoyDirty = false;

static char g_dbgLastWebBtn = 0;
static bool g_dbgWebBtnDirty = false;

static uint8_t g_dbgBtnA = 1;
static uint8_t g_dbgBtnB = 1;
static uint8_t g_dbgBtnC = 1;
static uint8_t g_dbgBtnD = 1;
static uint8_t g_dbgSw = 1;
static bool g_dbgInputsDirty = true;

// ------------------------------
// Motor layer (Servo)
// ------------------------------
Servo MotorOutput[13];

enum MotorType {
  NFG,
  MG90_CR,
  MG90_Degree,
  N20Plus,
  STD_SERVO,
  Victor_SPX,
  Talon_SRX,
  CUSTOM
};

struct MotorTypeConfig {
  MotorType type;
  float pwmFreq;
  int minPulseUs;
  int maxPulseUs;
};

MotorTypeConfig motorTypeConfigs[] = {
  { NFG, 50, 500, 2500 },
  { MG90_CR, 50, 500, 2500 },
  { MG90_Degree, 50, 500, 2500 },
  { N20Plus, 50, 1000, 2000 },
  { Victor_SPX, 50, 1000, 2000 },
  { Talon_SRX, 50, 1000, 2000 },
  { STD_SERVO, 50, 1000, 2000 },
};

static inline int pctToServoAngle_(int pct) {
  pct = constrain(pct, -100, 100);
  long ang = map(pct, -100, 100, 0, 180);
  return (int)constrain(ang, 0, 180);
}

void ConfigureMotorOutput(uint8_t slot, MotorType motorType, int startupPositionDeg = 90) {
  float pwmFreq = 50;
  int minPulseUs = 1000;
  int maxPulseUs = 2000;

  for (auto &cfg : motorTypeConfigs) {
    if (cfg.type == motorType) {
      pwmFreq = cfg.pwmFreq;
      minPulseUs = cfg.minPulseUs;
      maxPulseUs = cfg.maxPulseUs;
      break;
    }
  }

  const uint8_t pin = IO_PINS[slot];
  pinMode(pin, OUTPUT);
  MotorOutput[slot].setPeriodHertz((int)pwmFreq);
  MotorOutput[slot].attach(pin, minPulseUs, maxPulseUs);
  MotorOutput[slot].write(startupPositionDeg);
}

void Left_Group_Write(int pct) {
  const int angle = pctToServoAngle_(pct);
  for (int s = 7; s <= 12; ++s) MotorOutput[s].write(angle);
}

void Right_Group_Write(int pct) {
  const int angle = pctToServoAngle_(pct);
  for (int s = 1; s <= 6; ++s) MotorOutput[s].write(angle);
}

void Motor_Control(int leftPct, int rightPct) {
  Left_Group_Write(leftPct);
  Right_Group_Write(rightPct);
}

void Motor_STOP() {
  for (int s = 1; s <= 12; ++s) MotorOutput[s].write(90);
}

// ------------------------------
// LED state manager
// ------------------------------
enum LedMode {
  LEDM_BOOT,
  LEDM_DISCONNECTED,  // alternating blue/white at 1 Hz
  LEDM_IDLE,          // red
  LEDM_COMMAND,       // green
  LEDM_FUNCTION       // cyan
};

static volatile LedMode g_ledRequested = LEDM_BOOT;
static LedMode g_ledApplied = LEDM_BOOT;

static uint32_t g_ledFnHoldUntilMs = 0;
static const uint32_t LED_FN_HOLD_MS = 200;

inline void LED_SetSolid_(uint8_t r, uint8_t g, uint8_t b) {
  fill_solid(leds, LED_COUNT, CRGB(r, g, b));
  FastLED.show();
}

inline void LED_SetDisconnectedPattern_(bool phase) {
  if (!phase) {
    leds[0] = CRGB(0, 0, 255);      // LED 1 blue
    leds[1] = CRGB(255, 255, 255);  // LED 2 white
    leds[2] = CRGB(0, 0, 255);      // LED 3 blue
    leds[3] = CRGB(255, 255, 255);  // LED 4 white
  } else {
    leds[0] = CRGB(255, 255, 255);  // LED 1 white
    leds[1] = CRGB(0, 0, 255);      // LED 2 blue
    leds[2] = CRGB(255, 255, 255);  // LED 3 white
    leds[3] = CRGB(0, 0, 255);      // LED 4 blue
  }
  FastLED.show();
}

inline void LED_Boot() {
  g_ledRequested = LEDM_BOOT;
}

inline void LED_Ready() {
  g_ledRequested = LEDM_DISCONNECTED;  // alternating blue/white disconnected pattern
}

inline void LED_Cmd() {
  g_ledRequested = LEDM_COMMAND;  // green
}

inline void LED_Stop() {
  g_ledRequested = LEDM_IDLE;  // red
}

inline void LED_Fn() {
  g_ledRequested = LEDM_FUNCTION;  // cyan
  g_ledFnHoldUntilMs = millis() + LED_FN_HOLD_MS;
}

void serviceLed_() {
  const uint32_t now = millis();

  LedMode desired = g_ledRequested;

  // Cyan overrides everything for at least 200 ms
  if ((int32_t)(g_ledFnHoldUntilMs - now) > 0) {
    desired = LEDM_FUNCTION;
  }

  // Special handling for disconnected animation:
  // alternate every 500 ms = full 1 Hz swap cycle
  if (desired == LEDM_DISCONNECTED) {
    static bool lastPhase = false;
    const bool phase = ((now / 500) & 0x1) != 0;

    if (g_ledApplied != LEDM_DISCONNECTED || phase != lastPhase) {
      LED_SetDisconnectedPattern_(phase);
      g_ledApplied = LEDM_DISCONNECTED;
      lastPhase = phase;
    }
    return;
  }

  if (desired == g_ledApplied) return;

  switch (desired) {
    case LEDM_BOOT: LED_SetSolid_(0, 0, 255); break;
    case LEDM_DISCONNECTED: break;
    case LEDM_IDLE: LED_SetSolid_(255, 0, 0); break;
    case LEDM_COMMAND: LED_SetSolid_(0, 255, 0); break;
    case LEDM_FUNCTION: LED_SetSolid_(0, 255, 255); break;
    default: break;
  }

  g_ledApplied = desired;
}

// ------------------------------
// Web server + WebSocket
// ------------------------------
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ------------------------------
// Control state
// ------------------------------
static volatile float g_joyX = 0.0f;
static volatile float g_joyY = 0.0f;
static volatile uint32_t g_lastCmdMs = 0;

static const int DRIVE_MAX_PCT = 90;
static const float DEADZONE = 0.06f;

static const uint32_t JOY_RX_TIMEOUT_MS = 150;
static const float JOY_ZERO_EPS = 0.02f;

static inline float clampf_(float v, float lo, float hi) {
  return (v < lo) ? lo : (v > hi) ? hi
                                  : v;
}

static inline float applyDeadzone_(float v) {
  if (fabsf(v) < DEADZONE) return 0.0f;
  float s = (fabsf(v) - DEADZONE) / (1.0f - DEADZONE);
  return copysignf(s, v);
}

static inline bool joyIsZero_(float x, float y) {
  return (fabsf(x) <= JOY_ZERO_EPS) && (fabsf(y) <= JOY_ZERO_EPS);
}

void DriveFromJoystick(float x, float y) {
  x = clampf_(x, -1.0f, 1.0f);
  y = clampf_(y, -1.0f, 1.0f);

  x = applyDeadzone_(x);
  y = applyDeadzone_(y);

  float left = y + x;
  float right = y - x;

  float m = max(fabsf(left), fabsf(right));
  if (m > 1.0f) {
    left /= m;
    right /= m;
  }

  int leftPct = (int)roundf(left * DRIVE_MAX_PCT);
  int rightPct = (int)roundf(right * DRIVE_MAX_PCT);

  Motor_Control(-leftPct, +rightPct);
}

void serviceDriveFromLatestJoy_() {
  const uint32_t now = millis();
  const float x = -g_joyX; //inverted if MG90_CR
  const float y = -g_joyY; //inverted if MG90_CR

  // PRIORITY: if no WebSocket clients are attached, show disconnected pattern
  if (ws.count() == 0) {
    Motor_STOP();
    LED_Ready();
    return;
  }

  if (g_lastCmdMs == 0 || (now - g_lastCmdMs) > JOY_RX_TIMEOUT_MS) {
    Motor_STOP();
    LED_Stop();
    return;
  }

  if (joyIsZero_(x, y)) {
    Motor_STOP();
    LED_Stop();
    return;
  }

  DriveFromJoystick(x, y);
  LED_Cmd();
}

// ------------------------------
// Function hooks (A/B/C/D)
// ------------------------------
void functionA() {}
void functionB() {}
void functionC() {}
void functionD() {}

// ------------------------------
// Telemetry
// ------------------------------
static uint32_t g_lastTelPushMs = 0;
static const uint32_t TEL_PERIOD_MS = 50;

static inline uint16_t readVinRaw_() {
  return (uint16_t)analogRead(VIN_SENSE);
}

static inline float readVinVolts_() {
  const uint16_t raw = readVinRaw_();
  return (raw * VOLT_SLOPE) + VOLT_OFFSET;
}

static inline uint8_t readActiveLow_(uint8_t pin) {
  return (digitalRead(pin) == LOW) ? 0 : 1;
}

static inline int rssiDbm_() {
  wifi_sta_list_t stationList;
  if (esp_wifi_ap_get_sta_list(&stationList) != ESP_OK) return -127;
  if (stationList.num == 0) return -127;
  return stationList.sta[0].rssi;
}

static inline uint8_t stationCount_() {
  wifi_sta_list_t stationList;
  if (esp_wifi_ap_get_sta_list(&stationList) != ESP_OK) return 0;
  return (uint8_t)stationList.num;
}

static inline uint32_t heapFree_() {
  return (uint32_t)ESP.getFreeHeap();
}

static inline uint32_t heapMin_() {
  return (uint32_t)ESP.getMinFreeHeap();
}

static inline uint32_t cpuMhz_() {
  return (uint32_t)getCpuFrequencyMhz();
}

static inline uint32_t flashMb_() {
  return (uint32_t)(ESP.getFlashChipSize() / (1024UL * 1024UL));
}

static inline const char *resetReasonShort_() {
  esp_reset_reason_t r = esp_reset_reason();
  switch (r) {
    case ESP_RST_POWERON: return "PWR";
    case ESP_RST_SW: return "SW";
    case ESP_RST_PANIC: return "PANIC";
    case ESP_RST_INT_WDT: return "WDT";
    case ESP_RST_TASK_WDT: return "WDT";
    case ESP_RST_BROWNOUT: return "BROWN";
    case ESP_RST_DEEPSLEEP: return "SLEEP";
    default: return "UNK";
  }
}

void pushTelemetry_() {
  const float vin = readVinVolts_();
  const uint16_t vinRaw = readVinRaw_();

  const uint32_t now = millis();
  const uint32_t cmdAge = (g_lastCmdMs == 0) ? 0 : (now - g_lastCmdMs);

  const uint8_t bA = readActiveLow_(User_BTN_A);
  const uint8_t bB = readActiveLow_(User_BTN_B);
  const uint8_t bC = readActiveLow_(User_BTN_C);
  const uint8_t bD = readActiveLow_(User_BTN_D);
  const uint8_t sw = readActiveLow_(User_SW);

  char out[512];
  snprintf(out, sizeof(out),
           "{\"type\":\"tel\","
           "\"vin\":%.3f,"
           "\"vin_raw\":%u,"
           "\"rssi_dbm\":%d,"
           "\"btnA\":%u,\"btnB\":%u,\"btnC\":%u,\"btnD\":%u,\"sw\":%u,"
           "\"uptime_ms\":%lu,"
           "\"cmd_age_ms\":%lu,"
           "\"stations\":%u,"
           "\"heap_free\":%lu,"
           "\"heap_min\":%lu,"
           "\"cpu_mhz\":%lu,"
           "\"flash_mb\":%lu,"
           "\"reset\":\"%s\""
           "}",
           vin,
           (unsigned)vinRaw,
           rssiDbm_(),
           (unsigned)bA, (unsigned)bB, (unsigned)bC, (unsigned)bD, (unsigned)sw,
           (unsigned long)now,
           (unsigned long)cmdAge,
           (unsigned)stationCount_(),
           (unsigned long)heapFree_(),
           (unsigned long)heapMin_(),
           (unsigned long)cpuMhz_(),
           (unsigned long)flashMb_(),
           resetReasonShort_());

  ws.textAll(out);
}

// ------------------------------
// WebSocket message handling
// ------------------------------
static bool parseJoy_(const String &s, float &x, float &y) {
  int ix = s.indexOf("\"x\":");
  int iy = s.indexOf("\"y\":");
  if (ix < 0 || iy < 0) return false;

  x = s.substring(ix + 4).toFloat();
  y = s.substring(iy + 4).toFloat();
  x = clampf_(x, -1.0f, 1.0f);
  y = clampf_(y, -1.0f, 1.0f);
  return true;
}

static char parseFn_(const String &s) {
  int ii = s.indexOf("\"id\"");
  if (ii < 0) return 0;
  int q1 = s.indexOf('"', ii + 4);
  if (q1 < 0) return 0;
  int q2 = s.indexOf('"', q1 + 1);
  if (q2 < 0) return 0;
  if (q2 == q1 + 1) return 0;
  char c = s.charAt(q1 + 1);
  return c;
}

static inline bool changedF_(float a, float b, float eps = 0.01f) {
  return fabsf(a - b) > eps;
}

void debugMarkJoy_(float x, float y) {
  if (changedF_(g_dbgJoyX, x) || changedF_(g_dbgJoyY, y)) {
    g_dbgJoyX = x;
    g_dbgJoyY = y;
    g_dbgJoyDirty = true;
  }
}

void debugMarkWebBtn_(char id) {
  if (g_dbgLastWebBtn != id) {
    g_dbgLastWebBtn = id;
  }
  g_dbgWebBtnDirty = true;
}

void debugPollInputs_() {
  const uint8_t a = readActiveLow_(User_BTN_A);
  const uint8_t b = readActiveLow_(User_BTN_B);
  const uint8_t c = readActiveLow_(User_BTN_C);
  const uint8_t d = readActiveLow_(User_BTN_D);
  const uint8_t sw = readActiveLow_(User_SW);

  if (a != g_dbgBtnA || b != g_dbgBtnB || c != g_dbgBtnC || d != g_dbgBtnD || sw != g_dbgSw) {
    g_dbgBtnA = a;
    g_dbgBtnB = b;
    g_dbgBtnC = c;
    g_dbgBtnD = d;
    g_dbgSw = sw;
    g_dbgInputsDirty = true;
  }
}

void debugPrintIfNeeded_() {
  const uint32_t now = millis();
  if ((now - g_lastDebugPrintMs) < DEBUG_PRINT_PERIOD_MS) return;

  const bool anythingDirty = g_dbgJoyDirty || g_dbgWebBtnDirty || g_dbgInputsDirty;
  if (!anythingDirty) return;

  g_lastDebugPrintMs = now;

  if (g_dbgJoyDirty) {
    Serial.printf("WEB JOY  x=%.2f y=%.2f\n", g_dbgJoyX, g_dbgJoyY);
    g_dbgJoyDirty = false;
  }

  if (g_dbgWebBtnDirty) {
    Serial.printf("WEB BTN  %c\n", g_dbgLastWebBtn ? g_dbgLastWebBtn : '-');
    g_dbgWebBtnDirty = false;
  }

  if (g_dbgInputsDirty) {
    Serial.printf("CORE IN  A=%u B=%u C=%u D=%u SW=%u\n",
                  g_dbgBtnA, g_dbgBtnB, g_dbgBtnC, g_dbgBtnD, g_dbgSw);
    g_dbgInputsDirty = false;
  }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    g_joyX = 0.0f;
    g_joyY = 0.0f;
    g_lastCmdMs = 0;
    Motor_STOP();
    LED_Stop();
    return;
  }

  if (type == WS_EVT_DISCONNECT) {
    g_joyX = 0.0f;
    g_joyY = 0.0f;
    g_lastCmdMs = 0;
    Motor_STOP();
    LED_Ready();
    return;
  }

  if (type != WS_EVT_DATA) return;

  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (!info->final || info->index != 0 || info->len != len) return;
  if (info->opcode != WS_TEXT) return;

  String msg;
  msg.reserve(len + 1);
  for (size_t i = 0; i < len; i++) msg += (char)data[i];

  if (msg.indexOf("\"type\":\"joy\"") >= 0) {
    float x, y;
    if (parseJoy_(msg, x, y)) {
      g_joyX = x;
      g_joyY = y;
      g_lastCmdMs = millis();
      debugMarkJoy_(x, y);
    }
    return;
  }

  if (msg.indexOf("\"type\":\"fn\"") >= 0) {
    char id = parseFn_(msg);
    g_lastCmdMs = millis();
    debugMarkWebBtn_(id);
    LED_Fn();
    switch (id) {
      case 'A': functionA(); break;
      case 'B': functionB(); break;
      case 'C': functionC(); break;
      case 'D': functionD(); break;
      default: break;
    }
    return;
  }
}

// ------------------------------
// Wi-Fi AP setup
// ------------------------------
void wifiStartAP_() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GW, AP_SN);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP:   ");
  Serial.println(WiFi.softAPIP());
}

// ------------------------------
// Server routes
// ------------------------------
void serverStart_() {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ok");
  });

  server.begin();
  Serial.println("HTTP server started");
}

// ------------------------------
// Setup
// ------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(FW_VERSION);

  pinMode(User_BTN_A, INPUT);
  pinMode(User_BTN_B, INPUT);
  pinMode(User_BTN_C, INPUT);
  pinMode(User_BTN_D, INPUT);
  pinMode(User_SW, INPUT);

  analogReadResolution(12);
  pinMode(VIN_SENSE, INPUT);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  LED_Ready();
  serviceLed_();

  Serial.println("Motors init...");
  for (int s = 1; s <= 11; ++s) ConfigureMotorOutput(s, MG90_CR, 90);
  ConfigureMotorOutput(12, MG90_CR, 90);
  Motor_STOP();

  wifiStartAP_();
  serverStart_();

  LED_Ready();
  serviceLed_();

  Serial.println("Ready.");
}

// ------------------------------
// Main loop
// ------------------------------
void loop() {
  ws.cleanupClients();

  const uint32_t now = millis();

  debugPollInputs_();

  serviceDriveFromLatestJoy_();

  if (now - g_lastTelPushMs >= TEL_PERIOD_MS) {
    g_lastTelPushMs = now;
    pushTelemetry_();
  }

  debugPrintIfNeeded_();

  serviceLed_();
}