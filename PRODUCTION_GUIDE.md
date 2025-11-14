# 🚀 Production Version Setup Guide

## ESP8266 Smart Lamp Pro v2.0

This guide covers the **production-ready version** with advanced features.

---

## 🆕 What's New in v2.0

### Core Features
- ✅ **OTA Updates** - Upload firmware wirelessly (no USB needed!)
- ✅ **WiFi Manager** - Easy setup via captive portal (no hardcoded WiFi passwords)
- ✅ **Persistent Storage** - Remembers state after power loss
- ✅ **Physical Button** - Manual control without phone
- ✅ **Watchdog Protection** - Auto-recovery from crashes
- ✅ **Auto WiFi Reconnect** - Resilient connection handling

### Smart Features
- ✅ **5 Scene Presets**:
  - 🌟 **Bright** (100%) - Full brightness
  - 📖 **Reading** (70%) - Comfortable reading light
  - 😌 **Relax** (40%) - Ambient lighting
  - 🌙 **Night** (10%) - Gentle night light
  - ✨ **Custom** - Your own brightness level

- ✅ **Smooth Fading** - Professional fade in/out transitions
- ✅ **Enhanced Web UI** - Full-featured control panel
- ✅ **Status LED** - Visual feedback for system state
- ✅ **Factory Reset** - Reset to defaults via button

---

## 📋 Requirements

### Hardware
- **ESP8266 board** (ESP8266MOD, NodeMCU, Wemos D1 Mini)
- **USB cable** for initial programming
- **Physical button** (optional but recommended)
- **LED or Relay module** for lamp control
- **Status LED** (optional, can use built-in)

### Software & Libraries
Install these libraries in Arduino IDE (**Sketch → Include Library → Manage Libraries**):

1. **HomeKit-ESP8266** by Mixiaoxiao
2. **WiFiManager** by tzapu (v2.0.x or newer)
3. **ArduinoJson** by Benoit Blanchon (v6.x)

ESP8266 board support (if not already installed):
- **esp8266** by ESP8266 Community

---

## 🔌 Hardware Wiring

### Required Connections
```
ESP8266 Pin    →    Component
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO2 (D4)     →    Lamp/Relay IN
GPIO0 (D3)     →    Button → GND
GPIO16 (D0)    →    Status LED (+) → 220Ω → GND
VCC/3.3V       →    Relay VCC (if using relay)
GND            →    Common Ground
```

### Button Wiring (Recommended)
```
                ┌──────┐
GPIO0 (D3) ────┤ BTN  ├──── GND
                └──────┘
(Uses internal pull-up resistor)
```

### Relay Wiring (For AC/DC Lamp)
```
ESP8266          Relay Module
━━━━━━━━━━━━━━━━━━━━━━━━━━━━
3.3V      →      VCC
GND       →      GND
GPIO2     →      IN

Relay Output (to lamp):
AC/DC Source → COM
NO → Lamp
```

⚠️ **WARNING**: Working with AC voltage is dangerous! Only attempt if qualified.

### Status LED (Optional)
```
GPIO16 → 220Ω Resistor → LED (+) → LED (-) → GND
```

---

## 🛠️ Installation Steps

### Step 1: Install Libraries

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search and install:
   - `HomeKit-ESP8266` by Mixiaoxiao
   - `WiFiManager` by tzapu
   - `ArduinoJson` by Benoit Blanchon

### Step 2: Configure Board Settings

**Tools Menu:**
- **Board**: "Generic ESP8266 Module" or "NodeMCU 1.0"
- **Flash Size**: **"4MB (FS:1MB OTA:~1019KB)"** ⚠️ CRITICAL for OTA!
- **CPU Frequency**: "160 MHz"
- **Upload Speed**: "115200"
- **Port**: Your COM port

### Step 3: Upload Firmware

1. Open `ESP8266_HomeKit_Production.ino`
2. Connect ESP8266 via USB
3. Click **Upload** (→ arrow button)
4. Wait for "Done uploading"
5. Open **Serial Monitor** (115200 baud)

### Step 4: Initial WiFi Setup

On **first boot**, the ESP8266 creates a WiFi access point:

1. **Connect** to WiFi: **ESP8266-Setup**
2. **Password**: `12345678`
3. Captive portal opens automatically (or go to `192.168.4.1`)
4. **Select your home WiFi** network
5. **Enter password**
6. Click **Save**
7. ESP8266 reboots and connects to your WiFi!

