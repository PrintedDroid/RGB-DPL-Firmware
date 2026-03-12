# RGB Data Port Lights (RGB-DPL) Controller v5.8
**Advanced ESP32-S3 based RGB LED display controller for R2-D2 Data Port Lights, CBI, and Large Logic Display panels**

## 🤖 Project Overview

This controller brings your R2-D2's body panels to life with fully addressable RGB LEDs, replacing older MAX7219-based designs with far greater flexibility, color control, and dynamic animations. Built on the ESP32-S3 platform, it drives six independent WS2811/WS2812B LED strips across all major display panels: DPL (Data Port Lights), CBI (Charge Bay Indicator), and the Large Logic Display.

Designed for builders who want film-accurate animations with the power to customize every color, speed, and behavior.

### Key Features

- **🎨 Fully Addressable RGB** - 144 total WS2811/WS2812B LEDs across 6 independent strips
- **🌈 8 Color Schemes** - Classic, Blue, Pink, Green, Cyberpunk, Forest, Sunset, Custom
- **🧠 5 Personality Modes** - Normal, Happy, Grumpy, Excited, Sleepy (affect speed, hue, and behavior)
- **💾 5 User Profiles** - Persistent EEPROM storage with checksummed integrity
- **🚪 Door Sensor Integration** - Automatic panel activation/deactivation via left and right door switches
- **⚡ Voltage Monitor** - Universal 12V/24V battery monitoring with CBI status LED indicators
- **📡 I2C Slave Interface** - Address 0x20 for integration with MarcDuino/BetterDuino systems
- **⌨️ Serial CLI** - Comprehensive command-line interface with ANSI color terminal support
- **🎬 Sequence Mode** - Automatic cycling through animation modes
- **🔄 5 TopBlocks Animations** - Classic, Horizontal, Paired, Chaos, Single Pixel
- **🖥️ 7 CBI Display Modes** - Organic, ESB, Rainbow, Sparkle, Heart, Smiley, Matrix Rain
- **🔮 4 Large Logic Modes** - Breathing, Rainbow, Off, Personality-driven
- **🐕 Watchdog Timer** - Auto-recovery from hangs (10-second timeout)
- **🚀 Boot Sequence** - Rainbow wave startup animation with white flash confirmation

---

## 📝 Changelog

### Version 5.8 (ESP32-S3 Final)

**Stability, Safety & Code Quality Improvements**

This release focuses on robustness, thread safety, and platform compatibility for the ESP32-S3.

#### 🐛 Bug Fixes

**1. PROGMEM String Access Fixed**
- Corrected PROGMEM string access patterns for ESP32 compatibility
- Color scheme names now properly read via `pgm_read_ptr()`

**2. ANSI Escape Codes Corrected**
- Fixed ANSI escape sequences for proper terminal color rendering
- Uses `\x1b` escape format for cross-platform compatibility

**3. Pattern Array Sizes Corrected**
- Fixed ESB pattern array dimensions to match actual data (28 patterns x 20 elements)

#### ⚡ Improvements

**4. I2C Critical Sections**
- Added `portENTER_CRITICAL_ISR` / `portEXIT_CRITICAL_ISR` for thread-safe ISR operations
- Prevents race conditions between I2C receive interrupt and main loop processing

**5. Buffer Overflow Protection**
- Serial buffer enforces 64-byte limit with explicit overflow error messages
- Prevents memory corruption from excessively long commands

**6. Overflow-Safe Timers**
- All `millis()` comparisons use subtraction-based elapsed time calculation
- Prevents 49-day rollover bugs in long-running installations

**7. Watchdog Timer Support**
- Auto-detects ESP32 Arduino Core version (2.x vs 3.x)
- Uses appropriate WDT API for each core version
- 10-second timeout with panic-on-trigger for automatic recovery

**8. Named Constants**
- Converted all magic numbers to descriptive `#define` constants
- Improved code readability and maintainability

**9. System Health Monitoring**
- `INFO` command shows free heap memory, boot count, version, and total LED count
- Boot count tracked persistently across power cycles

**10. Optimized String Processing**
- Command parser works directly on buffer (no extra copy)
- Case-insensitive command matching via in-place `toupper()` conversion

---

### Previous Versions (Summary)

