# RGB Data Port Lights (RGB-DPL) Controller v5.1.1
**Arduino Nano / Pro Mini based RGB LED display controller for R2-D2 Data Port Lights, CBI, and Large Logic Display panels**

## Project Overview

This controller brings your R2-D2's body panels to life with fully addressable RGB LEDs, replacing older MAX7219-based designs with far greater flexibility, color control, and dynamic animations. Built on the Arduino Nano (ATmega328P) platform, it drives six independent WS2811/WS2812B LED strips across all major display panels: DPL (Data Port Lights), CBI (Charge Bay Indicator), and Large Logic Display.

Designed for builders who want film-accurate animations with the power to customize every color, speed, and behavior.

### Key Features

- **Fully Addressable RGB** - 144 total WS2811/WS2812B LEDs across 6 independent strips
- **8 Color Schemes** - Classic, Blue, Pink, Green, Cyberpunk, Forest, Sunset, Custom
- **5 Personality Modes** - Normal, Happy, Grumpy, Excited, Sleepy (affect speed, hue, and behavior)
- **5 User Profiles** - Persistent EEPROM storage with checksummed integrity
- **Door Sensor Integration** - Automatic panel activation/deactivation via left and right door switches
- **Voltage Monitor** - 12V battery monitoring with CBI status LED indicators and low-voltage blinking warning
- **I2C Slave Interface** - Address 0x20 for integration with MarcDuino/BetterDuino systems
- **Serial CLI** - Comprehensive command-line interface at 57600 baud
- **Sequence Mode** - Automatic cycling through animation modes
- **2 TopBlocks Animations** - Random, Classic
- **7 CBI Display Modes** - Organic, ESB, Rainbow, Sparkle, Heart, Smiley, Matrix Rain
- **4 Large Logic Modes** - Breathing, Rainbow, Off, Personality-driven
- **Boot Sequence** - 3x white flash startup animation

> **Note:** CSL (Coin Slot Logics) was removed in v5.1.1 due to ATmega328P SRAM limitations. CSL should run on a separate dedicated controller.

---

## Changelog

### Version 5.1.1 (Arduino Nano - Current)

**Critical SRAM Fix**

- **Removed CSL (Coin Slot Logics)** - The 7th LED strip (leds_g[36] = 108 bytes) caused SRAM overflow, leaving only 63 bytes for the stack. This caused a permanent reset loop with all LEDs stuck on white. CSL should run on a separate dedicated controller.
- **Fixed cmdBuffer stack overflow** - processSerialCommand() allocated 64 bytes on the stack (more than available). Now works directly on the serial buffer.
- **Reduced serial buffer** from 32 to 24 bytes

### Version 5.1

**Stability, Safety & Code Quality**

#### Bug Fixes

**1. Compilation Error Fixed**
- Removed orphaned comment block (missing opening `/*`) that prevented compilation

**2. Header Consolidation**
- Consolidated 4 redundant header blocks (v2.0, v4.0, v5.0, duplicate v5.0) into one clean v5.1 header

#### Improvements

**3. EEPROM Wear Reduction**
- Boot counter now only writes to EEPROM every 10th boot
- Extends EEPROM lifespan significantly (~100k write cycle limit on ATmega328P)

**4. Low Voltage Blinking Warning**
- When battery drops below 11.5V, all 3 CBI status LEDs blink red (300ms interval)
- Provides clear visual alert for critically low battery

**5. matrixRain() Readability**
- Reformatted compressed single-line function into readable multi-line code with comments

**7. Voltage Divider Corrected**
- Fixed resistor values to 100k / 10k to match actual hardware

**8. Memory Optimization**
- Reduced serial buffer from 64 to 32 bytes
- Changed Timer interval from unsigned long to uint16_t (saves 16 bytes SRAM)
- Compressed help menu and error messages
- Simplified startup sequence (3x white flash)
- Final: Flash 98% (348 bytes free), SRAM 96% (63 bytes free)

---

### Previous Versions (Summary)

