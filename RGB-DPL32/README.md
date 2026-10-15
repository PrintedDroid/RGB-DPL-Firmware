# RGB-DPL32 — ESP32-WROOM variant (firmware v6.1.1)

Standalone RGB-DPL firmware for the **ESP32-WROOM** board (RGB-DPL32), hardware revisions up to 1.3.
Self-contained Arduino sketch — open `RGB-DPL32.ino` in the Arduino IDE; the three `rgbdpl_*.h`
headers in this folder are part of the sketch.

## Arduino IDE setup

- Board: `ESP32 Dev Module`
- Partition Scheme: Default; baud rate `9600`, line ending Newline
- Required library: **FastLED 3.10.3** (pinned — newer/older RMT changes can boot-loop with this many strips)

## Differences vs the ESP32-S3 Mini variant

- **6 LED strips (A–F), no CSL** (there is no Strip G) — 144 LEDs total
- LED data pins: A=25, B=27, C=32, D=4, E=19 (LDPL), F=13; door switches 16/17; VCC sense GPIO0 (ADC2)
- Local **Web UI / Access Point** (http://192.168.4.1), same as the S3 Mini
- Selectable **startup style** (`STARTUP 0|1`) and per-panel **serial on/off** (`ON <panel>` / `OFF <panel>`,
  `ALLON` / `ALLOFF`)

For full setup, wiring, all commands, animations and troubleshooting, see the user manual in the
repository root (`RGB-DPL_Manual_EN.pdf` / `RGB-DPL_Handbuch_DE.pdf`).