| Version | Changes |
|---------|---------|
| v5.7 | Added fifth TopBlocks animation mode (SINGLE_PIXEL) |
| v5.6 | Expanded TopBlocks to three modes (RANDOM, HORIZONTAL, PAIRED) |
| v5.3 | Enhanced serial help menu with ANSI color codes |
| v5.2 | Ported to ESP32-S3, added universal 12V/24V voltage monitor |
| v5.0 | Complete serial parser rewrite with robust C-style string handling |
| v4.0 | Advanced animation modes (Bargraph, Top Blocks, Smiley), Sequence Mode |
| v3.1 | User profiles, color schemes, and personalities with EEPROM storage |
| v2.0 | Initial RGB version for Arduino Nano, replacing MAX7219 controllers |

---

## 🔧 Hardware Requirements

### Core Components
- **ESP32-S3 Mini** development board (Lolin S3 Mini or equivalent)
- **WS2811/WS2812B LED strips** - 6 strips, 144 LEDs total
- **5V Power Supply** - Minimum 4A recommended (power limiter set to 4000mA)
- **Voltage divider** (optional) - 220k / 33k resistors for battery monitoring

### Optional Components
- **Door sensors** - 2x microswitch or magnetic reed switches (normally open)
- **I2C connection** - SDA/SCL to MarcDuino/BetterDuino (address 0x20)

---

## 📋 Pin Configuration

### ESP32-S3 Mini Pin Assignments

#### LED Data Pins

| Pin | Strip | LED Count | Function |
|-----|-------|-----------|----------|
| GPIO4 (D4) | Strip A | 8 LEDs | Bottom white lights (6) + Large red lights (2) |
| GPIO5 (D5) | Strip B | 28 LEDs | VU-Meter display |
| GPIO6 (D6) | Strip C | 24 LEDs | Right panel - Blue column (6) + Bargraph (18) |
| GPIO7 (D7) | Strip D | 18 LEDs | Top panel - Yellow blocks (9) + Green blocks (9) |
| GPIO8 (D8) | Strip E | 43 LEDs | Large Logic Display Panel |
| GPIO9 (D9) | Strip F | 23 LEDs | CBI - Matrix (20) + Status lights (3) |

#### Sensor & Monitor Pins

| Pin | Function | Notes |
|-----|----------|-------|
| GPIO10 | Left Door Sensor | INPUT_PULLUP, active LOW (controls Strips A-D) |
| GPIO11 | Right Door Sensor | INPUT_PULLUP, active LOW (controls Strip F / CBI) |
| GPIO12 | Voltage Monitor (ADC) | Analog input via voltage divider |

#### I2C Interface

| Pin | Function |
|-----|----------|
| SDA | I2C Data (default ESP32-S3 SDA pin) |
| SCL | I2C Clock (default ESP32-S3 SCL pin) |

**I2C Slave Address: `0x20`**

---

## 🚀 Installation

### Arduino IDE Setup

1. **Install ESP32 board support:**
   - File -> Preferences -> Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools -> Board -> Board Manager -> Search "ESP32" -> Install

2. **Board Configuration for ESP32-S3:**
   - Board: "Lolin S3 Mini" or "ESP32S3 Dev Module"
   - USB CDC On Boot: **"Enabled"** (CRITICAL - required for Serial output)
   - USB Mode: "Hardware CDC and JTAG"
   - Upload Mode: "UART0 / Hardware CDC"
   - Flash Size: "4MB" or "8MB" (depending on your board)
   - Partition Scheme: "Default 4MB with spiffs"

3. **Required Library:**
   Install via Arduino Library Manager:
   - **FastLED** (3.5.0+)

   Built-in libraries (no installation needed):
   - EEPROM
   - Wire (I2C)
   - esp_task_wdt (Watchdog)

### Upload Procedure

1. Hold the **BOOT** button while connecting USB
2. Click **Upload** in Arduino IDE
3. Release the BOOT button when upload starts
4. Press **RESET** button after upload completes

---

## 📡 Communication

### Serial Interface

| Parameter | Value |
|-----------|-------|
| Baud Rate | **57600** |
| Line Ending | Newline (`\n`) |
| Buffer Size | 64 bytes |
| Timeout | 500ms |
| Case Sensitivity | None (commands are case-insensitive) |

> **IMPORTANT:** The current baud rate is **57600**. If you plan to integrate this board with a **MarcDuino** or **BetterDuino** controller (which communicate at 9600 baud), you must change the `SERIAL_BAUD` define in the source code from `57600` to `9600` and re-upload.

### I2C Slave Interface

