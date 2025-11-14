/*
 * ========================================================================
 * ESP8266 HOMEKIT SMART LAMP - PRODUCTION VERSION v2.0
 * ========================================================================
 *
 * PRODUCTION FEATURES:
 * ✅ OTA Updates - Upload firmware wirelessly
 * ✅ WiFi Manager - Easy setup via captive portal (no hardcoded WiFi)
 * ✅ Persistent Storage - Remembers state after power loss
 * ✅ Physical Button - Toggle lamp, cycle scenes, factory reset
 * ✅ Watchdog Protection - Auto-recovery from crashes
 * ✅ 5 Scene Presets - Bright, Reading, Relax, Night, Custom
 * ✅ Smooth Fading - Professional transitions
 * ✅ Enhanced Web UI - Full settings control
 * ✅ Status LED - Visual feedback for different states
 * ✅ Auto WiFi Reconnect - Resilient connection
 * ✅ Factory Reset - Hold button 10 seconds
 *
 * ========================================================================
 * REQUIRED LIBRARIES - INSTALL ALL OF THESE!
 * ========================================================================
 * 1. Go to: Sketch → Include Library → Manage Libraries
 * 2. Search and install:
 *    - "HomeKit-ESP8266" by Mixiaoxiao
 *    - "WiFiManager" by tzapu (version 2.0.x)
 *    - "ArduinoJson" by Benoit Blanchon (v6.x)
 *
 * ========================================================================
 * BOARD SETTINGS
 * ========================================================================
 * Board: "Generic ESP8266 Module" or "NodeMCU 1.0"
 * Flash Size: "4MB (FS:1MB OTA:~1019KB)" ⚠️ IMPORTANT for OTA!
 * CPU Frequency: "160 MHz"
 * Upload Speed: "115200"
 *
 * ========================================================================
 * HARDWARE CONNECTIONS
 * ========================================================================
 * GPIO2 (D4)  - Lamp output (built-in LED or relay)
 * GPIO0 (D3)  - Physical button (with pull-up)
 * GPIO16 (D0) - Status LED (optional, or use built-in)
 *
 * Button wiring: GPIO0 → Button → GND (uses internal pull-up)
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

// ========================================================================
// CONFIGURATION
// ========================================================================
#define LAMP_PIN 2              // GPIO2 - Lamp control (inverted logic)
#define BUTTON_PIN 0            // GPIO0 - Physical button
#define STATUS_LED_PIN 16       // GPIO16 - Status LED (optional)

#define DEVICE_NAME "ESP8266 Lamp"
#define HOSTNAME "esp8266-lamp"
#define FIRMWARE_VERSION "2.0.0"

// Button timing
#define DEBOUNCE_DELAY 50       // ms
#define LONG_PRESS_TIME 3000    // 3s to cycle scenes
#define FACTORY_RESET_TIME 10000 // 10s to factory reset

// Fade settings
#define FADE_STEP_DELAY 20      // ms between fade steps
#define FADE_STEPS 50           // Number of steps for smooth fade

// WiFi settings
#define WIFI_CONNECT_TIMEOUT 30 // seconds
#define WIFI_RECONNECT_INTERVAL 30000 // ms

// Watchdog
#define WATCHDOG_TIMEOUT 8000   // 8 seconds

// ========================================================================
// GLOBAL VARIABLES
// ========================================================================
ESP8266WebServer server(80);
WiFiManager wifiManager;
Ticker statusLEDTicker;
Ticker fadeTicker;

// Lamp state
bool lampOn = false;
int lampBrightness = 100;
int currentScene = 0; // 0=Custom, 1=Bright, 2=Reading, 3=Relax, 4=Night
int targetBrightness = 100;
int currentPWM = 0;
bool isFading = false;

// Button state
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
bool longPressHandled = false;
bool veryLongPressHandled = false;

// WiFi state
unsigned long lastWiFiCheck = 0;
bool otaInProgress = false;

// Scene definitions (brightness %)
const int SCENE_BRIGHT = 100;
const int SCENE_READING = 70;
const int SCENE_RELAX = 40;
const int SCENE_NIGHT = 10;

// ========================================================================
// HOMEKIT FORWARD DECLARATIONS
// ========================================================================
void updateLamp();
void setLampBrightness(int brightness, bool smooth = true);
void lampIdentify(homekit_value_t _value);
homekit_value_t lampOnGet();
void lampOnSet(homekit_value_t value);
homekit_value_t lampBrightnessGet();
void lampBrightnessSet(homekit_value_t value);

// ========================================================================
// HOMEKIT ACCESSORY DEFINITION
// ========================================================================
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "DIY Maker"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "ESP8266-PROD-001"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Smart Lamp Pro v2.0"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, FIRMWARE_VERSION),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, lampIdentify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
            HOMEKIT_CHARACTERISTIC(ON, false, .getter=lampOnGet, .setter=lampOnSet),
            HOMEKIT_CHARACTERISTIC(BRIGHTNESS, 100, .getter=lampBrightnessGet, .setter=lampBrightnessSet),
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
    doc["lampOn"] = lampOn;
    doc["brightness"] = lampBrightness;
    doc["scene"] = currentScene;

    File file = LittleFS.open("/settings.json", "w");
    if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.println("✓ Settings saved");
    } else {
        Serial.println("✗ Failed to save settings");
    }
}

void loadSettings() {
    if (!LittleFS.exists("/settings.json")) {
        Serial.println("⚠️  No saved settings, using defaults");
        return;
    }

    File file = LittleFS.open("/settings.json", "r");
    if (!file) {
        Serial.println("✗ Failed to load settings");
        return;
    }

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("✗ Failed to parse settings");
        return;
    }

    lampOn = doc["lampOn"] | false;
    lampBrightness = doc["brightness"] | 100;
    currentScene = doc["scene"] | 0;
    targetBrightness = lampBrightness;

    Serial.println("✓ Settings loaded");
}

void factoryReset() {
    Serial.println("\n⚠️  FACTORY RESET INITIATED!");

    // Blink rapidly to indicate reset
    for (int i = 0; i < 20; i++) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
        delay(100);
    }

    // Clear settings
    LittleFS.format();
    Serial.println("✓ File system formatted");

    // Clear WiFi
    wifiManager.resetSettings();
    Serial.println("✓ WiFi settings cleared");

    Serial.println("✓ Factory reset complete - rebooting...");
    delay(1000);
    ESP.restart();
}

// ========================================================================
// LAMP CONTROL WITH SMOOTH FADING
// ========================================================================
void fadeStep() {
    if (!isFading) return;

    if (currentPWM < targetBrightness) {
        currentPWM += max(1, (targetBrightness - currentPWM) / 10);
        if (currentPWM >= targetBrightness) {
            currentPWM = targetBrightness;
            isFading = false;
            fadeTicker.detach();
        }
    } else if (currentPWM > targetBrightness) {
        currentPWM -= max(1, (currentPWM - targetBrightness) / 10);
        if (currentPWM <= targetBrightness) {
            currentPWM = targetBrightness;
            isFading = false;
            fadeTicker.detach();
        }
    }

    int pwmValue = map(currentPWM, 0, 100, 0, 1023);
    analogWrite(LAMP_PIN, 1023 - pwmValue); // Inverted logic
}

void setLampBrightness(int brightness, bool smooth) {
    lampBrightness = constrain(brightness, 0, 100);
    targetBrightness = lampOn ? lampBrightness : 0;

    if (smooth) {
        if (!isFading) {
            isFading = true;
            fadeTicker.attach_ms(FADE_STEP_DELAY, fadeStep);
        }
    } else {
        currentPWM = targetBrightness;
        int pwmValue = map(currentPWM, 0, 100, 0, 1023);
        analogWrite(LAMP_PIN, 1023 - pwmValue);
        isFading = false;
        fadeTicker.detach();
    }
}

void updateLamp() {
    targetBrightness = lampOn ? lampBrightness : 0;
    setLampBrightness(lampBrightness, true);
    saveSettings();

    Serial.print("💡 Lamp ");
    Serial.print(lampOn ? "ON" : "OFF");
    if (lampOn) {
        Serial.print(" at ");
        Serial.print(lampBrightness);
        Serial.print("%");
    }
    Serial.println();
}

// ========================================================================
// SCENE MANAGEMENT
// ========================================================================
void applyScene(int scene) {
    currentScene = scene;

    switch (scene) {
        case 1: // Bright
            lampBrightness = SCENE_BRIGHT;
            Serial.println("🌟 Scene: BRIGHT");
            break;
        case 2: // Reading
            lampBrightness = SCENE_READING;
            Serial.println("📖 Scene: READING");
            break;
        case 3: // Relax
            lampBrightness = SCENE_RELAX;
            Serial.println("😌 Scene: RELAX");
            break;
        case 4: // Night
            lampBrightness = SCENE_NIGHT;
            Serial.println("🌙 Scene: NIGHT");
            break;
        default: // Custom
            currentScene = 0;
            Serial.println("✨ Scene: CUSTOM");
            break;
    }

    if (lampOn) {
        setLampBrightness(lampBrightness, true);

        // Notify HomeKit
        homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_BRIGHTNESS);
        if (ch) {
            homekit_characteristic_notify(ch, HOMEKIT_INT(lampBrightness));
        }
    }

    saveSettings();
}

void cycleScene() {
    int nextScene = (currentScene + 1) % 5;
    if (!lampOn) {
        lampOn = true;
    }
    applyScene(nextScene);
}

// ========================================================================
// BUTTON HANDLING
// ========================================================================
void handleButton() {
    bool buttonState = digitalRead(BUTTON_PIN) == LOW; // Active LOW

    if (buttonState && !buttonPressed) {
        // Button just pressed
        buttonPressed = true;
        buttonPressTime = millis();
        longPressHandled = false;
        veryLongPressHandled = false;
    } else if (!buttonState && buttonPressed) {
        // Button released
        unsigned long pressDuration = millis() - buttonPressTime;

        if (pressDuration < LONG_PRESS_TIME && !longPressHandled && !veryLongPressHandled) {
            // Short press - toggle lamp
            lampOn = !lampOn;
            updateLamp();

            // Notify HomeKit
            homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
                &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
            if (ch) {
                homekit_characteristic_notify(ch, HOMEKIT_BOOL(lampOn));
            }

            Serial.println("🔘 Button: SHORT PRESS - Toggle");
        }

        buttonPressed = false;
    } else if (buttonPressed) {
        // Button held
        unsigned long pressDuration = millis() - buttonPressTime;

        if (pressDuration >= FACTORY_RESET_TIME && !veryLongPressHandled) {
            // Very long press - factory reset
            veryLongPressHandled = true;
            Serial.println("🔘 Button: VERY LONG PRESS - Factory Reset!");
            factoryReset();
        } else if (pressDuration >= LONG_PRESS_TIME && !longPressHandled && !veryLongPressHandled) {
            // Long press - cycle scenes
            longPressHandled = true;
            cycleScene();
            Serial.println("🔘 Button: LONG PRESS - Cycle Scene");
        }
    }
}

// ========================================================================
// STATUS LED PATTERNS
// ========================================================================
void statusLEDSolid() {
    statusLEDTicker.detach();
    digitalWrite(STATUS_LED_PIN, LOW); // ON
}

void statusLEDOff() {
    statusLEDTicker.detach();
    digitalWrite(STATUS_LED_PIN, HIGH); // OFF
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
        digitalWrite(LAMP_PIN, LOW);
        delay(200);
        digitalWrite(LAMP_PIN, HIGH);
        delay(200);
    }
    updateLamp();
}

homekit_value_t lampOnGet() {
    return HOMEKIT_BOOL(lampOn);
}

void lampOnSet(homekit_value_t value) {
    lampOn = value.bool_value;
    Serial.print("📱 HomeKit: Lamp ");
    Serial.println(lampOn ? "ON" : "OFF");
    updateLamp();
}

homekit_value_t lampBrightnessGet() {
    return HOMEKIT_INT(lampBrightness);
}

void lampBrightnessSet(homekit_value_t value) {
    lampBrightness = value.int_value;
    currentScene = 0; // Custom when changed from HomeKit
    Serial.print("📱 HomeKit: Brightness ");
    Serial.print(lampBrightness);
    Serial.println("%");
    updateLamp();
}

// ========================================================================
// WEB SERVER - ENHANCED UI
// ========================================================================
const char HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Lamp Pro</title>
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
            padding: 40px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
        }
        h1 {
            text-align: center;
            color: #333;
            font-size: 32px;
            margin-bottom: 5px;
        }
        .version {
            text-align: center;
            color: #999;
            font-size: 12px;
            margin-bottom: 30px;
        }
        .lamp-icon {
            text-align: center;
            font-size: 100px;
            margin: 20px 0;
            transition: all 0.4s;
            filter: grayscale(100%) opacity(0.4);
        }
        .lamp-icon.active {
            filter: grayscale(0%) opacity(1);
            text-shadow: 0 0 40px rgba(255, 215, 0, 0.7);
        }
        .status {
            background: #f5f5f5;
            padding: 15px;
            border-radius: 15px;
            text-align: center;
            margin: 20px 0;
        }
        .status-value {
            font-size: 24px;
            font-weight: bold;
            color: #4CAF50;
        }
        .status-value.off { color: #f44336; }
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin: 20px 0;
        }
        button {
            padding: 15px;
            border: none;
            border-radius: 12px;
            font-size: 16px;
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
            font-size: 14px;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 20px rgba(0,0,0,0.2);
        }
        .brightness {
            margin: 25px 0;
        }
        .brightness-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            color: #555;
        }
        input[type="range"] {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: linear-gradient(to right, #ddd, #667eea);
            outline: none;
            -webkit-appearance: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: white;
            border: 3px solid #667eea;
            cursor: pointer;
            box-shadow: 0 2px 8px rgba(0,0,0,0.2);
        }
        .scenes {
            margin: 25px 0;
        }
        .scenes h3 {
            color: #555;
            margin-bottom: 10px;
            font-size: 16px;
        }
        .info-box {
            background: #e3f2fd;
            padding: 15px;
            border-radius: 10px;
            margin-top: 20px;
            font-size: 13px;
            color: #1976D2;
        }
        .info-box strong { display: block; margin-bottom: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>💡 Smart Lamp Pro</h1>
        <div class="version">v2.0 Production • HomeKit Ready</div>

        <div class="lamp-icon" id="lampIcon">💡</div>

        <div class="status">
            <div style="font-size: 12px; color: #666; margin-bottom: 5px;">STATUS</div>
            <div class="status-value" id="statusText">OFF</div>
        </div>

        <div class="controls">
            <button class="btn-on" onclick="turnOn()">ON</button>
            <button class="btn-off" onclick="turnOff()">OFF</button>
        </div>

        <div class="brightness">
            <div class="brightness-header">
                <span>Brightness</span>
                <span style="color: #667eea; font-weight: bold;"><span id="brightValue">100</span>%</span>
            </div>
            <input type="range" min="1" max="100" value="100" id="brightnessSlider" oninput="setBrightness(this.value)">
        </div>

        <div class="scenes">
            <h3>⭐ Scenes</h3>
            <div class="controls" style="grid-template-columns: repeat(2, 1fr);">
                <button class="btn-scene" onclick="setScene(1)">🌟 Bright</button>
                <button class="btn-scene" onclick="setScene(2)">📖 Reading</button>
                <button class="btn-scene" onclick="setScene(3)">😌 Relax</button>
                <button class="btn-scene" onclick="setScene(4)">🌙 Night</button>
            </div>
        </div>

        <div class="info-box">
            <strong>🍎 Apple Home Integration</strong>
            Add to Home app with code: <strong>111-22-333</strong><br>
            Say: "Hey Siri, turn on the lamp"
        </div>

        <div class="info-box" style="background: #fff3cd; color: #856404; margin-top: 10px;">
            <strong>🔘 Physical Button</strong>
            Short press: Toggle • Long press: Cycle scenes<br>
            Hold 10s: Factory reset
        </div>
    </div>

    <script>
        function turnOn() {
            fetch('/lamp?state=1').then(() => refresh());
        }

        function turnOff() {
            fetch('/lamp?state=0').then(() => refresh());
        }

        function setBrightness(val) {
            document.getElementById('brightValue').textContent = val;
            fetch('/brightness?value=' + val);
        }

        function setScene(scene) {
            fetch('/scene?id=' + scene).then(() => refresh());
        }

        function refresh() {
            fetch('/status').then(r => r.json()).then(data => {
                const icon = document.getElementById('lampIcon');
                const status = document.getElementById('statusText');

                if (data.on) {
                    icon.classList.add('active');
                    status.textContent = 'ON';
                    status.classList.remove('off');
                } else {
                    icon.classList.remove('active');
                    status.textContent = 'OFF';
                    status.classList.add('off');
                }

                document.getElementById('brightnessSlider').value = data.brightness;
                document.getElementById('brightValue').textContent = data.brightness;
            });
        }

        setInterval(refresh, 2000);
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
        updateLamp();

        homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
        if (ch) {
            homekit_characteristic_notify(ch, HOMEKIT_BOOL(lampOn));
        }

        server.send(200, "text/plain", lampOn ? "ON" : "OFF");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

void handleBrightness() {
    if (server.hasArg("value")) {
        lampBrightness = constrain(server.arg("value").toInt(), 0, 100);
        currentScene = 0; // Custom

        if (lampOn) {
            setLampBrightness(lampBrightness, true);
        }

        homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_BRIGHTNESS);
        if (ch) {
            homekit_characteristic_notify(ch, HOMEKIT_INT(lampBrightness));
        }

        saveSettings();
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing value");
    }
}

void handleScene() {
    if (server.hasArg("id")) {
        int scene = server.arg("id").toInt();
        if (!lampOn) lampOn = true;
        applyScene(scene);

        homekit_characteristic_t *ch_on = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
        homekit_characteristic_t *ch_br = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_BRIGHTNESS);

        if (ch_on) homekit_characteristic_notify(ch_on, HOMEKIT_BOOL(lampOn));
        if (ch_br) homekit_characteristic_notify(ch_br, HOMEKIT_INT(lampBrightness));

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing id");
    }
}

void handleStatus() {
    String json = "{\"on\":" + String(lampOn ? "true" : "false") +
                  ",\"brightness\":" + String(lampBrightness) +
                  ",\"scene\":" + String(currentScene) + "}";
    server.send(200, "application/json", json);
}

void handleInfo() {
    String info = "ESP8266 Smart Lamp Pro v" + String(FIRMWARE_VERSION) + "\n";
    info += "Chip ID: " + String(ESP.getChipId(), HEX) + "\n";
    info += "Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB\n";
    info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    info += "Uptime: " + String(millis() / 1000) + " seconds\n";
    info += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    info += "IP: " + WiFi.localIP().toString() + "\n";
    server.send(200, "text/plain", info);
}

// ========================================================================
// OTA SETUP
// ========================================================================
void setupOTA() {
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword("admin"); // Change this!

    ArduinoOTA.onStart([]() {
        otaInProgress = true;
        statusLEDBlink(100); // Fast blink during OTA
        Serial.println("\n🔄 OTA Update Started");

        // Turn off lamp during update
        digitalWrite(LAMP_PIN, HIGH);
    });

    ArduinoOTA.onEnd([]() {
        statusLEDSolid();
        Serial.println("\n✓ OTA Update Complete");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("\n✗ OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");

        otaInProgress = false;
        statusLEDSolid();
    });

    ArduinoOTA.begin();
    Serial.println("✓ OTA enabled");
}

// ========================================================================
// WiFi MANAGEMENT
// ========================================================================
void setupWiFi() {
    Serial.println("\n📶 Configuring WiFi...");

    wifiManager.setConfigPortalTimeout(180); // 3 minutes
    wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT);

    statusLEDBlink(500); // Slow blink while connecting

    // Try to connect to saved WiFi or start config portal
    if (!wifiManager.autoConnect("ESP8266-Setup", "12345678")) {
        Serial.println("✗ Failed to connect - restarting");
        delay(3000);
        ESP.restart();
    }

    Serial.println("✓ WiFi connected!");
    Serial.print("📍 IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    statusLEDSolid();
}

void checkWiFi() {
    if (millis() - lastWiFiCheck < WIFI_RECONNECT_INTERVAL) return;
    lastWiFiCheck = millis();

    if (WiFi.status() != WL_CONNECTED && !otaInProgress) {
        Serial.println("⚠️  WiFi disconnected! Reconnecting...");
        statusLEDBlink(500);
        WiFi.reconnect();
    } else if (WiFi.status() == WL_CONNECTED) {
        statusLEDSolid();
    }
}

// ========================================================================
// SETUP
// ========================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║  ESP8266 SMART LAMP PRO v2.0          ║");
    Serial.println("║  Production Ready • Full Featured     ║");
    Serial.println("╚════════════════════════════════════════╝\n");

    // Initialize file system
    if (!LittleFS.begin()) {
        Serial.println("✗ LittleFS mount failed - formatting...");
        LittleFS.format();
        if (!LittleFS.begin()) {
            Serial.println("✗ LittleFS failed!");
        }
    }
    Serial.println("✓ File system mounted");

    // Load saved settings
    loadSettings();

    // Setup GPIO
    pinMode(LAMP_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(STATUS_LED_PIN, OUTPUT);

    digitalWrite(LAMP_PIN, HIGH); // Start OFF (inverted)
    digitalWrite(STATUS_LED_PIN, HIGH); // OFF

    Serial.println("✓ GPIO configured");

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
    server.on("/brightness", handleBrightness);
    server.on("/scene", handleScene);
    server.on("/status", handleStatus);
    server.on("/info", handleInfo);
    server.begin();
    Serial.println("✓ Web server started");

    // Setup HomeKit
    Serial.println("\n🏠 Starting HomeKit...");
    arduino_homekit_setup(&homekitConfig);
    Serial.println("✓ HomeKit ready");

    // Restore lamp state
    setLampBrightness(lampBrightness, false);
    Serial.print("✓ Restored state: ");
    Serial.print(lampOn ? "ON" : "OFF");
    Serial.print(" @ ");
    Serial.print(lampBrightness);
    Serial.println("%");

    // Enable watchdog
    ESP.wdtEnable(WATCHDOG_TIMEOUT);
    Serial.println("✓ Watchdog enabled");

    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║          🎉 SYSTEM READY! 🎉          ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println("\n🌐 Web UI: http://" + WiFi.localIP().toString());
    Serial.println("🌐 Or: http://" + String(HOSTNAME) + ".local");
    Serial.println("🍎 HomeKit Code: 111-22-333");
    Serial.println("🔄 OTA Password: admin");
    Serial.println("\n");
}

// ========================================================================
// MAIN LOOP
// ========================================================================
void loop() {
    // Feed watchdog
    ESP.wdtFeed();

    // Handle services
    arduino_homekit_loop();
    server.handleClient();
    ArduinoOTA.handle();
    MDNS.update();

    // Handle button
    handleButton();

    // Check WiFi
    checkWiFi();

    // Small delay to prevent watchdog timeout
    yield();
}
