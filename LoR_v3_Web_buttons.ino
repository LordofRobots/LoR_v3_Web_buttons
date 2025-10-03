/*
================================================================================
  LORD of ROBOTS — LoR_Core_WebInterface (Merged)
  Purpose: Self-hosted Wi-Fi AP + HTTP control UI for MiniBot (Servo drive + LEDs)
  Version: AUG 27, 2025  |  File: LoR_Core_WebInterface.ino
================================================================================

SETUP / CONNECTION
• Power the LoR Core V3. The ESP32 starts a Wi-Fi Access Point:
    SSID: "MiniBot_v3"        (change in `ssid`)
    PASS: "password"          (change in `password`, ≥8 chars)
    IP:   10.0.0.1            (static; set in `WifiSetup`)
• Optional mDNS: "robot.local" (depends on client OS resolver)
• Connect your phone/PC to the AP, then open:
    http://10.0.0.1           → Control web UI

WEB UI / USER CONTROLS
• Drive pad: Forward, Left, Right, Back, Stop.
• Speed toggle: Low / High. Default = Low. Affects drive output percentage.
• Hotkeys (desktop):
    Arrow keys or WASD → Drive
    L/H                → Low/High speed
    1/2/3/4 or Numpad 1/2/3/4 → A/B/C/D function buttons
• Function buttons: A..D placeholders call functionA()..functionD() (stub hooks).

FIRMWARE ARCHITECTURE
• Networking: Wi-Fi AP with fixed IP + HTTP server (ESP-IDF httpd).
• Routes:
    GET "/"               → serves the HTML UI
    GET "/action?go=CMD"  → executes command (forward/left/right/back/stop/high/low/A..D)
• LEDs: FastLED on WS2812B, LED_PIN=33, LED_COUNT=4. Simple color cues for state.
• Drive: ESP32Servo on IO_PINS[], 90° = neutral/stop. API accepts percent [-100..100],
  mapped to [0..180]. Left group = slots 1..6, Right group = slots 7..12.
• Motor profiles: ConfigureMotorOutput() selects timing/pulse limits per device type.


================================================================================
*/

#include "soc/soc.h"           // Kept from original for brownout register access
#include "soc/rtc_cntl_reg.h"  //   "
#include "esp_http_server.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>

#include <ESP32Servo.h>     // Servo-based motor layer (replaces H-bridge PWM)
#include <FastLED.h>        // LED layer (replaces Adafruit_NeoPixel)

// -----------------------------------------------------------------------------
// Version string (printed at boot)
// -----------------------------------------------------------------------------
String Version = "LoR Core Web Interface (Merged) - AUG 27, 2025";

// -----------------------------------------------------------------------------
// === User-configurable parameters ===
// -----------------------------------------------------------------------------
const String ssid     = "MiniBot_v3";   // AP SSID
const String password = "password";     // AP password (≥8 chars)

// Drive speed presets. UI toggles between Low/High.
int highSpeed  = 90;                    // percent of full-scale
int lowSpeed   = 60;
int driveSpeed = lowSpeed;              // current selection

// -----------------------------------------------------------------------------
// === LoR Core V3 pinout fragments (IO/AUX/User inputs/LED) ===
// - Slots are 1-indexed in IO_PINS for readability against docs
// -----------------------------------------------------------------------------
const uint8_t AUX_PINS[9] = { 0, 5, 18, 23, 19, 22, 21, 1, 3 };       // [1..8]
const uint8_t IO_PINS[13] = { 0, 32,25,26,27,14,12,13,15, 2, 4,22,21 };// [1..12]

// User inputs (not used in handlers yet; available for expansion)
#define User_BTN_A 35
#define User_BTN_B 39
#define User_BTN_C 38
#define User_BTN_D 37
#define User_SW    36

// VIN sense (unused here; scale retained)
#define VIN_SENSE   34
#define VOLT_SLOPE  0.0063492
#define VOLT_OFFSET 1.079