| Version | Changes |
|---------|---------|
| v5.0 | Complete serial parser rewrite with robust C-style string handling |
| v4.0 | Advanced animation modes (Bargraph, Top Blocks, Smiley), Sequence Mode |
| v3.1 | User profiles, color schemes, and personalities with EEPROM storage |
| v2.0 | Initial RGB version for Arduino Nano, replacing MAX7219 controllers |

---

## Hardware Requirements

### Core Components
- **Arduino Nano** (ATmega328P) or compatible clone
- **WS2811/WS2812B LED strips** - 6 strips, 144 LEDs total
- **5V Power Supply** - Minimum 2A recommended (power limiter set to 2000mA)
- **Voltage divider** (optional) - 100k / 10k resistors for 12V battery monitoring

> **CSL Note:** Coin Slot Logics require a separate dedicated controller due to ATmega328P memory constraints.

### Optional Components
- **Door sensors** - 2x microswitch or magnetic reed switches (normally open)
- **I2C connection** - SDA/SCL to MarcDuino/BetterDuino (address 0x20)

---

## Pin Configuration

### Arduino Nano Pin Assignments

#### LED Data Pins

| Pin | Strip | LED Count | Function |
|-----|-------|-----------|----------|
| D2 | Strip A | 8 LEDs | Bottom white lights (6) + Large red lights (2) |
| D3 | Strip B | 28 LEDs | VU-Meter display |
| D6 | Strip F | 23 LEDs | CBI - Matrix (20) + Status lights (3) |
| D7 | Strip E | 43 LEDs | Large Logic Display Panel |
| D8 | Strip D | 18 LEDs | Top panel - Yellow blocks (9) + Green blocks (9) |
| D9 | Strip C | 24 LEDs | Right panel - Blue column (6) + Bargraph (18) |

#### Sensor & Monitor Pins

| Pin | Function | Notes |
|-----|----------|-------|
| Pin 16 (A2) | Left Door Sensor | INPUT_PULLUP, active LOW (controls Strips A-D) |
| Pin 15 (A1) | Right Door Sensor | INPUT_PULLUP, active LOW (controls Strip F / CBI) |
| A0 | Voltage Monitor (ADC) | Analog input via voltage divider (100k/10k) |

#### I2C Interface

| Pin | Function |
|-----|----------|
| A4 (SDA) | I2C Data |
| A5 (SCL) | I2C Clock |

**I2C Slave Address: `0x20`**

#### Free Pins (Available for Expansion)

| Pin | Status |
|-----|--------|
| D4 | Free |
| D5 | Free |
| D10 | Free |
| D11 | Free |
| D12 | Free |
| D13 | Free |

---

## Installation

### Arduino IDE Setup

1. **Board Configuration:**
   - Board: "Arduino Nano"
   - Processor: "ATmega328P" (or "ATmega328P (Old Bootloader)" for clones)
   - Port: Select the correct COM port

2. **Required Library:**
   Install via Arduino Library Manager:
   - **FastLED** (3.5.0+)

   Built-in libraries (no installation needed):
   - EEPROM
   - Wire (I2C)

### Upload Procedure

1. Connect Arduino Nano via USB
2. Select the correct Board and Port in Arduino IDE
3. Click **Upload**
4. If upload fails, try selecting "ATmega328P (Old Bootloader)" as processor

---

## Communication

### Serial Interface

| Parameter | Value |
|-----------|-------|
| Baud Rate | **57600** |
| Line Ending | Newline (`\n`) |
| Buffer Size | 24 bytes |
| Timeout | 500ms |
| Case Sensitivity | None (commands are case-insensitive) |

> **IMPORTANT:** The current baud rate is **57600**. If you plan to integrate this board with a **MarcDuino** or **BetterDuino** controller (which communicate at 9600 baud), you must change the `SERIAL_BAUD` define in the source code from `57600` to `9600` and re-upload.

### I2C Slave Interface

| Parameter | Value |
|-----------|-------|
| Slave Address | `0x20` |
| Protocol | Standard I2C receive |

The I2C interface allows external controllers (e.g., MarcDuino) to send single-byte commands to the RGB-DPL board. Commands received via I2C are processed in the main loop.

> **Note:** I2C command routing is currently a stub (TODO). The interface is ready for future Marcduino integration.

