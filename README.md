# RGB-DPL - R2-D2 Data Port Lights Controller

**Fully addressable RGB LED display controller for Printed-Droid R2-D2 replicas**

Controls all body display panels -- Data Port Lights (DPL), Charge Bay Indicator (CBI), Large Logic Display, and optionally Coin Slot Logics (CSL) -- using WS2811/WS2812B addressable RGB LEDs. Replaces older MAX7219-based designs with full color control, dynamic animations, and extensive configurability.

---

## Board Variants

This project supports two hardware platforms. Choose the version that matches your build:

| | Arduino Nano / Pro Mini | ESP32-S3 Mini |
|---|---|---|
| **Folder** | [`RGB_DPL-Nano_ProMini_v5.1/`](RGB_DPL-Nano_ProMini_v5.1/) | [`RGB_DPL-ESP32_S3_Mini_v5.8/`](RGB_DPL-ESP32_S3_Mini_v5.8/) |
| **Version** | v5.1 | v5.8 |
| **MCU** | ATmega328P | ESP32-S3 |
| **LED Strips** | 7 strips, 180 LEDs | 6 strips, 144 LEDs |
| **CSL (Coin Slots)** | Integrated (Strip G, 36 LEDs) | Not included (separate controller) |
| **Baud Rate** | 57600 | 9600 (MarcDuino-ready) |
| **Power Limit** | 5V / 2A | 5V / 4A |
| **Voltage Monitor** | 12V only (100k/10k divider) | 12V / 24V auto-config (220k/33k divider) |
| **Watchdog Timer** | No | Yes (10s, auto-recovery) |
| **I2C Thread Safety** | Basic | Critical section protected (ISR-safe) |
| **Timer Overflow Safe** | No (49-day rollover risk) | Yes |
| **Serial Buffer** | 32 bytes | 64 bytes with overflow protection |
| **EEPROM Wear Protection** | Yes (write every 10th boot) | Yes |
| **PROGMEM Handling** | Standard AVR | Corrected for ESP32 (`pgm_read_ptr`) |
| **ANSI Color Help Menu** | No | Yes |
| **System INFO Command** | No | Yes (free heap, boot count, version) |
| **TopBlocks Animations** | 2 modes (Random, Classic) | 5 modes (Classic, Horizontal, Paired, Chaos, Single) |
| **Flash Usage** | 98% (348 bytes free) | Plenty of headroom |
| **SRAM Usage** | 96% (63 bytes free) | Plenty of headroom |

---

## Shared Features (Both Versions)

- **8 Color Schemes** -- Classic, Blue, Pink, Green, Cyberpunk, Forest, Sunset, Custom
- **5 Personality Modes** -- Normal, Happy, Grumpy, Excited, Sleepy (affect speed, hue, and animation behavior)
- **5 User Profiles** -- Persistent EEPROM storage with checksummed integrity
- **7 CBI Display Modes** -- Organic, ESB (28-frame authentic pattern), Rainbow, Sparkle, Heart, Smiley, Matrix Rain
- **4 Large Logic Modes** -- Breathing, Rainbow, Off, Personality-driven
- **Door Sensor Integration** -- Automatic panel activation/deactivation via left and right door switches
- **CBI Voltage Monitor** -- Battery status LEDs (Green/Yellow/Red)
- **I2C Slave Interface** -- Address `0x20` for MarcDuino/BetterDuino integration
- **Serial CLI** -- Case-insensitive command parser with full configuration control
- **Sequence Mode** -- Automatic animation cycling
- **Bargraph Styles** -- Classic (mirrored) and Split (independent sides)
- **Custom Colors** -- Per-section RGB color override via `COLOR` command

---

## Which Version Do I Need?

The firmware version is determined by your RGB-DPL board hardware:

- **RGB-DPL Board with Arduino Nano / Pro Mini** --> use **v5.1** from [`RGB_DPL-Nano_ProMini_v5.1/`](RGB_DPL-Nano_ProMini_v5.1/)
- **RGB-DPL Board with ESP32-S3 Mini** --> use **v5.8** from [`RGB_DPL-ESP32_S3_Mini_v5.8/`](RGB_DPL-ESP32_S3_Mini_v5.8/)

Check which microcontroller is on your board and use the matching sketch. The two versions are not interchangeable.

---

## Quick Start

### 1. Install Dependencies