**Success!** The device will now connect automatically on every boot.

### Step 5: Add to Apple Home

1. Open **Home** app on iPhone/iPad
2. Tap **+** → **Add Accessory**
3. Tap **More options...**
4. Select **ESP8266 Lamp**
5. Tap **Add Anyway** (ignore warning)
6. Enter code: **111-22-333**
7. Choose room → Done!

### Step 6: Access Web Interface

Open browser and go to:
- `http://esp8266-lamp.local` (if mDNS works)
- Or use IP address shown in Serial Monitor

---

## 🎛️ Using the Features

### Physical Button Controls

| Action | Function |
|--------|----------|
| **Short Press** (< 3s) | Toggle lamp ON/OFF |
| **Long Press** (3-10s) | Cycle through scenes |
| **Very Long Press** (10s+) | Factory reset |

### Scene Presets

Access via:
- **Web UI**: Click scene buttons
- **Button**: Long press to cycle
- **Siri**: "Set lamp to 100%" (Bright), "Set lamp to 70%" (Reading), etc.

| Scene | Brightness | Best For |
|-------|-----------|----------|
| 🌟 Bright | 100% | Full illumination |
| 📖 Reading | 70% | Reading without strain |
| 😌 Relax | 40% | Ambient mood lighting |
| 🌙 Night | 10% | Gentle night light |
| ✨ Custom | Any % | Your preference |

### Voice Control (Siri)

```
"Hey Siri, turn on the lamp"
"Hey Siri, set the lamp to 50 percent"
"Hey Siri, turn off the lamp"
"Hey Siri, set the lamp to reading mode"  (70%)
```

### Web Interface

Navigate to `http://esp8266-lamp.local`:
- **ON/OFF buttons** - Control power
- **Brightness slider** - Adjust 0-100%
- **Scene buttons** - Quick presets
- **Real-time updates** - Syncs with HomeKit

---

## 🔄 OTA Updates (Wireless Firmware Upload)

### Using Arduino IDE

1. Make sure ESP8266 is powered and connected to WiFi
2. In Arduino IDE:
   - **Tools → Port** → Select **esp8266-lamp at [IP]**
   - Click **Upload**
3. Wait for upload to complete
4. ESP8266 automatically reboots with new firmware!

**No USB cable needed!** ✨

### OTA Password
Default: `admin` (change in code line 685)

---

## 💾 Persistent Settings

The following settings are **saved to flash** and persist after power loss:
- Lamp ON/OFF state
- Brightness level
- Current scene
- WiFi credentials (managed by WiFiManager)

Settings are saved when:
- Lamp state changes
- Brightness changes
- Scene is selected

---

## 🔴 Status LED Indicators

| Pattern | Meaning |
|---------|---------|
| **Solid ON** | Normal operation |
| **Slow blink** (500ms) | Connecting to WiFi |
| **Fast blink** (100ms) | OTA update in progress |
| **Very fast blink** (50ms) | Factory reset in progress |

---

## 🔧 Troubleshooting

### WiFi Won't Connect

**Solution 1**: Factory Reset
- Hold button for 10 seconds
- LED blinks rapidly
- Device resets and creates "ESP8266-Setup" AP again

**Solution 2**: Clear WiFi via Serial Monitor
```
wifiManager.resetSettings();
ESP.restart();
```

### OTA Updates Fail

**Causes:**
- Wrong OTA password
- Not enough flash space
- Poor WiFi signal

**Solutions:**
- Check OTA password (default: `admin`)
- Verify Flash Size is **4MB (FS:1MB OTA:~1019KB)**
- Move ESP8266 closer to router

### Lamp Doesn't Turn On

1. Check wiring (GPIO2 → Lamp/Relay)
2. Check if logic is inverted (some relays need `HIGH` not `LOW`)
3. Verify power supply to relay module
4. Check Serial Monitor for errors

### HomeKit "No Response"

1. Check WiFi connection (Serial Monitor)
2. Restart ESP8266 (power cycle or reset button)
3. Verify iPhone is on same WiFi network
4. Remove and re-add accessory in Home app

### Button Not Working

1. Verify button wiring (GPIO0 → Button → GND)
2. Check Serial Monitor for "Button:" messages
3. Try different button (might be faulty)
4. Ensure pull-up is enabled (code has `INPUT_PULLUP`)

### Settings Not Saving