---

## Serial Command Reference

All commands are sent via the Serial Monitor at 57600 baud. Commands must end with a newline (`\n`). Commands are **case-insensitive**.

### System & Profile Commands

| Command | Description |
|---------|-------------|
| `CONFIG` | Enter configuration mode (Large Logic blinks blue as indicator) |
| `EXIT` | Exit configuration mode, resume normal animation |
| `HELP` | Display the full help menu with all commands |
| `STATUS` | Show current profile settings (brightness, speed, scheme, etc.) |
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

### Panel Mode Commands

| Command | Description | Range |
|---------|-------------|-------|
| `LLMODE <value>` | Set Large Logic Display mode | 0-3 |
| `CBIMODE <value>` | Set CBI Display mode | 0-6 |
| `BARGRAPH <style>` | Set bargraph animation style | SPLIT / CLASSIC |
| `TOPBLOCKS <style>` | Set Top Blocks animation mode | RANDOM / CLASSIC |

---

## Color Schemes

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

## Personality Modes

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

## Panel Modes Reference

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
| RANDOM | Random | Organic random block activation using bit patterns |
| CLASSIC | Classic | Single random pixel per color section |

### Bargraph (Strip C - 18 LEDs within 24-LED strip)

| Style | Description |
|-------|-------------|
| CLASSIC | Both sides mirror the same random level |
| SPLIT | Left and right sides animate independently |

---

## Door Sensor Integration

The controller uses two door sensors to automatically activate or deactivate display panels when body doors open or close.

### Wiring

- **Left Door Sensor** (Pin 16 / A2): Controls Strips A, B, C, D (DPL panels)
- **Right Door Sensor** (Pin 15 / A1): Controls Strip F (CBI panel)
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

## Voltage Monitor

The CBI panel includes a 3-LED battery voltage indicator using a resistor voltage divider on pin A0.

### Hardware Setup

```
Battery (+) ----[ 100k R1 ]----+----[ 10k R2 ]---- GND
                                |
                               A0
```

### Voltage Thresholds (12V System)

| LED | Threshold | Color | Meaning |
|-----|-----------|-------|---------|
| F[20] | >= 12.5V | Green | Voltage OK |
| F[21] | >= 12.0V | Yellow | Voltage low |
| F[22] | Always on | Red | Battery connected |

### Low Voltage Warning (v5.1)

When battery voltage drops **below 11.5V**, all 3 status LEDs (F[20], F[21], F[22]) **blink red** at a 300ms interval, providing a clear visual warning for critically low battery.

Enable/disable via the `VOLTAGE ON` / `VOLTAGE OFF` command.

---

## Profile Management

The controller supports 5 user profiles stored in EEPROM with checksum validation.

### Profile Storage

- **EEPROM Usage**: ~330 bytes of 1024 available (config + 5 profiles)
- **Checksum**: XOR-based checksum with `0xDEAD` magic number
- **Boot Counter**: Tracks total system boots (written every 10th boot to reduce EEPROM wear)

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
- Top Blocks mode (Random/Classic)
- Sequence mode (ON/OFF)
- Voltage monitor (ON/OFF)
- Custom colors (12 RGB values)

### Default Profile Values

| Parameter | Default |
|-----------|---------|
| Brightness | 25% |
| DPL Speed | 5 |
| CBI Speed | 5 |
| Color Scheme | CLASSIC (0) |
| Personality | NORMAL (0) |
| Large Logic Mode | 0 (Breathing) |
| CBI Mode | 0 (Organic) |
| Bargraph | Classic |
| Top Blocks | Random |
| Sequence Mode | OFF |
| Voltage Monitor | OFF |

---

## Boot Sequence

On power-up, the controller performs the following initialization:

1. Serial port initialization (57600 baud)
2. GPIO configuration (door sensors as INPUT_PULLUP)
3. I2C slave initialization (address 0x20)
4. FastLED initialization (6 strips, power limit 5V / 2000mA)
5. EEPROM initialization and validation
6. System configuration load (boot count increment, EEPROM write every 10th boot)
7. Default profile load (if configured)
8. **Startup animation** - 3x white flash across all 144 LEDs
9. System ready message

