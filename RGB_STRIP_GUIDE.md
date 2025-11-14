# 🌈 RGB LED Strip Setup Guide

## ESP8266 Smart RGB Lamp v2.1

Complete guide for controlling addressable LED strips (WS2812B, NeoPixels) with Apple HomeKit!

---

## ✨ What You Can Do

### Full Color Control
- 🎨 **16.7 Million Colors** - Complete RGB spectrum
- 🍎 **Apple HomeKit** - Control with Home app and Siri
- 🗣️ **Voice Commands** - "Hey Siri, set lamp to purple"
- 🌐 **Web Interface** - Color picker and presets
- 📱 **Cross-Platform** - iPhone, iPad, Mac, Apple Watch

### Features
- ✅ Hue, Saturation, Brightness control
- ✅ 8 Color Scene Presets
- ✅ 3 Dynamic Effects (Rainbow, Fire, Breathing)
- ✅ Physical button control
- ✅ OTA wireless updates
- ✅ WiFi Manager (easy setup)
- ✅ Persistent settings
- ✅ Smooth color transitions

---

## 🛒 What You Need

### Hardware

#### Required:
1. **ESP8266 Board**
   - NodeMCU, Wemos D1 Mini, or ESP8266MOD
   - ~$3-5 on Amazon/AliExpress

2. **WS2812B LED Strip** (Addressable RGB)
   - 30, 60, or 144 LEDs per meter
   - Any length (adjust `NUM_LEDS` in code)
   - ~$10-20 for 1-5 meters
   - **Aliases**: NeoPixels, WS2813, SK6812
   - Must be **addressable** (each LED controllable)

3. **5V Power Supply**
   - **CRITICAL**: LED strips need external power!
   - Calculate: ~60mA per LED at full white
   - 30 LEDs = 2A minimum (use 3A adapter)
   - 60 LEDs = 4A minimum (use 5A adapter)
   - Quality 5V adapter with barrel jack

4. **Wires**
   - 3 wires: Data, 5V, GND
   - 22-18 AWG wire recommended
   - Male-to-female jumper wires

#### Optional:
- **Physical Button** - For manual control
- **Capacitor** - 1000µF 6.3V+ (protects LEDs)
- **Resistor** - 470Ω (for data line)
- **Enclosure** - 3D printed case

### Software

Install these libraries in Arduino IDE:

1. **HomeKit-ESP8266** by Mixiaoxiao
2. **WiFiManager** by tzapu
3. **ArduinoJson** by Benoit Blanchon
4. **FastLED** by Daniel Garcia ⭐ (for LED control)

---

## 🔌 Wiring Diagram

### Basic Connection (Minimum)

```
ESP8266          WS2812B LED Strip          5V Power Supply
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

GPIO2 (D4) ────→ DI/Data In

                 5V/VCC     ←────────────── 5V (+)

GND        ────→ GND        ←────────────── GND (-)
           │
           └──────────────────────────────→ GND (-)
                                         (Common ground!)
```

### Recommended Connection (Best Practice)

```
                                         5V Power Supply
                                               │
                                               ├─→ 5V
                                               └─→ GND
                                                    │
                                                    │
ESP8266                                      ┌──────┴──────┐
                                            │              │
GPIO2 ──→ 470Ω Resistor ──→ Data In        5V            GND
                             │              │              │
GND   ─────────────────────→ GND ──────────┘              │
                                                           │
                          Capacitor                        │
                          1000µF                           │
                          ┌─(+)────────────────────────────┘
                          └─(-)────────────────────────────┐
                                                            │
                                                           GND
```

### Component Explanations:

**Resistor (470Ω)**:
- Protects ESP8266 and first LED
- Place between GPIO2 and LED Data pin
- Can use 220Ω - 680Ω

**Capacitor (1000µF)**:
- Smooths power supply voltage
- Protects LEDs from voltage spikes
- Connect across 5V and GND near LED strip
- Must respect polarity! (+) to 5V, (-) to GND

