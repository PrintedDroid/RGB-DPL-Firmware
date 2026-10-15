# RGB-DPL Firmware

<!-- webflash-notice -->
> **Notice:** This repository holds the open standalone firmware sources: **ESP32-WROOM** (`RGB-DPL32`, **v6.1.1**) and **Nano / Pro Mini** (**v5.2.1**), both maintained here, plus the older **ESP32-S3 Mini** snapshot (**v6.0**). The current **ESP32-S3** firmware (v6.1.1) and ready-to-flash installers for the ESP variants are in the [Printed Droid Firmware Flasher](https://www.printed-droid.com/firmware-flasher/). The v6.1.1 ESP firmware adds a local **Web UI / Access Point**, extended **LDPL effects** (`LDPLCOLOR` + 16 modes), a selectable **startup style** (`STARTUP 0|1`), and per-panel **serial on/off** (`ON`/`OFF <panel>`, `ALLON`/`ALLOFF`). Documentation (commands, usage, settings) is updated here and in the wiki.
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
   - **ESP32-WROOM (RGB-DPL32):** Board = `ESP32 Dev Module`, Partition = Default, baud = `9600`
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

| | Pro Mini | ESP32-S3 Mini | ESP32-WROOM (RGB-DPL32) |
|---|---|---|---|
| **Folder** | [`RGB_DPL-Nano_ProMini_v5.2/`](RGB_DPL-Nano_ProMini_v5.2/) | [`RGB_DPL-ESP32_S3_Mini_v6.0/`](RGB_DPL-ESP32_S3_Mini_v6.0/) | [`RGB-DPL32/`](RGB-DPL32/) |
| **Version** | v5.2.1 | v6.0 (older snapshot) | v6.1.1 |
| **MCU** | ATmega328P | ESP32-S3 | ESP32-WROOM |
| **LED outputs** | 7 strips, 180 LEDs | 7 strips, 180 LEDs | 6 strips, 144 LEDs (no CSL) |
| **Baud rate** | 57600 | 9600 | 9600 |
| **LDPL** | yes | yes | yes |
| **CSL** | yes | yes | no (not wired) |
| **Local Web UI / AP** | no | v6.1.1 only (not in this v6.0 folder) | yes |
| **VCC commands** (`VM`, `VL`, `RR`) | yes | yes | yes |
| **Watchdog** | no | yes | yes |
| **TopBlocks modes** | 2 (RANDOM/CLASSIC) | 5 (0-4) | 5 (0-4) |
| **Voltage divider default** | 100k / 10k | 100k / 10k | 100k / 10k |
| **Headroom** | very tight (98% flash) | plenty | plenty |

The sketches are not interchangeable. Use the one that matches your controller.

> **Note on the ESP32-S3 Mini folder:** it is the older **v6.0** snapshot (no Web UI). The current S3
> firmware (**v6.1.1**, with Web UI and the new `STARTUP` / panel on/off commands) is on the web flasher;
> the S3 source can be refreshed here on request.

### Short recommendation

- **ESP32-WROOM (RGB-DPL32):** `RGB-DPL32/` — current, with Web UI and the new commands.
- **ESP32-S3 Mini:** this repo has the v6.0 snapshot; for the current build use the web flasher.
- **Only choose Pro Mini:** if your hardware is already built around Arduino Nano / Pro Mini.

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

ALLON / ALLOFF   All panels on/off (Nano v5.2.1 and current ESP firmware)

P0-P4 / S0-S7    Quick commands for personality / scheme
```

> The current web-flasher ESP firmware (v6.1.1) additionally has `STARTUP 0|1` (boot style) and
> per-panel `ON <panel>` / `OFF <panel>`; the Nano v5.2.1 in this repo has `ALLON` / `ALLOFF`. See the
> manual for the full, per-variant command reference.

---

## Color schemes & personalities

**Color schemes** (`SCHEME <name|0-7>`): CLASSIC, BLUE, PINK, GREEN, CYBERPUNK, FOREST, SUNSET, CUSTOM.

**Personality modes** (`PERSONALITY 0-4`): NORMAL, HAPPY, GRUMPY, EXCITED, SLEEPY — affect speed, hue and behavior globally.

**Profiles:** 5 EEPROM slots (`SAVE 1-5`, `LOAD 1-5`) with checksum validation. Set the boot profile with `DEFAULT <0-5>` (0 disables auto-load).

---

## Build status

Both sketches compile cleanly:

- `RGB-DPL32` (v6.1.1) for `esp32:esp32:esp32` (ESP32 Dev Module) — flash `1125084 / 1310720`, globals `51444 / 327680` (plenty of headroom)
- `RGB_DPL-ESP32_S3_Mini_v6.0` for `esp32:esp32:lolin_s3_mini` — flash `397k / 1310k`, RAM `25k / 327k` (plenty of headroom)
- `RGB_DPL-Nano_ProMini_v5.2` (v5.2.1) for `arduino:avr:nano` — flash `30296 / 30720`, RAM `1822 / 2048` (very close to ATmega328P limits, treat as careful test build)

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

  RGB_DPL-Nano_ProMini_v5.2/           current Pro Mini build (v5.2.1)
    RGB_DPL-Nano_ProMini_v5.2.ino
    README.md

  RGB-DPL32/                           current ESP32-WROOM build (v6.1.1), self-contained
    RGB-DPL32.ino
    rgbdpl_power.h
    rgbdpl_standalone_web.h
    rgbdpl_ldpl_patterns.h
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