| Parameter | Value |
|-----------|-------|
| Slave Address | `0x20` |
| Protocol | Standard I2C receive |
| Thread Safety | Critical section protected (ISR-safe) |

The I2C interface allows external controllers (e.g., MarcDuino) to send single-byte commands to the RGB-DPL board. Commands received via I2C are processed in the main loop with thread-safe critical section protection to prevent race conditions between the I2C receive interrupt and normal operation.

### ANSI Color Terminal Support

The help menu and status output use ANSI escape codes for colored terminal output:
- **Cyan + Bold** - Section headers
- **Yellow + Bold** - Category headers
- **Standard** - Command descriptions

For best results, use a terminal that supports ANSI colors (e.g., PuTTY, Tera Term, or the Arduino IDE 2.x Serial Monitor). The Arduino IDE 1.x Serial Monitor does not render ANSI colors and will display raw escape sequences.

---

## ⌨️ Serial Command Reference

All commands are sent via the Serial Monitor at 57600 baud. Commands must end with a newline (`\n`). Commands are **case-insensitive**.

### System & Profile Commands

| Command | Description |
|---------|-------------|
| `CONFIG` | Enter configuration mode (Large Logic blinks blue as indicator) |
| `EXIT` | Exit configuration mode, resume normal animation |
| `HELP` | Display the full help menu with all commands |
| `STATUS` | Show current profile settings (brightness, speed, scheme, etc.) |
| `INFO` | Display system information (version, free heap, total LEDs, boot count) |
| `TEST` | Run a full diagnostic test on LEDs and sensors |
| `SAVE <1-5>` | Save current settings to profile slot 1-5 |
| `LOAD <1-5>` | Load settings from profile slot 1-5 |
| `DEFAULT <0-5>` | Set the default startup profile (0 = disabled, no auto-load) |

**Profile Examples:**
```
SAVE 3         -> Save current settings to profile 3
LOAD 1         -> Load profile 1
DEFAULT 2      -> Always load profile 2 on boot
DEFAULT 0      -> Disable auto-load (use built-in defaults on boot)
```

### Global Settings Commands

| Command | Description | Range |
|---------|-------------|-------|
| `BRIGHTNESS <value>` | Set master brightness | 1-100 (%) |
| `SPEED <value>` | Set DPL animation speed | 1-10 |
| `CBISPEED <value>` | Set CBI animation speed | 1-10 |
| `SEQUENCE <ON\|OFF>` | Enable/disable automatic animation cycling | ON / OFF |
| `VOLTAGE <ON\|OFF>` | Toggle CBI voltage monitor display | ON / OFF |
| `PERSONALITY <value>` | Set personality mode | 0-4 |
| `SCHEME <name\|number>` | Set color scheme by name or number | See table below |
| `COLOR <section> <R> <G> <B>` | Set custom RGB color for a specific LED section | Section: 0-11, RGB: 0-255 |

**Examples:**
```
BRIGHTNESS 50      -> Set brightness to 50%
SPEED 8            -> Fast DPL animations
CBISPEED 3         -> Moderate CBI speed
SEQUENCE ON        -> Enable auto-cycling
VOLTAGE ON         -> Show battery status on CBI LEDs
PERSONALITY 1      -> Set to Happy mode
SCHEME CYBERPUNK   -> Apply Cyberpunk color scheme
SCHEME 4           -> Same as above (by number)
COLOR 0 255 200 0  -> Set Top Yellow section to custom orange
```

### Panel Mode Commands

| Command | Description | Range |
|---------|-------------|-------|
| `LLMODE <value>` | Set Large Logic Display mode | 0-3 |
| `CBIMODE <value>` | Set CBI Display mode | 0-6 |
| `BARGRAPH <style>` | Set bargraph animation style | SPLIT / CLASSIC |
| `TOPBLOCKS <value>` | Set Top Blocks animation mode | 0-4 |

### Legacy Quick Commands

Single-character shortcuts for rapid changes:

| Command | Function |
|---------|----------|
| `L0` - `L3` | Quick-set Large Logic mode (0-3) |
| `C0` - `C6` | Quick-set CBI mode (0-6) |
| `P0` - `P4` | Quick-change personality (0-4) |
| `S0` - `S7` | Quick-change color scheme (0-7) |

**Examples:**
```
L1    -> Large Logic to Rainbow mode
C6    -> CBI to Matrix Rain mode
P3    -> Set Excited personality
S4    -> Apply Cyberpunk scheme
```

---

## 🎨 Color Schemes

