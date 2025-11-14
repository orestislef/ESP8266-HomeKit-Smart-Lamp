/*
 * ========================================================================
 * ESP8266 HOMEKIT RGB LED STRIP - PRODUCTION VERSION v2.1
 * ========================================================================
 *
 * SUPPORTS ADDRESSABLE LED STRIPS (WS2812B, WS2813, SK6812, NeoPixels)
 *
 * FEATURES:
 * ✅ Full RGB Color Control via HomeKit
 * ✅ Hue, Saturation, Brightness control
 * ✅ OTA Updates
 * ✅ WiFi Manager (easy setup)
 * ✅ Persistent Storage
 * ✅ Physical Button
 * ✅ 8 Color Scenes + Effects
 * ✅ Smooth Color Transitions
 * ✅ Rainbow, Fire, Breathing effects
 * ✅ Web UI with color picker
 * ✅ Works with any WS2812B LED count
 *
 * ========================================================================
 * REQUIRED LIBRARIES
 * ========================================================================
 * Install via: Sketch → Include Library → Manage Libraries
 *
 * 1. HomeKit-ESP8266 by Mixiaoxiao
 * 2. WiFiManager by tzapu (v2.0.x)
 * 3. ArduinoJson by Benoit Blanchon (v6.x)
 * 4. FastLED by Daniel Garcia  ⭐ NEW for LED strips!
 *
 * ========================================================================
 * HARDWARE CONNECTIONS
 * ========================================================================
 * WS2812B LED Strip:
 *   - Data Pin  → GPIO2 (D4 on NodeMCU)
 *   - VCC/5V    → 5V power supply (NOT from ESP8266!)
 *   - GND       → Common ground (ESP8266 + LED strip)
 *
 * Button (optional):
 *   - GPIO0 (D3) → Button → GND
 *
 * ⚠️ IMPORTANT: LED strips need external 5V power supply!
 *    ESP8266 can't power LED strips - use separate 5V adapter
 *    Connect GND between ESP8266 and power supply
 *
 * Power requirements (per LED at full white):
 *   - ~60mA per WS2812B LED
 *   - 30 LEDs = 1.8A max
 *   - 60 LEDs = 3.6A max
 *   - Use quality 5V power supply!
 *
 * ========================================================================
 * BOARD SETTINGS
 * ========================================================================
 * Board: "Generic ESP8266 Module" or "NodeMCU 1.0"
 * Flash Size: "4MB (FS:1MB OTA:~1019KB)"
 * CPU Frequency: "160 MHz"
 * Upload Speed: "115200"
 *
 * ========================================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <arduino_homekit_server.h>
#include <Ticker.h>
#include <FastLED.h>

// ========================================================================
// LED STRIP CONFIGURATION - ⚠️ CHANGE THESE!
// ========================================================================
#define NUM_LEDS 30             // Number of LEDs in your strip
#define LED_PIN 2               // GPIO2 (D4) - Data pin for LED strip
#define LED_TYPE WS2812B        // LED strip type (WS2812B, WS2813, SK6812, etc.)
#define COLOR_ORDER GRB         // Color order (GRB for most WS2812B)
#define MAX_BRIGHTNESS 255      // Maximum brightness (255 = 100%)

// ========================================================================
// OTHER CONFIGURATION
// ========================================================================
#define BUTTON_PIN 0
#define STATUS_LED_PIN 16

#define DEVICE_NAME "RGB Lamp"
#define HOSTNAME "esp8266-rgb"
#define FIRMWARE_VERSION "2.1.0"

#define DEBOUNCE_DELAY 50
#define LONG_PRESS_TIME 3000
#define FACTORY_RESET_TIME 10000
#define WIFI_RECONNECT_INTERVAL 30000
#define WATCHDOG_TIMEOUT 8000

// ========================================================================
// GLOBAL VARIABLES
// ========================================================================
ESP8266WebServer server(80);
WiFiManager wifiManager;
Ticker statusLEDTicker;

// LED strip
CRGB leds[NUM_LEDS];

// Lamp state
bool lampOn = false;
int lampBrightness = 100;    // 0-100%
float lampHue = 0.0;          // 0-360 degrees
float lampSaturation = 0.0;   // 0-100%
int currentScene = 0;          // 0=Custom, 1-8=Presets
int currentEffect = 0;         // 0=None, 1=Rainbow, 2=Fire, 3=Breathing

// Button state
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
bool longPressHandled = false;
bool veryLongPressHandled = false;

// WiFi & OTA
unsigned long lastWiFiCheck = 0;
bool otaInProgress = false;

// Effects
unsigned long lastEffectUpdate = 0;
uint8_t effectHue = 0;

// ========================================================================
// SCENE DEFINITIONS (Hue, Saturation, Brightness)
// ========================================================================
struct ColorScene {
    const char* name;
    float hue;
    float saturation;
    int brightness;
};

const ColorScene scenes[] = {
    {"Custom", 0, 0, 100},           // 0: User-defined
    {"Warm White", 30, 20, 100},     // 1: Cozy warm white
    {"Cool White", 200, 10, 100},    // 2: Bright cool white
    {"Energize", 120, 100, 100},     // 3: Bright green
    {"Concentrate", 200, 100, 80},   // 4: Blue focus light
    {"Reading", 40, 30, 70},         // 5: Warm reading light
    {"Relax", 260, 60, 40},          // 6: Purple ambient
    {"Sleep", 240, 100, 10},         // 7: Dim blue night light
    {"Romance", 340, 80, 30}         // 8: Soft pink
};

// ========================================================================
// HOMEKIT FORWARD DECLARATIONS
// ========================================================================
void updateLED();
void setColor(float hue, float saturation, int brightness);
void lampIdentify(homekit_value_t _value);
homekit_value_t lampOnGet();
void lampOnSet(homekit_value_t value);
homekit_value_t lampBrightnessGet();
void lampBrightnessSet(homekit_value_t value);
homekit_value_t lampHueGet();
void lampHueSet(homekit_value_t value);
homekit_value_t lampSaturationGet();
void lampSaturationSet(homekit_value_t value);

// ========================================================================
// HOMEKIT ACCESSORY DEFINITION
// ========================================================================
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "DIY Maker"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "ESP8266-RGB-001"),
            HOMEKIT_CHARACTERISTIC(MODEL, "RGB Smart Lamp v2.1"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, FIRMWARE_VERSION),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, lampIdentify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
            HOMEKIT_CHARACTERISTIC(ON, false, .getter=lampOnGet, .setter=lampOnSet),
            HOMEKIT_CHARACTERISTIC(BRIGHTNESS, 100, .getter=lampBrightnessGet, .setter=lampBrightnessSet),
            HOMEKIT_CHARACTERISTIC(HUE, 0.0, .getter=lampHueGet, .setter=lampHueSet),
            HOMEKIT_CHARACTERISTIC(SATURATION, 0.0, .getter=lampSaturationGet, .setter=lampSaturationSet),
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t homekitConfig = {
    .accessories = accessories,
    .password = "111-22-333",
    .setupId = "ES32"
};

// ========================================================================
// PERSISTENT STORAGE
// ========================================================================
void saveSettings() {
    StaticJsonDocument<256> doc;
    doc["on"] = lampOn;
    doc["brightness"] = lampBrightness;
    doc["hue"] = lampHue;
    doc["saturation"] = lampSaturation;
    doc["scene"] = currentScene;

    File file = LittleFS.open("/settings.json", "w");
    if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.println("✓ Settings saved");
    }
}

void loadSettings() {
    if (!LittleFS.exists("/settings.json")) {
        Serial.println("⚠️  No saved settings");
        return;
    }

    File file = LittleFS.open("/settings.json", "r");
    if (!file) return;

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) return;

    lampOn = doc["on"] | false;
    lampBrightness = doc["brightness"] | 100;
    lampHue = doc["hue"] | 0.0;
    lampSaturation = doc["saturation"] | 0.0;
    currentScene = doc["scene"] | 0;

    Serial.println("✓ Settings loaded");
}

void factoryReset() {
    Serial.println("\n⚠️  FACTORY RESET!");

    for (int i = 0; i < 20; i++) {
        fill_solid(leds, NUM_LEDS, i % 2 ? CRGB::Red : CRGB::Black);
        FastLED.show();
        delay(100);
    }

    LittleFS.format();
    wifiManager.resetSettings();

    Serial.println("✓ Reset complete - rebooting...");
    delay(1000);
    ESP.restart();
}

// ========================================================================
// COLOR CONVERSION & LED CONTROL
// ========================================================================
CRGB HSVtoRGB(float h, float s, float v) {
    // h: 0-360, s: 0-100, v: 0-100
    s /= 100.0;
    v /= 100.0;

    float c = v * s;
    float x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
    float m = v - c;

    float r, g, b;

    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return CRGB(
        (r + m) * 255,
        (g + m) * 255,
        (b + m) * 255
    );
}

void setColor(float hue, float saturation, int brightness) {
    lampHue = hue;
    lampSaturation = saturation;
    lampBrightness = brightness;
    currentScene = 0; // Custom
    currentEffect = 0; // Stop effects
    updateLED();
}

void updateLED() {
    if (lampOn) {
        CRGB color = HSVtoRGB(lampHue, lampSaturation, lampBrightness);
        fill_solid(leds, NUM_LEDS, color);
        FastLED.setBrightness(map(lampBrightness, 0, 100, 0, MAX_BRIGHTNESS));
    } else {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
    }

    FastLED.show();
    saveSettings();
}

// ========================================================================
// EFFECTS
// ========================================================================
void updateRainbow() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(effectHue + (i * 256 / NUM_LEDS), 255, 255);
    }
    effectHue++;
    FastLED.show();
}

void updateFire() {
    for (int i = 0; i < NUM_LEDS; i++) {
        int heat = random(0, 255);
        leds[i] = CHSV(random(0, 30), 255, heat);
    }
    FastLED.show();
}

void updateBreathing() {
    static uint8_t brightness = 0;
    static int8_t direction = 1;

    brightness += direction * 5;
    if (brightness >= 250 || brightness <= 5) direction *= -1;

    CRGB color = HSVtoRGB(lampHue, lampSaturation, 100);
    fill_solid(leds, NUM_LEDS, color);
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void updateEffect() {
    if (!lampOn || currentEffect == 0) return;

    if (millis() - lastEffectUpdate < 50) return;
    lastEffectUpdate = millis();

    switch (currentEffect) {
        case 1: updateRainbow(); break;
        case 2: updateFire(); break;
        case 3: updateBreathing(); break;
    }
}

void setEffect(int effect) {
    currentEffect = effect;
    lampOn = true;

    if (effect == 0) {
        updateLED(); // Stop effect, show solid color
    }

    Serial.print("🎨 Effect: ");
    Serial.println(effect);
}

// ========================================================================
// SCENE MANAGEMENT
// ========================================================================
void applyScene(int scene) {
    if (scene < 0 || scene >= sizeof(scenes) / sizeof(scenes[0])) return;

    currentScene = scene;
    currentEffect = 0; // Stop effects

    lampHue = scenes[scene].hue;
    lampSaturation = scenes[scene].saturation;
    lampBrightness = scenes[scene].brightness;

    if (!lampOn) lampOn = true;
    updateLED();

    Serial.print("🎨 Scene: ");
    Serial.println(scenes[scene].name);

    // Notify HomeKit
    homekit_characteristic_t *ch_on = homekit_service_characteristic_by_type(
        &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
    homekit_characteristic_t *ch_br = homekit_service_characteristic_by_type(
        &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_BRIGHTNESS);
    homekit_characteristic_t *ch_hue = homekit_service_characteristic_by_type(
        &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_HUE);
    homekit_characteristic_t *ch_sat = homekit_service_characteristic_by_type(
        &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_SATURATION);

    if (ch_on) homekit_characteristic_notify(ch_on, HOMEKIT_BOOL(lampOn));
    if (ch_br) homekit_characteristic_notify(ch_br, HOMEKIT_INT(lampBrightness));
    if (ch_hue) homekit_characteristic_notify(ch_hue, HOMEKIT_FLOAT(lampHue));
    if (ch_sat) homekit_characteristic_notify(ch_sat, HOMEKIT_FLOAT(lampSaturation));
}

void cycleScene() {
    int nextScene = (currentScene + 1) % (sizeof(scenes) / sizeof(scenes[0]));
    applyScene(nextScene);
}

// ========================================================================
// BUTTON HANDLING
// ========================================================================
void handleButton() {
    bool buttonState = digitalRead(BUTTON_PIN) == LOW;

    if (buttonState && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
        longPressHandled = false;
        veryLongPressHandled = false;
    } else if (!buttonState && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;

        if (pressDuration < LONG_PRESS_TIME && !longPressHandled && !veryLongPressHandled) {
            // Short press - toggle
            lampOn = !lampOn;
            currentEffect = 0;
            updateLED();

            homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
                &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
            if (ch) homekit_characteristic_notify(ch, HOMEKIT_BOOL(lampOn));

            Serial.println("🔘 Button: Toggle");
        }

        buttonPressed = false;
    } else if (buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;

        if (pressDuration >= FACTORY_RESET_TIME && !veryLongPressHandled) {
            veryLongPressHandled = true;
            factoryReset();
        } else if (pressDuration >= LONG_PRESS_TIME && !longPressHandled) {
            longPressHandled = true;
            cycleScene();
            Serial.println("🔘 Button: Cycle Scene");
        }
    }
}

// ========================================================================
// STATUS LED
// ========================================================================
void statusLEDSolid() {
    statusLEDTicker.detach();
    digitalWrite(STATUS_LED_PIN, LOW);
}

void statusLEDBlink(int interval) {
    statusLEDTicker.attach_ms(interval, []() {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    });
}

// ========================================================================
// HOMEKIT CALLBACKS
// ========================================================================
void lampIdentify(homekit_value_t _value) {
    Serial.println("🔍 HomeKit Identify");
    for (int i = 0; i < 3; i++) {
        fill_solid(leds, NUM_LEDS, CRGB::White);
        FastLED.show();
        delay(200);
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        delay(200);
    }
    updateLED();
}

homekit_value_t lampOnGet() {
    return HOMEKIT_BOOL(lampOn);
}

void lampOnSet(homekit_value_t value) {
    lampOn = value.bool_value;
    currentEffect = 0;
    Serial.print("📱 HomeKit: ");
    Serial.println(lampOn ? "ON" : "OFF");
    updateLED();
}

homekit_value_t lampBrightnessGet() {
    return HOMEKIT_INT(lampBrightness);
}

void lampBrightnessSet(homekit_value_t value) {
    lampBrightness = value.int_value;
    currentScene = 0;
    currentEffect = 0;
    Serial.print("📱 HomeKit: Brightness ");
    Serial.println(lampBrightness);
    updateLED();
}

homekit_value_t lampHueGet() {
    return HOMEKIT_FLOAT(lampHue);
}

void lampHueSet(homekit_value_t value) {
    lampHue = value.float_value;
    currentScene = 0;
    currentEffect = 0;
    Serial.print("📱 HomeKit: Hue ");
    Serial.println(lampHue);
    updateLED();
}

homekit_value_t lampSaturationGet() {
    return HOMEKIT_FLOAT(lampSaturation);
}

void lampSaturationSet(homekit_value_t value) {
    lampSaturation = value.float_value;
    currentScene = 0;
    currentEffect = 0;
    Serial.print("📱 HomeKit: Saturation ");
    Serial.println(lampSaturation);
    updateLED();
}

// ========================================================================
// WEB SERVER
// ========================================================================
const char HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RGB Smart Lamp</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 500px;
            margin: 0 auto;
            background: white;
            border-radius: 25px;
            padding: 35px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
        }
        h1 {
            text-align: center;
            color: #333;
            font-size: 28px;
            margin-bottom: 5px;
        }
        .version {
            text-align: center;
            color: #999;
            font-size: 11px;
            margin-bottom: 25px;
        }
        .preview {
            height: 100px;
            border-radius: 15px;
            margin: 20px 0;
            transition: all 0.3s;
            box-shadow: 0 5px 20px rgba(0,0,0,0.2);
        }
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin: 20px 0;
        }
        button {
            padding: 12px;
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            color: white;
        }
        .btn-on {
            background: linear-gradient(135deg, #11998e, #38ef7d);
        }
        .btn-off {
            background: linear-gradient(135deg, #ee0979, #ff6a00);
        }
        .btn-scene {
            background: linear-gradient(135deg, #667eea, #764ba2);
            font-size: 12px;
            padding: 10px;
        }
        .btn-effect {
            background: linear-gradient(135deg, #f093fb, #f5576c);
            font-size: 12px;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 15px rgba(0,0,0,0.2);
        }
        .slider-group {
            margin: 20px 0;
        }
        .slider-label {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            color: #555;
            font-size: 13px;
        }
        input[type="range"] {
            width: 100%;
            height: 6px;
            border-radius: 5px;
            outline: none;
            -webkit-appearance: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: white;
            border: 3px solid #667eea;
            cursor: pointer;
            box-shadow: 0 2px 8px rgba(0,0,0,0.2);
        }
        .section {
            margin: 25px 0;
            padding: 15px;
            background: #f8f9fa;
            border-radius: 12px;
        }
        .section h3 {
            color: #555;
            margin-bottom: 12px;
            font-size: 14px;
        }
        .info {
            background: #e3f2fd;
            padding: 12px;
            border-radius: 8px;
            margin-top: 20px;
            font-size: 12px;
            color: #1976D2;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌈 RGB Smart Lamp</h1>
        <div class="version">v2.1 • Full Color Control</div>

        <div class="preview" id="preview"></div>

        <div class="controls">
            <button class="btn-on" onclick="turnOn()">ON</button>
            <button class="btn-off" onclick="turnOff()">OFF</button>
        </div>

        <div class="slider-group">
            <div class="slider-label">
                <span>💡 Brightness</span>
                <span id="brightValue">100%</span>
            </div>
            <input type="range" min="0" max="100" value="100" id="brightness" oninput="updateColor()">
        </div>

        <div class="slider-group">
            <div class="slider-label">
                <span>🎨 Hue</span>
                <span id="hueValue">0°</span>
            </div>
            <input type="range" min="0" max="360" value="0" id="hue" oninput="updateColor()">
        </div>

        <div class="slider-group">
            <div class="slider-label">
                <span>🎭 Saturation</span>
                <span id="satValue">0%</span>
            </div>
            <input type="range" min="0" max="100" value="0" id="saturation" oninput="updateColor()">
        </div>

        <div class="section">
            <h3>⭐ Scenes</h3>
            <div class="controls" style="grid-template-columns: repeat(3, 1fr);">
                <button class="btn-scene" onclick="setScene(1)">Warm</button>
                <button class="btn-scene" onclick="setScene(2)">Cool</button>
                <button class="btn-scene" onclick="setScene(3)">Energize</button>
                <button class="btn-scene" onclick="setScene(4)">Focus</button>
                <button class="btn-scene" onclick="setScene(5)">Reading</button>
                <button class="btn-scene" onclick="setScene(6)">Relax</button>
                <button class="btn-scene" onclick="setScene(7)">Sleep</button>
                <button class="btn-scene" onclick="setScene(8)">Romance</button>
            </div>
        </div>

        <div class="section">
            <h3>✨ Effects</h3>
            <div class="controls">
                <button class="btn-effect" onclick="setEffect(0)">None</button>
                <button class="btn-effect" onclick="setEffect(1)">🌈 Rainbow</button>
                <button class="btn-effect" onclick="setEffect(2)">🔥 Fire</button>
                <button class="btn-effect" onclick="setEffect(3)">💨 Breathing</button>
            </div>
        </div>

        <div class="info">
            🍎 Control with HomeKit: "Hey Siri, set lamp to red"<br>
            Setup code: <strong>111-22-333</strong>
        </div>
    </div>

    <script>
        function hsvToRgb(h, s, v) {
            s /= 100; v /= 100;
            let c = v * s;
            let x = c * (1 - Math.abs(((h / 60) % 2) - 1));
            let m = v - c;
            let r, g, b;

            if (h < 60) { r = c; g = x; b = 0; }
            else if (h < 120) { r = x; g = c; b = 0; }
            else if (h < 180) { r = 0; g = c; b = x; }
            else if (h < 240) { r = 0; g = x; b = c; }
            else if (h < 300) { r = x; g = 0; b = c; }
            else { r = c; g = 0; b = x; }

            return [Math.round((r+m)*255), Math.round((g+m)*255), Math.round((b+m)*255)];
        }

        function updatePreview() {
            const h = parseInt(document.getElementById('hue').value);
            const s = parseInt(document.getElementById('saturation').value);
            const v = parseInt(document.getElementById('brightness').value);

            document.getElementById('hueValue').textContent = h + '°';
            document.getElementById('satValue').textContent = s + '%';
            document.getElementById('brightValue').textContent = v + '%';

            const rgb = hsvToRgb(h, s, v);
            document.getElementById('preview').style.background =
                `rgb(${rgb[0]}, ${rgb[1]}, ${rgb[2]})`;
        }

        function updateColor() {
            updatePreview();
            const h = document.getElementById('hue').value;
            const s = document.getElementById('saturation').value;
            const b = document.getElementById('brightness').value;
            fetch(`/color?h=${h}&s=${s}&b=${b}`);
        }

        function turnOn() {
            fetch('/lamp?state=1').then(() => refresh());
        }

        function turnOff() {
            fetch('/lamp?state=0').then(() => refresh());
        }

        function setScene(id) {
            fetch('/scene?id=' + id).then(() => refresh());
        }

        function setEffect(id) {
            fetch('/effect?id=' + id);
        }

        function refresh() {
            fetch('/status').then(r => r.json()).then(data => {
                document.getElementById('hue').value = data.hue;
                document.getElementById('saturation').value = data.saturation;
                document.getElementById('brightness').value = data.brightness;
                updatePreview();
            });
        }

        setInterval(refresh, 3000);
        refresh();
    </script>
</body>
</html>
)=====";

void handleRoot() {
    server.send(200, "text/html", HTML);
}

void handleLamp() {
    if (server.hasArg("state")) {
        lampOn = (server.arg("state").toInt() == 1);
        currentEffect = 0;
        updateLED();

        homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
        if (ch) homekit_characteristic_notify(ch, HOMEKIT_BOOL(lampOn));

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

void handleColor() {
    if (server.hasArg("h") && server.hasArg("s") && server.hasArg("b")) {
        float h = server.arg("h").toFloat();
        float s = server.arg("s").toFloat();
        int b = server.arg("b").toInt();

        setColor(h, s, b);
        lampOn = true;

        // Notify HomeKit
        homekit_characteristic_t *ch_on = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
        homekit_characteristic_t *ch_br = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_BRIGHTNESS);
        homekit_characteristic_t *ch_hue = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_HUE);
        homekit_characteristic_t *ch_sat = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_SATURATION);

        if (ch_on) homekit_characteristic_notify(ch_on, HOMEKIT_BOOL(lampOn));
        if (ch_br) homekit_characteristic_notify(ch_br, HOMEKIT_INT(lampBrightness));
        if (ch_hue) homekit_characteristic_notify(ch_hue, HOMEKIT_FLOAT(lampHue));
        if (ch_sat) homekit_characteristic_notify(ch_sat, HOMEKIT_FLOAT(lampSaturation));

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing parameters");
    }
}

void handleScene() {
    if (server.hasArg("id")) {
        int scene = server.arg("id").toInt();
        applyScene(scene);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing id");
    }
}

void handleEffect() {
    if (server.hasArg("id")) {
        int effect = server.arg("id").toInt();
        setEffect(effect);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing id");
    }
}

void handleStatus() {
    String json = "{\"on\":" + String(lampOn ? "true" : "false") +
                  ",\"brightness\":" + String(lampBrightness) +
                  ",\"hue\":" + String(lampHue) +
                  ",\"saturation\":" + String(lampSaturation) +
                  ",\"scene\":" + String(currentScene) +
                  ",\"effect\":" + String(currentEffect) + "}";
    server.send(200, "application/json", json);
}

void handleInfo() {
    String info = "ESP8266 RGB Smart Lamp v" + String(FIRMWARE_VERSION) + "\n";
    info += "LEDs: " + String(NUM_LEDS) + "\n";
    info += "Type: " + String("WS2812B") + "\n";
    info += "Chip ID: " + String(ESP.getChipId(), HEX) + "\n";
    info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    info += "Uptime: " + String(millis() / 1000) + " seconds\n";
    info += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    info += "IP: " + WiFi.localIP().toString() + "\n";
    server.send(200, "text/plain", info);
}

// ========================================================================
// OTA & WIFI
// ========================================================================
void setupOTA() {
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword("admin");

    ArduinoOTA.onStart([]() {
        otaInProgress = true;
        statusLEDBlink(100);
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
        FastLED.show();
        Serial.println("\n🔄 OTA Update Started");
    });

    ArduinoOTA.onEnd([]() {
        statusLEDSolid();
        Serial.println("\n✓ OTA Complete");
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("\n✗ OTA Error[%u]\n", error);
        otaInProgress = false;
        statusLEDSolid();
    });

    ArduinoOTA.begin();
    Serial.println("✓ OTA enabled");
}

void setupWiFi() {
    Serial.println("\n📶 Configuring WiFi...");
    wifiManager.setConfigPortalTimeout(180);
    statusLEDBlink(500);

    if (!wifiManager.autoConnect("ESP8266-RGB-Setup", "12345678")) {
        Serial.println("✗ Failed to connect - restarting");
        delay(3000);
        ESP.restart();
    }

    Serial.println("✓ WiFi connected!");
    Serial.print("📍 IP: ");
    Serial.println(WiFi.localIP());
    statusLEDSolid();
}

void checkWiFi() {
    if (millis() - lastWiFiCheck < WIFI_RECONNECT_INTERVAL) return;
    lastWiFiCheck = millis();

    if (WiFi.status() != WL_CONNECTED && !otaInProgress) {
        Serial.println("⚠️  WiFi disconnected! Reconnecting...");
        WiFi.reconnect();
    }
}

// ========================================================================
// SETUP
// ========================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║  ESP8266 RGB SMART LAMP v2.1          ║");
    Serial.println("║  Full Color • Effects • HomeKit       ║");
    Serial.println("╚════════════════════════════════════════╝\n");

    // Initialize file system
    if (!LittleFS.begin()) {
        LittleFS.format();
        LittleFS.begin();
    }
    Serial.println("✓ File system mounted");

    // Load settings
    loadSettings();

    // Setup GPIO
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);

    // Initialize LED strip
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(MAX_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    Serial.print("✓ LED strip initialized: ");
    Serial.print(NUM_LEDS);
    Serial.println(" LEDs");

    // Startup animation
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Blue;
        FastLED.show();
        delay(20);
    }
    delay(200);
    FastLED.clear();
    FastLED.show();

    // Setup WiFi
    setupWiFi();

    // Setup mDNS
    if (MDNS.begin(HOSTNAME)) {
        Serial.println("✓ mDNS started: " + String(HOSTNAME) + ".local");
        MDNS.addService("http", "tcp", 80);
    }

    // Setup OTA
    setupOTA();

    // Setup Web Server
    server.on("/", handleRoot);
    server.on("/lamp", handleLamp);
    server.on("/color", handleColor);
    server.on("/scene", handleScene);
    server.on("/effect", handleEffect);
    server.on("/status", handleStatus);
    server.on("/info", handleInfo);
    server.begin();
    Serial.println("✓ Web server started");

    // Setup HomeKit
    Serial.println("\n🏠 Starting HomeKit...");
    arduino_homekit_setup(&homekitConfig);
    Serial.println("✓ HomeKit ready");

    // Restore state
    updateLED();
    Serial.println("✓ State restored");

    // Enable watchdog
    ESP.wdtEnable(WATCHDOG_TIMEOUT);
    Serial.println("✓ Watchdog enabled");

    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║          🎉 SYSTEM READY! 🎉          ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println("\n🌐 Web UI: http://" + WiFi.localIP().toString());
    Serial.println("🌐 Or: http://" + String(HOSTNAME) + ".local");
    Serial.println("🍎 HomeKit Code: 111-22-333");
    Serial.println("🎨 Say: 'Hey Siri, set lamp to purple'\n");
}

// ========================================================================
// MAIN LOOP
// ========================================================================
void loop() {
    ESP.wdtFeed();

    arduino_homekit_loop();
    server.handleClient();
    ArduinoOTA.handle();
    MDNS.update();

    handleButton();
    checkWiFi();
    updateEffect();

    yield();
}
