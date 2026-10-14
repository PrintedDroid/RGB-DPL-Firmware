# RGB-DPL Firmware

<!-- webflash-notice -->
> **Notice:** New firmware for the **ESP32-S3 and ESP32-WROOM** variants (**v6.1** — including the local **Web UI / Access Point** and the extended **LDPL effects**: `LDPLCOLOR`, 16 modes) is released **only** as a ready-to-flash web installer in the [Printed Droid Firmware Flasher](https://www.printed-droid.com/firmware-flasher/). The **Nano / Pro Mini** firmware is **final at v5.2** and is available only here in this repository — there are no new Nano builds and it is not on the web flasher. The documentation (commands, usage, settings) continues to be updated here and in the wiki; the source code in this repository (ESP32-S3 v6.0, Nano v5.2) is the last openly published snapshot and will no longer be updated.
>
> **Why:** Printed Droid has shared its source code openly for years. In the meantime, more and more projects in the community build on openly shared work but release their own results as closed source only. Open source works in both directions – this one-way street is not something we will keep supplying.

**Fully addressable RGB LED controller firmware for the Printed-Droid RGB-DPL board.**
Drives DPL, CBI, LDPL and CSL panels for R2-D2 and other Astromech droids using WS2811 / WS2812 LEDs.

---

## Quick start

1. Pick the firmware variant matching your microcontroller (see folder structure below)
2. Open the `.ino` sketch in the Arduino IDE
3. Install the **FastLED** library (3.5.0 or newer)
4. Configure the board:
   - **Pro Mini:** Board = `Arduino Nano`, Processor = `ATmega328P`, baud = `57600`
   - **ESP32-S3:** Board = `LOLIN S3 Mini` (or `ESP32S3 Dev Module`), `USB CDC On Boot` = **Enabled**, baud = `9600`
5. Click **Upload**
6. Bridge the `Left Door` and `Right Door` headers (or attach door switches) — otherwise some LED sections stay dark
7. Open a serial terminal at the right baud rate, line ending **Newline**, and send `BRIGHTNESS 50` → LEDs respond

For full setup, hardware wiring, all commands, animations and troubleshooting, see the user manual:

| Manual | Language |
|--------|----------|
| `RGB-DPL_Manual_EN.pdf` | English |
| `RGB-DPL_Handbuch_DE.pdf` | German |

---

## Which sketch to use

| | Pro Mini variant | ESP32-S3 variant |
|---|---|---|
| **Folder** | [`RGB_DPL-Nano_ProMini_v5.2/`](RGB_DPL-Nano_ProMini_v5.2/) | [`RGB_DPL-ESP32_S3_Mini_v6.0/`](RGB_DPL-ESP32_S3_Mini_v6.0/) |
| **Version** | v5.2 | v6.0 |
| **MCU** | ATmega328P | ESP32-S3 |
| **LED outputs** | 7 strips, 180 LEDs | 7 strips, 180 LEDs |
| **Baud rate** | 57600 | 9600 |
| **LDPL** | yes | yes |
| **CSL** | yes | yes |
| **VCC commands** (`VM`, `VL`, `RR`) | yes | yes |
| **I2C slave** | no, removed to fit on Nano | yes (address 0x20) |
| **Watchdog** | no | yes (10 s) |
| **TopBlocks modes** | 2 (RANDOM/CLASSIC) | 5 (0-4) |
| **Voltage divider default** | 100k / 10k | 100k / 10k |
| **Headroom** | very tight (98% flash, 89% RAM) | plenty |
| **Recommended status** | existing Pro Mini hardware | mainline / recommended |

The two sketches are not interchangeable. Use the one that matches your controller.

### Short recommendation

- **Best overall / recommended:** `RGB_DPL-ESP32_S3_Mini_v6.0`
- **Only choose Pro Mini:** if your hardware is already built around Arduino Nano / Pro Mini

## Looking for the classic variant?

Printed-Droid also offers the **DPL Classic** and **DPL-VU** — the classic boards with a mix of MAX7219 (for DPL/CBI) and WS2812B (for DCBI/LDPL/UAL). They run the AstroCAN BodyLights firmware (by RealNobser) with slash-based commands (`/DP/BR/15`). Simpler build, fixed-color analog LEDs.

Repository: https://github.com/PrintedDroid/DPL-VU_and_DPL-Classic

---

## Hardware

### LED strips

The same 7 strips on both variants, only the pin assignment differs.

| Strip | LEDs | Pin (Pro Mini / ESP32-S3) | Function | On board / external |
|:-----:|:----:|---------------------------|----------|---------------------|
| A | 8 | D2 / IO4 | Bottom White (6) + Red Large (2) | DPL board |
| B | 28 | D3 / IO5 | VU meter | DPL board |
| C | 24 | D9 / IO6 | Blue column (6) + bargraph (18) | DPL board |
| D | 18 | D8 / IO7 | TopBlocks — yellow (9) + green (9) | DPL board |
| E | 43 | D7 / IO8 | LDPL (Large DataPanel Logics) | external |
| F | 23 | D6 / IO9 | D-CBI matrix (20) + status (3) | external |
| G | 36 | D5 / IO13 | CSL (Coin Slot Logics) | external |

### Inputs

| Function | Pin (Pro Mini / ESP32-S3) |
|----------|----------------------------|
| Left Door switch | A2 (16) / IO10 |
| Right Door switch | A1 (15) / IO11 |
| Voltage monitor (ADC) | A0 / IO12 |

### Voltage monitor

Default voltage divider on the board: **R1 = 100 kΩ**, R2 = 10 kΩ (suitable for 0–~30 V battery measurement).

Default thresholds: red `11.5 V`, yellow `12.0 V`, green `12.5 V`, charge `13.0 V`.

Configurable at runtime:
- `VM <0|1>` — enable/disable monitor
- `VL <RED YELLOW GREEN CHARGE>` — set thresholds in volts
- `RR <R1 R2>` — match software scaling to swapped resistors

---

## Serial commands (overview)

The full list and per-command details are in the manual. Quick reference:

```text
HELP / STATUS / INFO / CONFIG / EXIT
SAVE <1-5>     LOAD <1-5>     DEFAULT <0-5>

BRIGHTNESS <1-100>     SPEED <1-10>     CBISPEED <1-10>
SCHEME <name|0-7>      PERSONALITY <0-4>
SEQUENCE <ON|OFF>      VOLTAGE <ON|OFF>
COLOR <section> <R> <G> <B>

LDPLMODE <0-3>     LLMODE <0-3>     (alias)
CBIMODE <0-6>
CSLMODE <0-7>      CSLSPEED <1-10>     CSLBRIGHTNESS <1-100>
BARGRAPH <SPLIT|CLASSIC>
TOPBLOCKS <RANDOM|CLASSIC>     (Pro Mini)
TOPBLOCKS <0-4>                (ESP32-S3)

VM <0|1>     VL <RED YELLOW GREEN CHARGE>     RR <R1 R2>

P0-P4 / S0-S7    Quick commands for personality / scheme
```

---

## Color schemes & personalities

**Color schemes** (`SCHEME <name|0-7>`): CLASSIC, BLUE, PINK, GREEN, CYBERPUNK, FOREST, SUNSET, CUSTOM.

**Personality modes** (`PERSONALITY 0-4`): NORMAL, HAPPY, GRUMPY, EXCITED, SLEEPY — affect speed, hue and behavior globally.

**Profiles:** 5 EEPROM slots (`SAVE 1-5`, `LOAD 1-5`) with checksum validation. Set the boot profile with `DEFAULT <0-5>` (0 disables auto-load).

---

## Build status

Both sketches compile cleanly:

- `RGB_DPL-ESP32_S3_Mini_v6.0` for `esp32:esp32:lolin_s3_mini` — flash `397k / 1310k`, RAM `25k / 327k` (plenty of headroom)
- `RGB_DPL-Nano_ProMini_v5.2` for `arduino:avr:nano` — flash `30676 / 30720`, RAM `1825 / 2048` (very close to ATmega328P limits, treat as careful test build)

---

## Older firmware versions

Earlier firmware revisions (Nano v5.1, ESP32-S3 v5.8) are no longer kept in this repository. Migration notes between firmware versions are in the appendix of the user manual.

## Project structure

```text
RGB-DPL-Firmware/
  README.md                            (this file)
  RGB-DPL_Manual_EN.pdf                (full user manual, English)
  RGB-DPL_Handbuch_DE.pdf              (full user manual, German)

  RGB_DPL-ESP32_S3_Mini_v6.0/          current ESP32-S3 build (recommended)
    RGB_DPL-ESP32_S3_Mini_v6.0.ino
    README.md

  RGB_DPL-Nano_ProMini_v5.2/           current Pro Mini build
    RGB_DPL-Nano_ProMini_v5.2.ino
    README.md
```

Each sketch folder has its own `README.md` with full version-specific details (pin map, command reference, build sizes, Arduino IDE setup).

---

## Acknowledgements

- **Hardware & firmware:** Printed-Droid
- **LED control library:** [FastLED](https://github.com/FastLED/FastLED)

Build and wiring are at your own risk.

---

## Support

- **Product pages:** https://www.printed-droid.com
- **Facebook group:** https://www.facebook.com/groups/printeddroid/
- **Issues / documentation feedback:** open an issue in this repository

---

## License

- **Firmware:** © Printed-Droid — all rights reserved
- **Manuals & this documentation:** © Printed-Droid — all rights reserved. No commercial reuse, duplication or modification without explicit consent.