| Index | Name | Description |
|-------|------|-------------|
| 0 | CLASSIC | Film-accurate colors (Yellow, Green, Blue, Red, White) |
| 1 | BLUE | Cool blue tones throughout all panels |
| 2 | PINK | Hot pink and magenta theme (triggers Heart pattern on CBI) |
| 3 | GREEN | Forest green variations |
| 4 | CYBERPUNK | Magenta, Cyan, Purple neon aesthetic |
| 5 | FOREST | Earth tones with orange and dark green |
| 6 | SUNSET | Warm orange, red, and violet gradients |
| 7 | CUSTOM | User-defined colors via the `COLOR` command |

### LED Sections (for COLOR command)

| Section | Index | Default Color (Classic) |
|---------|-------|------------------------|
| Top Yellow Blocks | 0 | Yellow |
| Top Green Blocks | 1 | Green |
| Blue Column | 2 | Blue |
| Bargraph Low | 3 | Green |
| Bargraph Mid | 4 | Yellow |
| Bargraph High | 5 | Red |
| Bottom White | 6 | White |
| Red Large | 7 | Red |
| VU Meter | 8 | (palette-driven) |
| CBI Matrix | 9 | Red |
| CBI Status | 10 | (voltage-driven) |
| Large Logic | 11 | (mode-driven) |

---

## 🧠 Personality Modes

Personalities modify animation speed, CBI timing, hue shift, and organic randomness offset globally.

| Index | Name | Speed | CBI Speed | Hue Shift | Behavior |
|-------|------|-------|-----------|-----------|----------|
| 0 | Normal | 1.0x | 1.0x | 0 | Standard R2-D2 behavior |
| 1 | Happy | 1.5x | 1.2x | +30 | Faster, warmer, rainbow VU meter, smiley on CBI |
| 2 | Grumpy | 0.5x | 0.8x | -30 | Slower, cooler tones |
| 3 | Excited | 2.0x | 1.5x | 0 | Very fast, energetic rainbow Large Logic |
| 4 | Sleepy | 0.3x | 0.5x | -60 | Very slow, dim blue breathing |

**Special Personality Effects:**
- **Happy**: VU Meter shows rainbow animation; CBI periodically displays a smiley face; Large Logic shows juggle pattern
- **Grumpy**: Large Logic pulses dim red
- **Excited**: Large Logic shows fast rainbow sweep
- **Sleepy**: VU Meter pulses slow dim blue; Large Logic breathes very slowly in blue

---

## 🖥️ Panel Modes Reference

### Large Logic Display (Strip E - 43 LEDs)

| Mode | Name | Description |
|------|------|-------------|
| 0 | Breathing | Gentle brightness pulsing in scheme color |
| 1 | Rainbow | Continuous rainbow sweep across all 43 LEDs |
| 2 | Off | All LEDs off |
| 3 | Personality | Animation driven by current personality mode |

### CBI Display (Strip F - 20 matrix + 3 status LEDs)

| Mode | Name | Description |
|------|------|-------------|
| 0 | Organic | Film-style random flickering in scheme color |
| 1 | ESB | Empire Strikes Back authentic 28-frame pattern sequence |
| 2 | Rainbow | Rainbow gradient across matrix |
| 3 | Sparkle | Fading sparkle effect with hue variation |
| 4 | Heart | Static heart pattern in red |
| 5 | Smiley | Static smiley face in yellow |
| 6 | Matrix Rain | Green "digital rain" cascade effect |

### Top Blocks (Strip D - 18 LEDs: 9 Yellow + 9 Green)

| Mode | Name | Description |
|------|------|-------------|
| 0 | Classic | Random paired block activation (film-accurate) |
| 1 | Horizontal | Full row illumination |
| 2 | Paired | Two adjacent rows illuminate together |
| 3 | Chaos | Random individual pixel activation per column |
| 4 | Single Pixel | One random pixel per color section |

### Bargraph (Strip C - 18 LEDs within 24-LED strip)

| Style | Description |
|-------|-------------|
| CLASSIC | Both sides mirror the same random level |
| SPLIT | Left and right sides animate independently |

---

## 🚪 Door Sensor Integration

The controller uses two door sensors to automatically activate or deactivate display panels when body doors open or close.

### Wiring

- **Left Door Sensor** (GPIO10): Controls Strips A, B, C, D (DPL panels)
- **Right Door Sensor** (GPIO11): Controls Strip F (CBI panel)
- Pins configured as `INPUT_PULLUP` - connect switch between pin and GND
- **Active LOW**: Door open = pin reads LOW = panels ON
- **Inactive HIGH**: Door closed = pin reads HIGH = panels OFF (LEDs turn black)