**Common Ground**:
- **CRITICAL**: ESP8266 and Power Supply must share GND
- Without common ground = LED strip won't work!

---

## ⚡ Power Requirements

### Calculate Power Needed:

```
Power (mA) = Number of LEDs × 60mA
Power (A) = Power (mA) ÷ 1000

Examples:
- 30 LEDs: 30 × 60 = 1800mA = 1.8A → Use 2-3A adapter
- 60 LEDs: 60 × 60 = 3600mA = 3.6A → Use 4-5A adapter
- 144 LEDs: 144 × 60 = 8640mA = 8.6A → Use 10A adapter
```

**Safety Margin**: Use adapter rated 20-50% higher than calculation

### Power Supply Options:

1. **Phone Charger** (up to ~30 LEDs)
   - 5V 2-3A USB charger
   - Cut USB cable, connect wires
   - Quick test setup

2. **LED Strip Power Adapter** (30-300 LEDs)
   - 5V 3A, 5A, or 10A
   - Barrel jack connector
   - Dedicated LED power supplies

3. **PC Power Supply** (Large installations)
   - Old computer PSU
   - 5V rail can provide 10-30A
   - Overkill but reliable

⚠️ **NEVER power LED strip from ESP8266 3.3V or 5V pins!**
- ESP8266 can't handle the current
- Will damage the board
- Always use external power supply

---

## 💻 Software Setup

### Step 1: Install Libraries

Arduino IDE → Sketch → Include Library → Manage Libraries:

1. Search: **FastLED** → Install
2. Search: **HomeKit-ESP8266** → Install
3. Search: **WiFiManager** → Install
4. Search: **ArduinoJson** → Install

### Step 2: Configure Code

Open `ESP8266_HomeKit_RGB_Strip.ino` and edit:

**Line 43-44**: LED Strip Settings
```cpp
#define NUM_LEDS 30          // ⚠️ Change to your LED count!
#define LED_PIN 2            // GPIO2 (D4 on NodeMCU)
```

