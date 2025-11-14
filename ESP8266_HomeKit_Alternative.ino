/*
 * ========================================================================
 * ESP8266 APPLE HOMEKIT LAMP - ALTERNATIVE VERSION (RECOMMENDED!)
 * ========================================================================
 * This version uses arduino-homekit-esp8266 library
 * which is SPECIFICALLY designed for ESP8266
 *
 * Use this if the other version doesn't compile or work!
 *
 * ========================================================================
 * REQUIRED LIBRARIES - INSTALL THESE FIRST!
 * ========================================================================
 * 1. Go to: Sketch → Include Library → Manage Libraries
 * 2. Search and install these libraries:
 *
 *    a) "HomeKit-ESP8266" by Mixiaoxiao
 *    b) It will ask to install dependencies - click "Install All"
 *
 * ========================================================================
 * BOARD SETTINGS - VERY IMPORTANT!
 * ========================================================================
 * Tools → Board: "Generic ESP8266 Module" or "NodeMCU 1.0"
 * Tools → Flash Size: "4MB (FS:2MB OTA:~1019KB)"  ← IMPORTANT!
 * Tools → CPU Frequency: "160 MHz"  ← IMPORTANT for HomeKit!
 * Tools → Upload Speed: "115200"
 *
 * ========================================================================
 * CONFIGURATION - CHANGE YOUR WIFI HERE!
 * ========================================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

// ⚠️ CHANGE THESE TO YOUR WIFI CREDENTIALS ⚠️
#define WIFI_SSID "YOUR_WIFI_NAME"        // Your WiFi network name
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD" // Your WiFi password

// Device configuration
#define LAMP_PIN 2        // GPIO2 (D4 on NodeMCU) - built-in LED
#define DEVICE_NAME "ESP8266 Lamp"
#define HOSTNAME "esp8266-lamp"

// Web server
ESP8266WebServer server(80);

// Lamp state
bool lampOn = false;
int lampBrightness = 100;

// ========================================================================
// HOMEKIT ACCESSORY DEFINITION
// ========================================================================
// This is in a separate section for clarity

// Forward declarations
void updateLamp();
void lampIdentify(homekit_value_t _value);
homekit_value_t lampOnGet();
void lampOnSet(homekit_value_t value);
homekit_value_t lampBrightnessGet();
void lampBrightnessSet(homekit_value_t value);

// HomeKit Accessory definition
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "DIY Maker"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "ESP8266-12345"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Smart Lamp v1"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, lampIdentify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
            HOMEKIT_CHARACTERISTIC(
                ON,
                false,
                .getter=lampOnGet,
                .setter=lampOnSet
            ),
            HOMEKIT_CHARACTERISTIC(
                BRIGHTNESS,
                100,
                .getter=lampBrightnessGet,
                .setter=lampBrightnessSet
            ),
            NULL
        }),
        NULL
    }),
    NULL
};

// HomeKit server configuration
homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-22-333",  // Setup code for pairing
    .setupId = "ES32"          // 4-character setup ID
};

// ========================================================================
// HOMEKIT CALLBACK FUNCTIONS
// ========================================================================

// Called when "Identify" is triggered from Home app
void lampIdentify(homekit_value_t _value) {
    Serial.println("🔍 Identify requested - blinking lamp");
    for (int i = 0; i < 3; i++) {
        digitalWrite(LAMP_PIN, LOW);
        delay(200);
        digitalWrite(LAMP_PIN, HIGH);
        delay(200);
    }
    updateLamp();  // Restore original state
}

// Get lamp ON/OFF state
homekit_value_t lampOnGet() {
    return HOMEKIT_BOOL(lampOn);
}

// Set lamp ON/OFF state
void lampOnSet(homekit_value_t value) {
    lampOn = value.bool_value;
    Serial.print("📱 HomeKit set lamp: ");
    Serial.println(lampOn ? "ON" : "OFF");
    updateLamp();
}

// Get lamp brightness
homekit_value_t lampBrightnessGet() {
    return HOMEKIT_INT(lampBrightness);
}

// Set lamp brightness
void lampBrightnessSet(homekit_value_t value) {
    lampBrightness = value.int_value;
    Serial.print("📱 HomeKit set brightness: ");
    Serial.print(lampBrightness);
    Serial.println("%");
    updateLamp();
}

// ========================================================================
// LAMP CONTROL
// ========================================================================
void updateLamp() {
    if (lampOn) {
        // Calculate PWM value based on brightness (0-100%)
        int pwmValue = map(lampBrightness, 0, 100, 0, 1023);

        // For built-in LED (inverted logic: LOW=ON)
        analogWrite(LAMP_PIN, 1023 - pwmValue);

        Serial.print("💡 Lamp ON at ");
        Serial.print(lampBrightness);
        Serial.println("% brightness");
    } else {
        digitalWrite(LAMP_PIN, HIGH);  // HIGH=OFF (inverted logic)
        Serial.println("🌑 Lamp OFF");
    }
}

// ========================================================================
// HTML WEB INTERFACE
// ========================================================================
const char HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Smart Lamp</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .card {
            background: white;
            border-radius: 25px;
            padding: 45px;
            box-shadow: 0 25px 50px rgba(0,0,0,0.3);
            max-width: 450px;
            width: 100%;
        }
        h1 {
            text-align: center;
            color: #333;
            font-size: 32px;
            margin-bottom: 10px;
        }
        .subtitle {
            text-align: center;
            color: #888;
            margin-bottom: 35px;
            font-size: 15px;
        }
        .lamp-icon {
            text-align: center;
            font-size: 140px;
            margin: 25px 0;
            transition: all 0.4s ease;
            filter: grayscale(100%) opacity(0.4);
        }
        .lamp-icon.active {
            filter: grayscale(0%) opacity(1);
            text-shadow: 0 0 40px rgba(255, 215, 0, 0.7);
        }
        .status-box {
            background: #f5f5f5;
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            margin: 25px 0;
        }
        .status-label {
            color: #666;
            font-size: 13px;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 8px;
        }
        .status-value {
            font-size: 28px;
            font-weight: bold;
            color: #4CAF50;
        }
        .status-value.off {
            color: #f44336;
        }
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin: 25px 0;
        }
        button {
            padding: 18px;
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
        .btn-on:hover {
            transform: translateY(-3px);
            box-shadow: 0 10px 25px rgba(56, 239, 125, 0.4);
        }
        .btn-off {
            background: linear-gradient(135deg, #ee0979, #ff6a00);
        }
        .btn-off:hover {
            transform: translateY(-3px);
            box-shadow: 0 10px 25px rgba(238, 9, 121, 0.4);
        }
        .brightness-section {
            margin: 30px 0;
        }
        .brightness-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 12px;
            color: #555;
            font-weight: 500;
        }
        .brightness-value {
            color: #667eea;
            font-weight: bold;
            font-size: 18px;
        }
        input[type="range"] {
            width: 100%;
            height: 10px;
            border-radius: 5px;
            background: linear-gradient(to right, #ddd, #667eea);
            outline: none;
            -webkit-appearance: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 26px;
            height: 26px;
            border-radius: 50%;
            background: white;
            border: 3px solid #667eea;
            cursor: pointer;
            box-shadow: 0 3px 10px rgba(0,0,0,0.2);
        }
        .homekit-badge {
            background: linear-gradient(135deg, #e0f7fa, #b2ebf2);
            border-left: 5px solid #00bcd4;
            padding: 20px;
            border-radius: 12px;
            margin-top: 30px;
            text-align: center;
        }
        .homekit-badge strong {
            display: block;
            color: #00838f;
            font-size: 16px;
            margin-bottom: 8px;
        }
        .homekit-badge small {
            color: #00695c;
            line-height: 1.6;
        }
        .setup-code {
            display: inline-block;
            background: white;
            padding: 8px 16px;
            border-radius: 8px;
            font-family: 'Courier New', monospace;
            font-size: 20px;
            font-weight: bold;
            color: #667eea;
            margin: 10px 0;
            letter-spacing: 2px;
        }
    </style>
</head>
<body>
    <div class="card">
        <h1>💡 Smart Lamp</h1>
        <div class="subtitle">Apple HomeKit Compatible</div>

        <div class="lamp-icon" id="lampIcon">💡</div>

        <div class="status-box">
            <div class="status-label">Status</div>
            <div class="status-value" id="statusText">OFF</div>
        </div>

        <div class="controls">
            <button class="btn-on" onclick="turnOn()">🌟 ON</button>
            <button class="btn-off" onclick="turnOff()">🌙 OFF</button>
        </div>

        <div class="brightness-section">
            <div class="brightness-header">
                <span>💫 Brightness</span>
                <span class="brightness-value"><span id="brightValue">100</span>%</span>
            </div>
            <input type="range" min="1" max="100" value="100"
                   id="brightnessSlider" oninput="updateBrightness(this.value)">
        </div>

        <div class="homekit-badge">
            <strong>🍎 Add to Apple Home</strong>
            <small>
                Open Home app → Add Accessory<br>
                Select "ESP8266 Lamp"<br>
                Enter setup code:<br>
            </small>
            <div class="setup-code">111-22-333</div>
            <small>Say: "Hey Siri, turn on the lamp"</small>
        </div>
    </div>

    <script>
        function turnOn() {
            fetch('/lamp?state=1').then(() => refreshStatus());
        }

        function turnOff() {
            fetch('/lamp?state=0').then(() => refreshStatus());
        }

        function updateBrightness(val) {
            document.getElementById('brightValue').textContent = val;
            fetch('/brightness?value=' + val);
        }

        function refreshStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
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

        // Auto-refresh every 2 seconds
        setInterval(refreshStatus, 2000);
        refreshStatus();
    </script>
</body>
</html>
)=====";

// ========================================================================
// WEB SERVER HANDLERS
// ========================================================================
void handleRoot() {
    server.send(200, "text/html", HTML);
}

void handleLamp() {
    if (server.hasArg("state")) {
        int state = server.arg("state").toInt();
        lampOn = (state == 1);

        // Update HomeKit
        homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_ON);
        if (ch) {
            homekit_characteristic_notify(ch, HOMEKIT_BOOL(lampOn));
        }

        updateLamp();
        server.send(200, "text/plain", lampOn ? "ON" : "OFF");
    } else {
        server.send(400, "text/plain", "Missing state parameter");
    }
}

void handleBrightness() {
    if (server.hasArg("value")) {
        lampBrightness = server.arg("value").toInt();
        lampBrightness = constrain(lampBrightness, 0, 100);

        // Update HomeKit
        homekit_characteristic_t *ch = homekit_service_characteristic_by_type(
            &accessories[0]->services[1], HOMEKIT_CHARACTERISTIC_BRIGHTNESS);
        if (ch) {
            homekit_characteristic_notify(ch, HOMEKIT_INT(lampBrightness));
        }

        if (lampOn) {
            updateLamp();
        }

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing value");
    }
}

void handleStatus() {
    String json = "{\"on\":" + String(lampOn ? "true" : "false") +
                  ",\"brightness\":" + String(lampBrightness) + "}";
    server.send(200, "application/json", json);
}

// ========================================================================
// WIFI CONNECTION
// ========================================================================
void setupWiFi() {
    Serial.println("\n========================================");
    Serial.println("📶 Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.println("========================================");

    WiFi.mode(WIFI_STA);
    WiFi.hostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;

        if (attempts % 10 == 0) {
            Serial.println();
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n\n✓ WiFi connected successfully!");
        Serial.print("📍 IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("🌐 Hostname: http://");
        Serial.print(HOSTNAME);
        Serial.println(".local");
        Serial.print("📶 Signal: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("\n\n✗ WiFi connection FAILED!");
        Serial.println("⚠️  Please check:");
        Serial.println("   - WiFi name (SSID) is correct");
        Serial.println("   - WiFi password is correct");
        Serial.println("   - WiFi is 2.4GHz (ESP8266 doesn't support 5GHz)");
        Serial.println("   - ESP8266 is close enough to router");
        Serial.println("\n⚠️  Edit lines 40-41 in the code to fix WiFi credentials");
    }
}

// ========================================================================
// SETUP
// ========================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║   ESP8266 HOMEKIT SMART LAMP v1.0     ║");
    Serial.println("╚════════════════════════════════════════╝");

    // Setup lamp pin
    pinMode(LAMP_PIN, OUTPUT);
    digitalWrite(LAMP_PIN, HIGH);  // Start OFF
    Serial.println("✓ Lamp GPIO configured (Pin 2)");

    // Connect to WiFi
    setupWiFi();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n⚠️  Cannot continue without WiFi!");
        Serial.println("⚠️  Please fix WiFi and restart ESP8266");
        while(1) { delay(1000); }  // Stop here
    }

    // Start mDNS
    if (MDNS.begin(HOSTNAME)) {
        Serial.println("✓ mDNS responder started");
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("✗ mDNS failed to start");
    }

    // Setup web server
    server.on("/", handleRoot);
    server.on("/lamp", handleLamp);
    server.on("/brightness", handleBrightness);
    server.on("/status", handleStatus);
    server.begin();
    Serial.println("✓ Web server started (port 80)");

    // Start HomeKit server
    Serial.println("\n🏠 Starting HomeKit server...");
    Serial.println("   This may take 10-15 seconds...");

    arduino_homekit_setup(&config);

    Serial.println("✓ HomeKit server started!");

    // Print summary
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║          🎉 SYSTEM READY! 🎉          ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println("\n📱 APPLE HOME SETUP:");
    Serial.println("   1. Open Home app on iPhone/iPad");
    Serial.println("   2. Tap '+' → Add Accessory");
    Serial.println("   3. Tap 'More options...'");
    Serial.println("   4. Select 'ESP8266 Lamp'");
    Serial.println("   5. Tap 'Add Anyway'");
    Serial.println("   6. Enter code: 111-22-333");
    Serial.println("   7. Choose room and finish setup");
    Serial.println("\n🗣️  VOICE CONTROL:");
    Serial.println("   'Hey Siri, turn on the lamp'");
    Serial.println("   'Hey Siri, set lamp to 50 percent'");
    Serial.println("   'Hey Siri, turn off the lamp'");
    Serial.println("\n🌐 WEB CONTROL:");
    Serial.print("   http://");
    Serial.println(WiFi.localIP());
    Serial.print("   http://");
    Serial.print(HOSTNAME);
    Serial.println(".local");
    Serial.println("\n════════════════════════════════════════\n");
}

// ========================================================================
// MAIN LOOP
// ========================================================================
void loop() {
    arduino_homekit_loop();
    server.handleClient();
    MDNS.update();

    // Optional: Add a watchdog to reconnect WiFi if lost
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {  // Check every 30 seconds
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("⚠️  WiFi disconnected! Reconnecting...");
            WiFi.reconnect();
        }
        lastCheck = millis();
    }
}