- [Arduino IDE](https://www.arduino.cc/en/software) 1.8+ or 2.x
- **FastLED** library (3.5.0+) -- install via Library Manager
- For ESP32-S3: install ESP32 board support via Board Manager

### 2. Choose Your Board

Navigate to the folder matching your hardware and open the `.ino` file:

```
RGB_DPL-Nano_ProMini_v5.1/RGB_DPL-Nano_ProMini_v5.1.ino    (Arduino Nano)
RGB_DPL-ESP32_S3_Mini_v5.8/RGB_DPL_S3_v5.8.ino              (ESP32-S3 Mini)
```

### 3. Configure and Upload

Each folder contains its own **README.md** with detailed board configuration, pin assignments, wiring diagrams, serial command reference, and troubleshooting.

### 4. Connect and Control

Open Serial Monitor at the correct baud rate (57600 for Nano, 9600 for ESP32-S3) and type `HELP` to see all available commands.

---

## Serial Command Overview

Both versions share the same core command set:

```
HELP                    Full command reference
STATUS                  Show current settings
CONFIG / EXIT           Enter/exit configuration mode
SAVE <1-5>              Save to profile slot
LOAD <1-5>              Load from profile slot
DEFAULT <0-5>           Set startup profile (0 = none)
BRIGHTNESS <1-100>      Master brightness (%)
SPEED <1-10>            DPL animation speed
CBISPEED <1-10>         CBI animation speed
SCHEME <name|0-7>       Color scheme
PERSONALITY <0-4>       Personality mode
SEQUENCE <ON|OFF>       Auto-cycle animations
VOLTAGE <ON|OFF>        Battery monitor display
LLMODE <0-3>            Large Logic mode
CBIMODE <0-6>           CBI display mode
BARGRAPH <SPLIT|CLASSIC>
TOPBLOCKS <mode>        (Nano: RANDOM|CLASSIC, ESP32: 0-4)
COLOR <sec> <R> <G> <B> Custom color per section
```

**Nano-only CSL commands:** `CSLMODE <0-3>`, `CSLSPEED <10-200>`, `CSLAUTO <ON|OFF>`

**ESP32-only commands:** `INFO`, `TEST`

---

## LED Strip Layout

### Shared Strips (A-F)

| Strip | LEDs | Function |
|-------|------|----------|
| A | 8 | Bottom white lights (6) + Large red lights (2) |
| B | 28 | VU-Meter display |
| C | 24 | Right panel -- Blue column (6) + Bargraph (18) |
| D | 18 | Top panel -- Yellow blocks (9) + Green blocks (9) |
| E | 43 | Large Logic Display Panel |
| F | 23 | CBI -- Matrix (20) + Status lights (3) |

### Nano Only

| Strip | LEDs | Function |
|-------|------|----------|
| G | 36 | CSL -- Coin Slot Logics (6x6 grid) |

Pin assignments differ between boards -- see each version's README for details.

---

## Hardware Requirements

### Common Components
- WS2811/WS2812B LED strips (see LED counts above)
- 5V power supply (2A minimum for Nano, 4A for ESP32-S3)
- Door sensors (optional) -- 2x microswitch or magnetic reed switches
- Voltage divider resistors (optional) -- for battery monitoring
- I2C connection (optional) -- for MarcDuino/BetterDuino integration

### Board-Specific
- **Nano**: Arduino Nano (ATmega328P), voltage divider 100k/10k
- **ESP32-S3**: Lolin S3 Mini or equivalent, voltage divider 220k/33k

---

## Project Structure

```
RGB-DPL/
  README.md                              <- This file
  RGB_DPL-Nano_ProMini_v5.1/
    RGB_DPL-Nano_ProMini_v5.1.ino        <- Arduino Nano sketch
    README.md                            <- Nano-specific documentation
  RGB_DPL-ESP32_S3_Mini_v5.8/
    RGB_DPL_S3_v5.8.ino                  <- ESP32-S3 sketch
    README.md                            <- ESP32-specific documentation
```

---

## License & Credits

- **Software Development**: Printed-Droid.com
- **Hardware Compatibility**: Printed Droid RGB-DPL Board
- **Library**: [FastLED](https://github.com/FastLED/FastLED) -- WS2811/WS2812B LED control

**BUILD AT YOUR OWN RISK.** This project involves electrical components and LED displays. Ensure proper knowledge of electronics and safety practices. The authors assume no responsibility for damage, injury, or malfunction resulting from use of this design.

---

**May the Force be with your build!**

*RGB-DPL Controller | Printed-Droid.com*