### Behavior

| Left Door | Right Door | Active Panels |
|-----------|------------|---------------|
| Open (LOW) | Open (LOW) | All panels + Large Logic |
| Open (LOW) | Closed (HIGH) | DPL panels + Large Logic |
| Closed (HIGH) | Open (LOW) | CBI + Large Logic |
| Closed (HIGH) | Closed (HIGH) | Large Logic only |

> **Note:** The Large Logic Display (Strip E) is **always active** regardless of door sensor state.

---

## ⚡ Voltage Monitor

The CBI panel includes a 3-LED battery voltage indicator using a resistor voltage divider on GPIO12.

### Hardware Setup

```
Battery (+) ----[ 220k R1 ]----+----[ 33k R2 ]---- GND
                                |
                             GPIO12
```

### Voltage Thresholds

The system auto-configures thresholds based on the `SYSTEM_VOLTAGE` define (default: 24V):

| System | Green (OK) | Yellow (Low) | Red (Critical) |
|--------|-----------|--------------|----------------|
| 24V | >= 23.5V | >= 22.0V | < 22.0V |
| 12V | >= 12.5V | >= 12.0V | < 12.0V |

### Status LEDs (CBI Strip F, LEDs 20-22)

| LED Index | Color | Meaning |
|-----------|-------|---------|
| F[20] | Green | Voltage OK |
| F[21] | Yellow | Voltage low |
| F[22] | Red | Voltage critical |

Enable/disable via the `VOLTAGE ON` / `VOLTAGE OFF` command.

---

## 💾 Profile Management

The controller supports 5 user profiles stored in EEPROM with checksum validation.

### Profile Storage

- **EEPROM Size**: `sizeof(SystemConfig) + 5 * sizeof(UserProfile)` bytes
- **Checksum**: XOR-based checksum with `0xDEAD` magic number
- **Boot Counter**: Tracks total system boots persistently

### Profile Contents

Each profile stores:
- Profile name (16 characters)
- Brightness (1-100)
- DPL animation speed (1-10)
- CBI animation speed (1-10)
- Color scheme (0-7)
- Personality (0-4)
- Large Logic mode (0-3)
- CBI mode (0-6)
- Bargraph style (Split/Classic)
- Top Blocks mode (0-4)
- Sequence mode (ON/OFF)
- Voltage monitor (ON/OFF)
- Custom colors (12 RGB values)

### Default Profile Values

| Parameter | Default |
|-----------|---------|
| Brightness | 25% |
| DPL Speed | 5 |
| CBI Speed | 3 |
| Color Scheme | CLASSIC (0) |
| Personality | NORMAL (0) |
| Large Logic Mode | 0 (Breathing) |
| CBI Mode | 0 (Organic) |
| Bargraph | Classic |
| Top Blocks | Classic (0) |
| Sequence Mode | OFF |
| Voltage Monitor | OFF |

---

## 🚀 Boot Sequence

On power-up, the controller performs the following initialization:

1. Serial port initialization (57600 baud)
2. Voltage monitor threshold configuration (12V or 24V)
3. Watchdog timer initialization (10-second timeout)
4. EEPROM initialization and validation
5. GPIO configuration (door sensors as INPUT_PULLUP)
6. I2C slave initialization (address 0x20)
7. FastLED initialization (6 strips, power limit 5V / 4000mA)
8. System configuration load (boot count increment)
9. Default profile load (if configured)
10. **Startup animation** - Rainbow wave across all 144 LEDs + 3x white flash
11. System ready message with free heap report

---

## 🛠️ Troubleshooting

### Common Issues Quick Reference

| Problem | Quick Fix |
|---------|-----------|
| No serial output | Set baud rate to **57600**, enable "USB CDC On Boot" |
| Only see `ESP-ROM:esp32s3-20210327` | USB CDC On Boot must be "Enabled" in Arduino IDE |
| LEDs not lighting up | Check 5V power supply, verify data pin wiring |
| Door panels always off | Check door sensor wiring (active LOW, connect to GND) |
| Door panels always on | Verify INPUT_PULLUP, ensure switch connects pin to GND |
| Garbled serial output | Verify baud rate is 57600 (not 9600 or 115200) |
| Commands not accepted | Ensure line ending is set to "Newline" (not "No line ending") |
| EEPROM errors | Profile may be corrupted - use `LOAD` with a different profile |
| Watchdog resets | Check for blocking code, ensure animations complete within 10s |
| I2C not responding | Verify address 0x20, check SDA/SCL wiring |
| Voltage monitor incorrect | Verify resistor values (220k / 33k), check ADC pin |
| ANSI garbage in serial | Use a terminal with ANSI color support (PuTTY, Tera Term) |