// ---- LED (FastLED) ----
#define LED_PIN     33
#define LED_COUNT   4
#define BRIGHTNESS  255
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
CRGB leds[LED_COUNT];

// -----------------------------------------------------------------------------
// === Motor model (ESP32Servo) — 90° = stop/neutral ===
// -----------------------------------------------------------------------------
Servo MotorOutput[13];  // index 1..12 are valid slots

// Motor profiles for attach() timing and range
enum MotorType {
  MG90_CR, MG90_Degree, N20Plus, STD_SERVO, Victor_SPX, Talon_SRX, CUSTOM
};

struct MotorTypeConfig {
  MotorType type;
  float     pwmFreq;     // Hz
  int       minPulseUs;  // µs
  int       maxPulseUs;  // µs
  float     inputMin;    // reserved
  float     inputMax;    // reserved
};

// Tunable presets (adjust for your hardware if needed)
MotorTypeConfig motorTypeConfigs[] = {
  { MG90_CR,     50,  500, 2500, -1, 1 },
  { MG90_Degree, 50,  500, 2500,  1,180},
  { N20Plus,     50, 1000, 2000, -1, 1 },
  { Victor_SPX,  50, 1000, 2000, -1, 1 },
  { Talon_SRX,   50, 1000, 2000, -1, 1 },
  { STD_SERVO,   50, 1000, 2000,  0,180},
};

// Attach a slot to a motor profile and set a startup angle
void ConfigureMotorOutput(uint8_t slot, MotorType motorType, int startupPositionDeg = 90) {
  float pwmFreq   = 50;
  int   minPulseUs= 1000;
  int   maxPulseUs= 2000;

  for (auto &cfg : motorTypeConfigs) {
    if (cfg.type == motorType) {
      pwmFreq    = cfg.pwmFreq;
      minPulseUs = cfg.minPulseUs;
      maxPulseUs = cfg.maxPulseUs;
      break;
    }
  }

  const uint8_t pin = IO_PINS[slot];
  pinMode(pin, OUTPUT);
  MotorOutput[slot].setPeriodHertz(pwmFreq);
  MotorOutput[slot].attach(pin, minPulseUs, maxPulseUs);
  MotorOutput[slot].write(startupPositionDeg);  // neutral by default
}

// -----------------------------------------------------------------------------
// === LED helpers (simple color cues) ===
// -----------------------------------------------------------------------------
inline void LED_SetSolid(uint8_t r, uint8_t g, uint8_t b) {
  fill_solid(leds, LED_COUNT, CRGB(r,g,b));
  FastLED.show();
}
inline void LED_Red()    { LED_SetSolid(255,  0,  0); }  // error/stop/serving UI
inline void LED_Green()  { LED_SetSolid(  0,255,  0); }  // command received
inline void LED_Blue()   { LED_SetSolid(  0,  0,255); }  // booting
inline void LED_White()  { LED_SetSolid(255,255,255); }  // ready
inline void LED_Yellow() { LED_SetSolid(255,255,  0); }  // low speed selected
inline void LED_Cyan()   { LED_SetSolid(  0,255,255); }  // function A..D
inline void LED_Purple() { LED_SetSolid(255,  0,255); }  // high speed selected
inline void LED_Off()    { LED_SetSolid(  0,  0,  0); }

// -----------------------------------------------------------------------------
// === Drive layer (tank mix) ===
// Left group = slots 1..6, Right group = slots 7..12
// Input in percent [-100..+100] → angle [0..180], 90 = stop
// -----------------------------------------------------------------------------
static inline int percentToServoAngle(int pct) {
  pct = constrain(pct, -100, 100);
  const long ang = map(pct, -100, 100, 0, 180);
  return (int)constrain(ang, 0, 180);
}

