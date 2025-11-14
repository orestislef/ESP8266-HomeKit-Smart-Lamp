# 💡 ESP8266 HomeKit Smart Lamp

Transform your ESP8266 into an Apple HomeKit-compatible smart lamp with Siri voice control and web interface!

![ESP8266](https://img.shields.io/badge/ESP8266-Compatible-blue)
![Apple HomeKit](https://img.shields.io/badge/Apple-HomeKit-black)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-green)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Version](https://img.shields.io/badge/Version-2.0-brightgreen)
![Production Ready](https://img.shields.io/badge/Production-Ready-success)

## 🎯 Two Versions Available

### 🌟 **Production Version v2.0** (Recommended)
**File**: `ESP8266_HomeKit_Production.ino`

Full-featured, production-ready version with:
- ✅ OTA Updates (wireless firmware upload)
- ✅ WiFi Manager (easy setup, no hardcoded passwords)
- ✅ Persistent Storage (remembers state after power loss)
- ✅ Physical Button (toggle, scenes, factory reset)
- ✅ 5 Scene Presets (Bright, Reading, Relax, Night, Custom)
- ✅ Smooth Fading transitions
- ✅ Watchdog protection
- ✅ Factory reset capability
- ✅ Enhanced web UI

**📖 [Read Production Setup Guide →](PRODUCTION_GUIDE.md)**

### 📦 **Basic Version v1.0**
**File**: `ESP8266_HomeKit_Alternative.ino`

Simple, straightforward version - perfect for beginners:
- ✅ Apple HomeKit integration
- ✅ Siri voice control
- ✅ Web interface
- ✅ Brightness control
- ✅ Easy to understand code

**Great for**: Learning, simple projects, minimal features needed

---

## ✨ Core Features (Both Versions)

- 🍎 **Apple HomeKit Integration** - Add to Home app and control with Siri
- 🗣️ **Voice Control** - "Hey Siri, turn on the lamp"
- 🌐 **Web Interface** - Beautiful responsive web UI for control
- 📱 **Cross-Platform** - Works on iPhone, iPad, Mac, Apple Watch
- 🎚️ **Brightness Control** - Adjustable from 0-100%
- 🔄 **Auto-Reconnect** - Automatically reconnects if WiFi drops
- 📊 **Real-time Updates** - Syncs between Home app and web interface
- 🎨 **Modern UI** - Sleek gradient design with animations

## 🎥 Demo

Say "Hey Siri, turn on the lamp" and watch it work! ✨

## 🤔 Which Version Should I Use?

| Choose **Production v2.0** if you want: | Choose **Basic v1.0** if you want: |
|----------------------------------------|-----------------------------------|
| ✅ Wireless firmware updates (OTA) | ✅ Simplest possible setup |
| ✅ Easy WiFi setup (no coding) | ✅ Learning-friendly code |
| ✅ Physical button control | ✅ Minimal features |
| ✅ Settings that survive power loss | ✅ Quick prototype |
| ✅ Professional features | ✅ Hardcoded WiFi is OK |
| ✅ Production deployment | ✅ Basic functionality only |

**Recommendation**: Start with **Production v2.0** for best experience!

## 📋 Requirements

### Hardware
- **ESP8266 board** (ESP8266MOD, NodeMCU, Wemos D1 Mini, etc.)
- **USB cable** for initial programming
- **LED or relay module** (optional - has built-in LED for testing)
- **Physical button** (recommended for Production v2.0)
- **220Ω resistor** (if using external LED)

### Software
- **[Arduino IDE](https://www.arduino.cc/en/software)** (1.8.19 or newer)
- **ESP8266 board support**
- **HomeKit-ESP8266** library by Mixiaoxiao

**Production v2.0 also needs:**
- **WiFiManager** library by tzapu
- **ArduinoJson** library by Benoit Blanchon

### Other
- **2.4GHz WiFi network** (ESP8266 doesn't support 5GHz)
- **iPhone/iPad** with Home app (iOS 10 or newer)

## 🚀 Quick Start

### For Production v2.0 (Recommended)
**📖 See detailed guide:** [PRODUCTION_GUIDE.md](PRODUCTION_GUIDE.md)

**Quick steps:**
1. Install Arduino IDE + ESP8266 board support
2. Install libraries: HomeKit-ESP8266, WiFiManager, ArduinoJson
3. Open `ESP8266_HomeKit_Production.ino`
4. Upload to ESP8266
5. Connect to "ESP8266-Setup" WiFi and configure
6. Add to Apple Home app with code: **111-22-333**

### For Basic v1.0

### 1. Install Arduino IDE & ESP8266 Support

1. Download and install [Arduino IDE](https://www.arduino.cc/en/software)
2. Open Arduino IDE → **File → Preferences**
3. Add to "Additional Boards Manager URLs":
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search "ESP8266" and install **esp8266 by ESP8266 Community**

### 2. Install Required Library

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for: **HomeKit-ESP8266**
3. Install **HomeKit-ESP8266 by Mixiaoxiao**
4. Click **Install All** when asked for dependencies

### 3. Configure WiFi

1. Open `ESP8266_HomeKit_Alternative.ino`
2. Edit lines 33-34 with your WiFi credentials:
   ```cpp
   #define WIFI_SSID "YourWiFiName"        // Your WiFi network name
   #define WIFI_PASSWORD "YourPassword"     // Your WiFi password
   ```
   ⚠️ **Important**: WiFi must be 2.4GHz (ESP8266 doesn't support 5GHz)

### 4. Configure Board Settings

Go to **Tools** menu and set:
- **Board**: "Generic ESP8266 Module" or "NodeMCU 1.0"
- **Flash Size**: "4MB (FS:2MB OTA:~1019KB)" ⚠️ Important!
- **CPU Frequency**: "160 MHz" ⚠️ Important!
- **Upload Speed**: "115200"
- **Port**: Select your COM port

### 5. Upload Code

1. Connect ESP8266 to computer via USB
2. Click **Upload** button (→ arrow)
3. Wait for "Done uploading"
4. Open **Serial Monitor** (Tools → Serial Monitor)
5. Set baud rate to **115200**
6. You should see WiFi connection and IP address

### 6. Add to Apple Home

On your iPhone/iPad:

1. Open **Home** app
2. Tap **+** (top right) → **Add Accessory**
3. Tap **More options...**
4. Select **ESP8266 Lamp**
5. Tap **Add Anyway** (ignore "not certified" warning - this is normal for DIY devices)
6. Enter setup code: **111-22-333**
7. Choose a room and finish setup

### 7. Start Controlling!

**Voice Control:**
- "Hey Siri, turn on the lamp"
- "Hey Siri, set the lamp to 50 percent"
- "Hey Siri, turn off the lamp"

**Web Control:**
- Open browser and go to: `http://esp8266-lamp.local`
- Or use IP address shown in Serial Monitor

## 📁 Project Files

- **ESP8266_HomeKit_Alternative.ino** - Main HomeKit version (⭐ Recommended)
- **ESP8266_HomeKit_Lamp.ino** - Alternative HomeKit version (HomeSpan library)
- **ESP8266_WebServer.ino** - Basic web-only version (no HomeKit)
- **WHICH_VERSION_TO_USE.txt** - Detailed comparison and setup guide

## 🔌 Wiring Diagrams

### Built-in LED (Testing)
No wiring needed! Code uses GPIO2 (built-in LED on most ESP8266 boards).

### External LED
```
ESP8266 GPIO2 → 220Ω Resistor → LED+ (long leg) → LED- → GND
```

### Relay Module (Control AC/DC Devices)
```
ESP8266 3.3V → Relay VCC
ESP8266 GND  → Relay GND
ESP8266 GPIO2 → Relay IN
```

Then connect your lamp to relay COM and NO terminals.

⚠️ **WARNING**: Working with AC voltage is dangerous! Only attempt if you're qualified. For beginners, use LED or 12V DC lamp.

### Safe GPIO Pins
- ✅ GPIO 4, 5, 12, 13, 14 (safe to use)
- ⚠️ GPIO 0, 2, 15 (used for boot mode)
- ❌ GPIO 1, 3 (serial communication)

## 🛠️ Customization

### Change Device Name
```cpp
#define DEVICE_NAME "Living Room Lamp"  // Line 29
```

### Change Setup Code
```cpp
.password = "123-45-678"  // Line 62 (format: XXX-XX-XXX)
```

### Change GPIO Pin
```cpp
#define LAMP_PIN 5  // Use GPIO5 instead of GPIO2
```

### Invert Logic (if LED/relay works opposite)
In `updateLamp()` function:
```cpp
// Change from:
analogWrite(LAMP_PIN, 1023 - pwmValue);
// To:
analogWrite(LAMP_PIN, pwmValue);
```

## 🐛 Troubleshooting

### WiFi Connection Issues
- ✅ Verify WiFi name and password are correct (case-sensitive)
- ✅ Ensure WiFi is 2.4GHz (check router settings)
- ✅ Move ESP8266 closer to router
- ✅ Check Serial Monitor for error messages

### Can't Find Device in Home App
- ✅ Wait 30 seconds after ESP8266 boots
- ✅ Ensure iPhone is on SAME WiFi network
- ✅ Check Serial Monitor shows "HomeKit server started"
- ✅ Restart Home app
- ✅ Press RESET button on ESP8266

### "No Response" in Home App
- ✅ ESP8266 lost WiFi - press RESET button
- ✅ Check WiFi is connected (Serial Monitor)
- ✅ Ensure router hasn't restarted
- ✅ Wait 30 seconds after reset

### Compilation Errors
- ✅ Install "HomeKit-ESP8266" library
- ✅ Set Flash Size to 4MB
- ✅ Set CPU Frequency to 160MHz
- ✅ Update ESP8266 board package

### LED Works Backwards (ON=OFF)
- ✅ This is inverted logic - see Customization section above

## 🎯 Future Enhancements

- [ ] RGB color support
- [ ] Motion sensor integration
- [ ] Temperature/humidity monitoring
- [ ] Scheduling/timer functionality
- [ ] Energy usage monitoring
- [ ] OTA (Over-The-Air) updates
- [ ] Multiple lamp control
- [ ] Scene presets (Reading, Movie, Party mode)

## 📚 Resources

- [ESP8266 Arduino Documentation](https://arduino-esp8266.readthedocs.io)
- [HomeKit-ESP8266 Library](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266)
- [Apple HomeKit Specification](https://developer.apple.com/homekit/)
- [ESP8266 Community Forum](https://www.esp8266.com)

## 📝 Version History

### v1.0 (Initial Release)
- Apple HomeKit integration
- Web interface control
- Brightness adjustment
- Siri voice control
- Auto WiFi reconnection
- mDNS support

## ⚖️ License

This project is licensed under the MIT License - feel free to use, modify, and distribute!

## 🙏 Acknowledgments

- HomeKit-ESP8266 library by Mixiaoxiao
- ESP8266 community
- Arduino community

## 📬 Support

Having issues? Check the detailed troubleshooting guide inside the `.ino` files or create an issue in this repository.

---

**Made with ❤️ for the ESP8266 and HomeKit community**

⭐ If you find this project useful, please star it!