---

## Troubleshooting

### Common Issues Quick Reference

| Problem | Quick Fix |
|---------|-----------|
| No serial output | Set baud rate to **57600** in Serial Monitor |
| LEDs not lighting up | Check 5V power supply, verify data pin wiring |
| Door panels always off | Check door sensor wiring (active LOW, connect to GND) |
| Door panels always on | Verify INPUT_PULLUP, ensure switch connects pin to GND |
| Garbled serial output | Verify baud rate is 57600 (not 9600 or 115200) |
| Commands not accepted | Ensure line ending is set to "Newline" (not "No line ending") |
| EEPROM errors | Profile may be corrupted - use `LOAD` with a different profile |
| I2C not responding | Verify address 0x20, check SDA (A4) / SCL (A5) wiring |
| Voltage monitor incorrect | Verify resistor values (100k / 10k), check A0 pin |
| Upload fails | Try "ATmega328P (Old Bootloader)" as processor |

### Hardware Issues

**LEDs Not Working:**
- Verify 5V power supply is connected and provides sufficient current (2A minimum)
- Check data pin connections match the pin configuration table
- Ensure WS2811/WS2812B strip direction (data flows from DIN to DOUT)
- Test with `BRIGHTNESS 100` to rule out low brightness settings

**Door Sensors Not Working:**
- Sensors use `INPUT_PULLUP` - the switch must connect the pin to GND
- Active LOW: pin reads LOW when door is open (panels ON)
- If panels are always off, the pin may be floating - check wiring
- If panels are always on, the pullup may not be working - check for shorts

**Voltage Monitor Inaccurate:**
- Verify resistor values: R1 = 100k ohm, R2 = 10k ohm
- ADC reference is 5.0V with 10-bit resolution (0-1023)

### Software Issues

**Serial Communication:**
- Baud rate must be **57600** (or 9600 if modified for MarcDuino)
- Commands must end with newline (`\n`)
- Maximum command length: 23 characters (24-byte buffer minus null terminator)
- Commands timeout after 500ms of inactivity

**Configuration Recovery:**
```
STATUS         <- Check current settings
LOAD 1         <- Try loading a different profile
DEFAULT 0      <- Disable auto-load to use built-in defaults
```

---

## System Specifications

### Memory & Performance

| Parameter | Value |
|-----------|-------|
| MCU | ATmega328P (Arduino Nano) |
| Flash | 30720 bytes available (~92% used) |
| SRAM | 2048 bytes (~85% used, ~300 bytes free for stack) |
| EEPROM | 1 KB (~340 bytes used) |
| Total LEDs | 144 (across 6 strips) |
| LED Types | WS2811 (GRB, Strips A-F) |
| Power Limit | 5V / 2000mA (FastLED managed) |

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

## Support

### Getting Help

1. **Check this README** for configuration and troubleshooting
2. **Type `HELP`** in the Serial Monitor for a quick command reference
3. **Type `STATUS`** to verify current settings

### Diagnostic Commands

```
HELP           <- Full command reference
STATUS         <- All current profile settings
```

---

## License & Credits

### Project Credits
- **Software Development**: Printed-Droid.com
- **Hardware Compatibility**: Printed Droid RGB-DPL Board (Arduino Nano, WS2811/WS2812B)

### Open Source Libraries
- **FastLED** - WS2811/WS2812B LED control with power management

### Disclaimer

**IMPORTANT SAFETY NOTICE**

This project involves electrical components and LED displays. Users are responsible for:

- Proper electrical safety and insulation
- Adequate power supply sizing and protection (5V, 2-3A recommended)
- Safe assembly and operation
- Correct voltage divider resistor values for battery monitoring (100k / 10k)
- Testing all functions before final installation

**BUILD AT YOUR OWN RISK.** Ensure proper knowledge of electronics and safety practices. The authors assume no responsibility for damage, injury, or malfunction resulting from use of this design.

---

**May the Force be with your build!**

*RGB-DPL Controller v5.1.1 | Arduino Nano / Pro Mini | Printed-Droid.com*