void Left_Group_Write(int pct) {
  const int angle = percentToServoAngle(pct);
  for (int s = 1; s <= 6; ++s) MotorOutput[s].write(angle);
}

void Right_Group_Write(int pct) {
  const int angle = percentToServoAngle(pct);
  for (int s = 7; s <= 12; ++s) MotorOutput[s].write(angle);
}

void Motor_Control(int LeftPct, int RightPct) {
  Left_Group_Write(LeftPct);
  Right_Group_Write(RightPct);
}

void Motor_STOP() {
  for (int s = 1; s <= 12; ++s) MotorOutput[s].write(90); // neutral
}

// -----------------------------------------------------------------------------
// === Command bindings (called by HTTP /action?go=...) ===
// Semantics match original web foundation
// -----------------------------------------------------------------------------
void functionForward()  { Motor_Control( -driveSpeed, +driveSpeed ); }
void functionBackward() { Motor_Control( +driveSpeed, -driveSpeed ); }
void functionLeft()     { Motor_Control( -driveSpeed, -driveSpeed ); }
void functionRight()    { Motor_Control( +driveSpeed, +driveSpeed ); }
void functionStop()     { Motor_STOP(); }

// User-extendable function hooks
void functionA() { /* custom action */ }
void functionB() { /* custom action */ }
void functionC() { /* custom action */ }
void functionD() { /* custom action */ }