1. Check Serial Monitor for "Settings saved" messages
2. LittleFS might be corrupted - try factory reset
3. Flash might be full - check free space

### Memory Errors

HomeKit uses significant memory. If experiencing crashes:
- Set Flash Size to **4MB (FS:1MB)**
- Set CPU to **160 MHz**
- Restart ESP8266 regularly via power cycle

---

## 🔒 Security Notes

### Change Default Passwords

**OTA Password**: Line 685 in code
```cpp
ArduinoOTA.setPassword("your-secure-password");
```

**WiFi AP Password**: Line 935 in code
```cpp
wifiManager.autoConnect("ESP8266-Setup", "your-password");
```

**HomeKit Code**: Line 119 in code
```cpp
.password = "123-45-678"  // Format: XXX-XX-XXX
```

### Best Practices
- Change all default passwords
- Use strong WiFi password
- Keep firmware updated via OTA
- Don't expose device to public internet without proper security

---

## 📊 System Information

Access device info at: `http://esp8266-lamp.local/info`

Shows:
- Firmware version
- Chip ID
- Flash size
- Free memory
- Uptime
- WiFi signal strength
- IP address

---

## 🎯 Advanced Customization

### Adjust Fade Speed

Line 18-19:
```cpp
#define FADE_STEP_DELAY 20    // Decrease for faster fades
#define FADE_STEPS 50         // Adjust smoothness
```

### Change Scene Brightness

Lines 79-82:
```cpp
const int SCENE_BRIGHT = 100;   // Adjust as needed
const int SCENE_READING = 70;
const int SCENE_RELAX = 40;
const int SCENE_NIGHT = 10;
```

### Modify Button Timings

Lines 20-22:
```cpp
#define LONG_PRESS_TIME 3000        // 3s for scene cycle
#define FACTORY_RESET_TIME 10000    // 10s for reset
```

### Add More Scenes

1. Define new scene brightness constant
2. Add case in `applyScene()` function (line 191)
3. Add button in HTML UI (line 860)

---

## 📈 Performance Tips

### Optimize Memory
- Don't use `String` excessively (causes fragmentation)
- Use `PROGMEM` for large constants
- Clear unused variables

### Improve WiFi Stability
- Position ESP8266 near router
- Use 2.4GHz WiFi (not 5GHz)
- Avoid WiFi interference
- Use quality power supply (stable 5V)

### Extend Flash Lifetime
- Reduce save frequency if needed
- Settings save on change (already optimized)
- Flash has ~100,000 write cycles per sector

---

## 🐛 Debugging

### Enable Verbose Logging

Serial Monitor shows:
- Boot sequence
- WiFi connection
- HomeKit events
- Button presses
- State changes

### Common Serial Messages

```
✓ = Success
✗ = Error
⚠️ = Warning
🔘 = Button action
📱 = HomeKit command
💡 = Lamp state change
```

### Memory Check

Add to code to monitor heap:
```cpp
Serial.print("Free heap: ");
Serial.println(ESP.getFreeHeap());
```

---

## 🆚 Version Comparison

| Feature | Basic v1.0 | Production v2.0 |
|---------|-----------|----------------|
| HomeKit | ✅ | ✅ |
| Web UI | Basic | Enhanced |
| WiFi Setup | Hardcoded | WiFi Manager |
| OTA Updates | ❌ | ✅ |
| Persistent Storage | ❌ | ✅ |
| Physical Button | ❌ | ✅ |
| Scene Presets | ❌ | ✅ (5 scenes) |
| Smooth Fading | ❌ | ✅ |
| Watchdog | ❌ | ✅ |
| Factory Reset | ❌ | ✅ |
| Status LED | ❌ | ✅ |
| Auto Reconnect | Basic | Advanced |
| Production Ready | ❌ | ✅ |

---

## 🚀 Next Steps

After basic setup works:

1. **Customize scenes** to your preference
2. **Change default passwords** for security
3. **Add physical button** for convenient control
4. **Test OTA updates** to ensure they work
5. **Create automation** in Home app
6. **Add to scenes/rooms** in HomeKit
7. **Share with family** via Home app

---

## 📞 Support

**Issues?** Check:
- Serial Monitor output
- `/info` endpoint
- Troubleshooting section above
- Original code comments

**Feature Requests?**
Fork the project and customize! The code is well-commented and modular.

---

**Made with ❤️ for production IoT deployments**

Enjoy your professional-grade smart lamp! 💡✨