### Hardware Issues

**LEDs Not Working:**
- Verify 5V power supply is connected and provides sufficient current (4A recommended)
- Check data pin connections match the pin configuration table
- Ensure WS2811/WS2812B strip direction (data flows from DIN to DOUT)
- Test with `BRIGHTNESS 100` to rule out low brightness settings

**Door Sensors Not Working:**
- Sensors use `INPUT_PULLUP` - the switch must connect the pin to GND
- Active LOW: pin reads LOW when door is open (panels ON)
- If panels are always off, the pin may be floating - check wiring
- If panels are always on, the pullup may not be working - check for shorts

**Voltage Monitor Inaccurate:**
- Verify resistor values: R1 = 220k ohm, R2 = 33k ohm
- Check `SYSTEM_VOLTAGE` define matches your battery system (12 or 24)
- ADC reference is 3.3V with 12-bit resolution (0-4095)

### Software Issues

**Serial Communication:**
- Baud rate must be **57600** (or 9600 if modified for MarcDuino)
- Commands must end with newline (`\n`)
- Maximum command length: 63 characters (64-byte buffer minus null terminator)
- Commands timeout after 500ms of inactivity

**Configuration Recovery:**
```
STATUS         <- Check current settings
LOAD 1         <- Try loading a different profile
DEFAULT 0      <- Disable auto-load to use built-in defaults
```

---

## 📊 System Specifications

### Memory & Performance

| Parameter | Value |
|-----------|-------|
| Total LEDs | 144 (across 6 strips) |
| LED Type | WS2811 (GRB color order) |
| Power Limit | 5V / 4000mA (FastLED managed) |
| EEPROM Usage | ~1.5 KB (config + 5 profiles) |
| Watchdog Timeout | 10 seconds |
| Animation Timing | Overflow-safe Timer class |

### Animation Timing (Base Intervals)

| Animation | Base Interval | Affected By |
|-----------|--------------|-------------|
| Top Blocks | 200ms | Speed + Personality |
| Blue Column | 500ms | Speed + Personality |
| Bargraph | 200ms | Speed + Personality |
| Bottom LEDs | 200ms | Speed + Personality |
| Red LEDs | 500ms | Speed + Personality |
| CBI (Organic) | 100ms | CBI Speed + Personality |
| CBI (ESB) | 800ms | CBI Speed + Personality |
| Large Logic | 15ms | (internal) |
| Sequence Cycle | 15 seconds | (fixed) |

---

## 📞 Support

### Getting Help

1. **Check this README** for configuration and troubleshooting
2. **Type `HELP`** in the Serial Monitor for a quick command reference
3. **Type `STATUS`** to verify current settings
4. **Type `INFO`** to check system health (free heap, boot count)

### Diagnostic Commands

```
HELP           <- Full command reference (with ANSI colors)
STATUS         <- All current profile settings
INFO           <- System version, free heap, boot count, LED count
TEST           <- Hardware diagnostic for LEDs and sensors
```

---

## 📜 License & Credits

### Project Credits
- **Software Development**: Printed-Droid.com
- **Hardware Compatibility**: Printed Droid RGB-DPL Board (ESP32-S3 Mini, WS2811/WS2812B)

### Open Source Libraries
- **FastLED** - WS2811/WS2812B LED control with power management
- **ESP32 Arduino Core** - ESP32-S3 platform support

### Disclaimer

**IMPORTANT SAFETY NOTICE**

This project involves electrical components and LED displays. Users are responsible for:

- Proper electrical safety and insulation
- Adequate power supply sizing and protection (5V, 4A minimum)
- Safe assembly and operation
- Correct voltage divider resistor values for battery monitoring
- Testing all functions before final installation

**BUILD AT YOUR OWN RISK.** Ensure proper knowledge of electronics and safety practices. The authors assume no responsibility for damage, injury, or malfunction resulting from use of this design.

---

**May the Force be with your build!**

*RGB-DPL Controller v5.8.1-ESP32 | Printed-Droid.com*