// -----------------------------------------------------------------------------
// === Embedded Web Page (served at "/") ===
// -----------------------------------------------------------------------------
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>LORD of ROBOTS</title>
    <meta name="viewport" content="width=device-width, height=device-height, initial-scale=1.0, user-scalable=no">
    <style>
      body { text-align:center; margin:0 auto; padding-top:30px; background-color:#001336; }
      h1 { font-family:Monospace; color:white; margin:10px auto 30px; }
      h3 { font-family:Monospace; color:#b3c1db; margin-bottom:10px; font-style:italic; }
      .button {
        background-color:#b3c1db; width:110px; height:75px; color:#001336; font-size:20px; font-weight:bold;
        text-align:center; border-radius:5px; border:3px solid; border-color:white; display:inline-block;
        margin:6px 6px; cursor:pointer; -webkit-tap-highlight-color:rgba(0,0,0,0);
        -webkit-user-select:none; -moz-user-select:none; -ms-user-select:none; user-select:none;
      }
      .button:active { background-color:#001336; color:#ccd6e7; border:3px solid white; }
      .slider {
        position:relative; display:inline-block; -webkit-tap-highlight-color:transparent; vertical-align:top;
        cursor:pointer; width:100px; height:40px; border-radius:50px; background-color:#001336; border:3px solid white;
      }
      .slider:before {
        position:absolute; content:"Low"; font-style:italic; font-size:14px; font-weight:bold; color:#001336;
        line-height:30px; vertical-align:middle; border-radius:50px; height:30px; width:50px; left:5px; bottom:5px;
        background-color:#ccd6e7; -webkit-transition:.4s; transition:.4s;
      }
      .switch input { opacity:0; width:0; height:0; }
      input:checked+.slider:before { content:'High'; transform:translateX(40px); }
      #buttons { display:inline-block; text-align:center; }
      .emptySpace { width:50px; height:10px; display:inline-block; margin:6px 6px; }
    </style>
  </head>
  <body oncontextmenu="return false;">
    <h3>LORD of ROBOTS</h3>
    <h1>MiniBot Control Interface</h1>
    <div id="buttons" style="margin-bottom:20px;">
      <div class="emptySpace" style="width:110px"></div>
      <div class="emptySpace" style="width:110px"></div>
      <div class="emptySpace" style="color:white; width:110px; vertical-align:top; text-align:center; margin-bottom:15px;">Drive Speed</div>
      <br>
      <div class="emptySpace" style="width:105px"></div>
      <button class="button" onpointerdown="sendData('forward')" onpointerup="releaseData()" id="forward-button">Forward</button>
      <label class="switch">
        <input type="checkbox" id="toggle-switch"><span class="slider"></span>
      </label><br>
      <button class="button" onpointerdown="sendData('left')" onpointerup="releaseData()" id="left-button">Left</button>
      <button class="button" onpointerdown="sendData('stop')" onpointerup="releaseData()" id="stop-button">Stop</button>
      <button class="button" onpointerdown="sendData('right')" onpointerup="releaseData()" id="right-button">Right</button><br>
      <button class="button" onpointerdown="sendData('backward')" onpointerup="releaseData()" id="backward-button">Back</button>
    </div>
    <div class="emptySpace" style="width:150px; height:30px"></div>
    <div id="buttons" style="vertical-align:50px">
      <button class="button" onpointerdown="sendData('functionA')" onpointerup="releaseData()" id="functionA">A</button>
      <button class="button" onpointerdown="sendData('functionB')" onpointerup="releaseData()" id="functionB">B</button><br>
      <button class="button" onpointerdown="sendData('functionC')" onpointerup="releaseData()" id="functionC">C</button>
      <button class="button" onpointerdown="sendData('functionD')" onpointerup="releaseData()" id="functionD">D</button>
    </div>
    <script>
      // Simple AJAX helpers: send a command and auto-stop on release
      var isButtonPressed = false;
      function sendData(x){ var xhr=new XMLHttpRequest(); xhr.open("GET","/action?go="+x,true); xhr.send(); }
      function releaseData(){ isButtonPressed=false; sendData('stop'); }

      // Keyboard map for desktop control
      const keyMap = {
        'ArrowUp':'forward','ArrowLeft':'left','ArrowDown':'backward','ArrowRight':'right',
        'KeyW':'forward','KeyA':'left','KeyS':'backward','KeyD':'right',
        'KeyL':'low','KeyH':'high','Digit1':'functionA','Digit2':'functionB','Digit3':'functionC','Digit4':'functionD',
        'Numpad1':'functionA','Numpad2':'functionB','Numpad3':'functionC','Numpad4':'functionD',
      };

      // Prevent repeat spamming; one down → one command
      document.addEventListener('keydown', e=>{
        if(!isButtonPressed){ const a=keyMap[e.code]; if(a) sendData(a); isButtonPressed=true; }
      });
      document.addEventListener('keyup', e=>{ releaseData(); });

      // Speed toggle → low/high
      const toggleSwitch=document.getElementById("toggle-switch");
      toggleSwitch.addEventListener("change", function(){ sendData(toggleSwitch.checked?'high':'low'); });
    </script>
  </body>
</html>
)rawliteral";

// -----------------------------------------------------------------------------
// === HTTP Server handlers ===
// -----------------------------------------------------------------------------
httpd_handle_t Robot_httpd = NULL;

// GET "/" → serve UI. Red LED while serving page payload.
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  LED_Red();
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// GET "/action?go=CMD" → parse and execute command
String speed = "low";
static esp_err_t cmd_handler(httpd_req_t *req) {
  char buf[128];
  char variable[32] = {0};

  // Parse query string: expect key "go"
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) != ESP_OK ||
      httpd_query_key_value(buf, "go", variable, sizeof(variable)) != ESP_OK) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  int res = 0;
  LED_Green();  // indicate command activity

  // Command decoder
  if (!strcmp(variable, "high")) {
    Serial.println("High Speed");
    driveSpeed = highSpeed; speed = "High"; LED_Purple();
  } else if (!strcmp(variable, "low")) {
    Serial.println("Low Speed");
    driveSpeed = lowSpeed;  speed = "Low";  LED_Yellow();
  } else if (!strcmp(variable, "forward")) {
    Serial.println("Forward " + speed); functionForward();
  } else if (!strcmp(variable, "left")) {
    Serial.println("Left " + speed);    functionLeft();
  } else if (!strcmp(variable, "right")) {
    Serial.println("Right " + speed);   functionRight();
  } else if (!strcmp(variable, "backward")) {
    Serial.println("Backward " + speed);functionBackward();
  } else if (!strcmp(variable, "stop")) {
    Serial.println("Stop"); functionStop(); LED_Red();
  } else if (!strcmp(variable, "functionA")) {
    Serial.println("Function A"); LED_Cyan(); functionA();
  } else if (!strcmp(variable, "functionB")) {
    Serial.println("Function B"); LED_Cyan(); functionB();
  } else if (!strcmp(variable, "functionC")) {
    Serial.println("Function C"); LED_Cyan(); functionC();
  } else if (!strcmp(variable, "functionD")) {
    Serial.println("Function D"); LED_Cyan(); functionD();
  } else {
    // Unknown command → fail safe to STOP
    Serial.println("Stop (unknown cmd)");
    LED_Red(); Motor_STOP(); res = -1;
  }

  if (res) return httpd_resp_send_500(req);
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);  // 200 OK, empty body
}

