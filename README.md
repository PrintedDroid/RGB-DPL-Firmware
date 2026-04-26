# RGB-DPL - R2-D2 Data Port Lights Controller

Fully addressable RGB LED controller for Printed-Droid R2-D2 body displays.

This project drives the main body panels with WS2811/WS2812 LEDs:
- DPL
- CBI
- LDPL
- optional CSL

The current maintained sketches are:
- [`RGB_DPL-ESP32_S3_Mini_v6.0/`](RGB_DPL-ESP32_S3_Mini_v6.0/)
- [`RGB_DPL-Nano_ProMini_v5.2/`](RGB_DPL-Nano_ProMini_v5.2/)

---

## Which Sketch Should I Use?

### Use `RGB_DPL-ESP32_S3_Mini_v6.0` if:
- your board uses an ESP32-S3 Mini
- you want the best current version
- you need `CSL` on its own output
- you want the newer VCC commands `VM`, `VL`, `RR`
- you want the most headroom, robustness, and easiest future expansion

### Use `RGB_DPL-Nano_ProMini_v5.2` if:
- your board uses an Arduino Nano or Pro Mini
- you need a Nano-based build and accept tighter limits
- you want `LDPL` and `CSL` on the Nano as a best-effort build

### Short recommendation
- **Best overall / recommended:** `RGB_DPL-ESP32_S3_Mini_v6.0`
- **Only choose Nano:** if your hardware is already built around Nano / Pro Mini

The two sketches are not interchangeable. Use the one that matches your controller board.

---

## Board Overview

| | Nano / Pro Mini | ESP32-S3 Mini |
|---|---|---|
| **Folder** | [`RGB_DPL-Nano_ProMini_v5.2/`](RGB_DPL-Nano_ProMini_v5.2/) | [`RGB_DPL-ESP32_S3_Mini_v6.0/`](RGB_DPL-ESP32_S3_Mini_v6.0/) |
| **Version** | v5.2 | v6.0 |
| **MCU** | ATmega328P | ESP32-S3 |
| **LED Outputs** | 7 strips | 7 strips |
| **Total LEDs** | 180 | 180 |
| **Baud Rate** | 57600 | 9600 |
| **LDPL** | Yes | Yes |
| **CSL** | Yes | Yes |
| **Voltage Divider Default** | 100k / 10k | 100k / 10k |
| **VCC Commands** | Yes (`VM`, `VL`, `RR`) | Yes (`VM`, `VL`, `RR`) |
| **I2C** | No, removed to fit on Nano | Yes |
| **Headroom** | Very tight | Plenty |
| **Recommended Status** | Experimental / constrained | Mainline / recommended |

---

## Important Difference Between The Two

The ESP32-S3 build is the full-featured target.

The Nano build was trimmed so it still compiles on an ATmega328P with `LDPL` and `CSL` added. That means:
- no active I2C support in `v5.2`
- reduced CBI feature depth compared with the richer versions
- very little flash/RAM reserve left

If you are starting fresh, use the ESP32-S3 version.

---

## Shared Core Features

- profiles stored in EEPROM
- color schemes
- personality modes
- DPL / VU / CBI / LDPL control
- serial CLI
- sequence mode
- battery monitor
- `LDPL` naming in the newer command set
- `CSL` support in the current maintained sketches

---

## Current Pin Intent

### Nano / Pro Mini v5.2
- `D2` Strip A
- `D3` Strip B
- `D9` Strip C
- `D8` Strip D
- `D7` LDPL
- `D6` CBI
- `D5` CSL

### ESP32-S3 Mini v6.0
- `IO4` Strip A
- `IO5` Strip B
- `IO6` Strip C
- `IO7` Strip D
- `IO8` LDPL
- `IO9` CBI
- `IO13` CSL

---

## Command Notes

Both current sketches support the main RGB-DPL command style, including:
- `HELP`
- `STATUS`
- `SAVE` / `LOAD` / `DEFAULT`
- `BRIGHTNESS`
- `SPEED`
- `CBISPEED`
- `SCHEME`
- `PERSONALITY`
- `SEQUENCE`
- `VOLTAGE`
- `COLOR`
- `LDPLMODE`
- `CBIMODE`
- `CSLMODE`
- `CSLSPEED`
- `CSLBRIGHTNESS`
- `VM`
- `VL`
- `RR`

Compatibility note:
- `LLMODE` remains as an alias for `LDPLMODE`

---

## Folder Guide

### [`RGB_DPL-ESP32_S3_Mini_v6.0/`](RGB_DPL-ESP32_S3_Mini_v6.0/)
Use this for the current ESP32-S3 board.

Best choice when:
- you want the fullest feature set
- you need the cleanest path forward
- you want CSL without Nano memory compromises

### [`RGB_DPL-Nano_ProMini_v5.2/`](RGB_DPL-Nano_ProMini_v5.2/)
Use this only for Nano / Pro Mini hardware.

Best choice when:
- your existing board is already Nano-based
- you need a working Nano sketch with `LDPL` and `CSL`
- you accept that it is a tighter, reduced build

---

## Build Status

Both current sketches were compiled successfully in Arduino CLI:
- `RGB_DPL-ESP32_S3_Mini_v6.0` for `esp32:esp32:lolin_s3_mini`
- `RGB_DPL-Nano_ProMini_v5.2` for `arduino:avr:nano`

Nano note:
- the Nano build is very close to the ATmega328P limits and should be treated as a careful test build

---

## Project Structure

```text
RGB-DPL/
  README.md
  RGB_DPL-ESP32_S3_Mini_v6.0/
    RGB_DPL-ESP32_S3_Mini_v6.0.ino
    README.md
  RGB_DPL-Nano_ProMini_v5.2/
    RGB_DPL-Nano_ProMini_v5.2.ino
    README.md
```

---

## Credits

- Printed-Droid hardware ecosystem
- FastLED library

Build and wiring are at your own risk.