**Line 45-47**: LED Type (usually don't need to change)
```cpp
#define LED_TYPE WS2812B     // WS2812B, WS2813, SK6812
#define COLOR_ORDER GRB      // GRB for most strips
#define MAX_BRIGHTNESS 255   // 0-255 (255 = 100%)
```

### Step 3: Board Settings

**Tools Menu:**
- Board: "Generic ESP8266 Module" or "NodeMCU 1.0"
- Flash Size: "4MB (FS:1MB OTA:~1019KB)"
- CPU Frequency: "160 MHz"
- Upload Speed: "115200"
- Port: Your COM port

### Step 4: Upload

1. Connect ESP8266 via USB
2. Click **Upload**
3. Wait for "Done uploading"
4. Open Serial Monitor (115200 baud)

### Step 5: WiFi Setup

1. ESP8266 creates WiFi: **ESP8266-RGB-Setup**
2. Password: `12345678`
3. Connect and configure your home WiFi
4. ESP8266 reboots and connects

### Step 6: Test LEDs

After WiFi connects:
- LEDs should do a blue startup animation
- Open web interface: `http://esp8266-rgb.local`
- Try turning on and changing colors!

### Step 7: Add to HomeKit

1. Open **Home** app on iPhone
2. Tap **+** → **Add Accessory**
3. Select **RGB Lamp**
4. Enter code: **111-22-333**
5. Done!

---

## 🎨 Using Color Control

### Via Siri:

```
"Hey Siri, turn on the RGB lamp"
"Hey Siri, set RGB lamp to red"
"Hey Siri, set RGB lamp to blue at 50%"
"Hey Siri, make RGB lamp purple"
"Hey Siri, dim the RGB lamp"
```

### Via Home App:

1. Tap lamp icon
2. Long press for full controls
3. Use color wheel for any color
4. Adjust brightness slider

### Via Web Interface:

- `http://esp8266-rgb.local`
- Hue slider (0-360°)
- Saturation slider (0-100%)
- Brightness slider (0-100%)
- Live color preview

### Via Physical Button:

- **Short press** → Toggle ON/OFF
- **Long press (3s)** → Cycle color scenes
- **Hold 10s** → Factory reset

---

## 🎭 Color Scenes

8 pre-configured scenes accessible via:
- Web UI scene buttons
- Button long-press
- Siri: "Set lamp to X percent" (for brightness matching scene)

| Scene | Color | Use Case |
|-------|-------|----------|
| **Warm White** | 30° hue, 20% sat | Cozy home atmosphere |
| **Cool White** | 200° hue, 10% sat | Bright work lighting |
| **Energize** | Green (120°) | Morning wake-up |
| **Concentrate** | Blue (200°) | Focus & productivity |
| **Reading** | Warm (40°) | Comfortable reading |
| **Relax** | Purple (260°) | Evening wind-down |
| **Sleep** | Dim blue (240°) | Night light |
| **Romance** | Soft pink (340°) | Romantic mood |

---

## ✨ Dynamic Effects

3 animated effects via web interface:

### 1. 🌈 Rainbow
- Cycling rainbow colors
- Smooth transitions
- Great for parties

### 2. 🔥 Fire
- Flickering fire effect
- Random orange/yellow hues
- Campfire atmosphere

### 3. 💨 Breathing
- Slow fade in/out
- Uses current color
- Calming effect

**Note**: Effects don't work via HomeKit (HomeKit shows solid color)
Access via web interface only.

---

## 🛠️ Troubleshooting

### LEDs Don't Light Up

**Check:**
1. ✅ Power supply connected and ON?
2. ✅ 5V and GND to LED strip?
3. ✅ Data wire to GPIO2?
4. ✅ Common ground between ESP8266 and power supply?
5. ✅ Correct `NUM_LEDS` in code?
6. ✅ LED strip type correct? (WS2812B vs WS2813)

**Test:**
- Measure voltage at LED strip: should be 4.8-5.2V
- Try different GPIO pin (change `LED_PIN`)
- Swap data wire with another LED strip (if available)

### Wrong Colors

**Fix:**
- Change `COLOR_ORDER` in code
- Try: GRB, RGB, BRG
- Most WS2812B use GRB

### First LED Wrong, Rest OK

**Fix:**
- First LED might be dead
- Cut off first LED, connect to second
- Or replace first LED

### LEDs Flicker

**Causes:**
- Insufficient power supply
- Voltage drop on long strips
- No capacitor

**Fix:**
- Use higher amperage power supply
- Add 1000µF capacitor
- For long strips: inject power every 1-2 meters

### Random Colors/Glitches

**Fix:**
- Add 470Ω resistor on data line
- Shorten data wire (< 1 meter)
- Use shielded wire
- Add 0.1µF capacitor at ESP8266 power pins

### HomeKit "No Response"

**Fix:**
- Check WiFi connection
- Restart ESP8266
- Remove and re-add in Home app
- Check Serial Monitor for errors

### OTA Upload Fails

**Fix:**
- Power supply must handle both ESP8266 and LEDs
- Disable effects during upload
- Use wired USB upload instead

---

## 🎯 Advanced Customization

### Change LED Count

Line 43:
```cpp
#define NUM_LEDS 60  // Change to your count
```

### Adjust Brightness Limit

Line 47:
```cpp
#define MAX_BRIGHTNESS 200  // Lower = dimmer max (0-255)
```

### Change Data Pin

Line 44:
```cpp
#define LED_PIN 5  // Use GPIO5 (D1) instead
```

Safe pins: 4, 5, 12, 13, 14

### Add More Scenes

1. Add to `scenes` array (line 104)
2. Add button in HTML (line 859)
3. Format: `{name, hue, saturation, brightness}`

Example:
```cpp
{"Ocean", 180, 100, 60},  // Cyan blue
```

### Modify Effects

Edit functions:
- `updateRainbow()` - Line 277
- `updateFire()` - Line 284
- `updateBreathing()` - Line 291

Create your own effects!

---

## 📊 LED Strip Types Comparison

| Type | Addressable? | Works? | Notes |
|------|-------------|--------|-------|
| **WS2812B** | ✅ Yes | ✅ Perfect | Most common, recommended |
| **WS2813** | ✅ Yes | ✅ Perfect | Like WS2812B with backup line |
| **SK6812** | ✅ Yes | ✅ Perfect | Similar to WS2812B |
| **WS2811** | ✅ Yes | ✅ Perfect | External IC version |
| **APA102** | ✅ Yes | ⚠️ Different library | Needs different code |
| **RGB Strip** | ❌ No | ❌ No | Needs 3 pins + MOSFETs |
| **Single RGB LED** | ❌ No | ❌ No | Use common cathode code |

**Recommended**: WS2812B (cheapest, most common, easiest)

---

## 💡 Project Ideas

### Home Automation
- Sync with door sensor
- Flash red on alarm
- Wake-up light (sunrise simulation)
- Bedtime routine (sunset simulation)

### Entertainment
- Music visualizer (add microphone)
- Gaming RGB (sync with PC)
- TV backlight (ambilight effect)
- Party mode (effects + scenes)

### Practical
- Under-cabinet kitchen lighting
- Desk RGB accent light
- Closet motion-activated light
- Staircase lighting

### Advanced
- Multiple zones (separate strips)
- MQTT integration
- Home Assistant connection
- Web API for custom control

---

## 🔒 Safety Notes

### Electrical Safety:
- ✅ Use proper 5V power supply (no makeshift solutions)
- ✅ Don't exceed power supply rating
- ✅ Ensure good connections (solder if possible)
- ✅ Keep away from water
- ⚠️ Never use damaged power supplies
- ⚠️ Don't connect AC voltage directly!

### LED Strip Safety:
- ✅ Mount on aluminum channel (heat dissipation)
- ✅ Avoid covering LEDs (fire risk at high brightness)
- ✅ Use wire appropriate for current
- ⚠️ Full white at 100% = maximum heat
- ⚠️ Long-term 100% brightness reduces lifespan

---

## 📏 Strip Length Guidelines

| LEDs | Length (@30/m) | Length (@60/m) | Power Needed | Difficulty |
|------|---------------|---------------|--------------|------------|
| 10-30 | 0.3-1m | 0.15-0.5m | 2-3A | Easy |
| 30-60 | 1-2m | 0.5-1m | 3-5A | Easy |
| 60-150 | 2-5m | 1-2.5m | 5-10A | Medium |
| 150-300 | 5-10m | 2.5-5m | 10-20A | Hard* |

*For 150+ LEDs: Need power injection every 1-2 meters

---

## 🎬 Quick Start Checklist

- [ ] ESP8266 board
- [ ] WS2812B LED strip
- [ ] 5V power supply (correct amperage)
- [ ] Wires (data, 5V, GND)
- [ ] FastLED library installed
- [ ] Other 3 libraries installed
- [ ] `NUM_LEDS` set correctly in code
- [ ] Code uploaded successfully
- [ ] LEDs light up with blue animation
- [ ] WiFi configured
- [ ] Web interface accessible
- [ ] Colors change via web UI
- [ ] Added to HomeKit
- [ ] Siri voice control works

---

## 📞 Support

**LEDs not working?**
1. Check Serial Monitor output
2. Verify all connections
3. Test power supply voltage
4. Confirm LED strip type
5. Try example FastLED sketch first

**HomeKit issues?**
- See main troubleshooting guide
- Check `PRODUCTION_GUIDE.md`

---

**Made with 🌈 for colorful smart homes**

Enjoy your full-color RGB smart lamp! ✨