// Start the embedded HTTP server and register routes
void startServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.stack_size  = 8192;

  httpd_uri_t index_uri = { .uri="/",       .method=HTTP_GET, .handler=index_handler, .user_ctx=NULL };
  httpd_uri_t cmd_uri   = { .uri="/action", .method=HTTP_GET, .handler=cmd_handler,   .user_ctx=NULL };

  if (httpd_start(&Robot_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(Robot_httpd, &index_uri);
    httpd_register_uri_handler(Robot_httpd, &cmd_uri);
  }
  // Bump ports in config object copy (mirrors IDF template behavior)
  config.server_port += 1;
  config.ctrl_port   += 1;
}

// -----------------------------------------------------------------------------
// === Wi-Fi AP setup (static IP 10.0.0.1) ===
// -----------------------------------------------------------------------------
IPAddress local_ip(10,0,0,1), gateway(10,0,0,1), subnet(255,255,255,0);

void WifiSetup() {
  WiFi.mode(WIFI_AP);

  // Configure AP IP BEFORE starting the AP
  WiFi.softAPConfig(local_ip, gateway, subnet);

  // Start AP with SSID/PASS (channel/hidden/max_conn optional)
  WiFi.softAP(ssid.c_str(), password.c_str());

  // Print assigned AP IP
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // expected 10.0.0.1

  // mDNS (optional): http://robot.local
  if (!MDNS.begin("robot")) Serial.println("Error setting up MDNS responder!");
  MDNS.addService("http", "tcp", 80);

  Serial.println("WiFi start");
}

// -----------------------------------------------------------------------------
// === setup() — initialize peripherals, network, and server ===
// -----------------------------------------------------------------------------
void setup() {
  // Disable brownout detector (matches original foundation behavior)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(200);
  Serial.println("Serial Begin");

  // Configure inputs (available for future features)
  pinMode(User_BTN_A, INPUT);
  pinMode(User_BTN_B, INPUT);
  pinMode(User_BTN_C, INPUT);
  pinMode(User_BTN_D, INPUT);
  pinMode(User_SW,    INPUT);
  pinMode(VIN_SENSE,  INPUT);

  // LEDs
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  LED_Blue();  // boot indicator

  // Attach all motor slots to profiles; adjust per-mechatronics as needed
  Serial.println("Motors Startup");
  for (int s = 1; s <= 11; ++s) ConfigureMotorOutput(s, N20Plus, 90);
  ConfigureMotorOutput(12, MG90_CR, 90);

  // Network + HTTP
  WifiSetup();
  startServer();

  LED_White();
  Serial.println("MiniBot System Ready! Version = " + Version);
}

// -----------------------------------------------------------------------------
// === loop() — event-driven via HTTP; idle here ===
// -----------------------------------------------------------------------------
void loop() {
  // No periodic work required. HTTP handlers perform all actions.
}
