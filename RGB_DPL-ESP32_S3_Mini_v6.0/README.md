# RGB-DPL ESP32-S3 Mini v6.0

## Overview

This is the main ESP32-S3 build of the RGB-DPL firmware.

It targets ESP32-S3 Mini hardware and includes:
- DPL
- CBI
- LDPL
- CSL
- VCC configuration commands `VM`, `VL`, `RR`

This is the recommended current version for the project and the preferred base for future work.

Main sketch:
- [`RGB_DPL-ESP32_S3_Mini_v6.0.ino`](./RGB_DPL-ESP32_S3_Mini_v6.0.ino)

## When To Use

Use this version if:
- your controller board is ESP32-S3 based
- you want the best current RGB-DPL firmware
- you want `LDPL` and `CSL` on one controller
- you want the most room for future expansion

Do not use this version if:
- your board is Nano / Pro Mini based

## Hardware Mapping

### LED Outputs

| Pin | Strip | LEDs | Function |
|---|---|---:|---|
| `IO4` | A | 8 | Bottom white + large red |
| `IO5` | B | 28 | VU meter |
| `IO6` | C | 24 | Blue column + bargraph |
| `IO7` | D | 18 | Top yellow + green |
| `IO8` | E | 43 | `LDPL` |
| `IO9` | F | 23 | `CBI` matrix + status |
| `IO13` | G | 36 | `CSL` |

### Inputs

| Pin | Function |
|---|---|
| `IO10` | Left door sensor |
| `IO11` | Right door sensor |
| `IO12` | Voltage monitor |

## Voltage Monitor

Default voltage divider:
- `R1 = 100k`
- `R2 = 10k`

Default thresholds:
- red: `11.5V`
- yellow: `12.0V`
- green: `12.5V`
- charge: `13.0V`

Available commands:
- `VM <0|1>`
- `VL <RED YELLOW GREEN CHARGE>`
- `RR <R1 R2>`

Examples:

```text
VM 1
VL 11.5 12.0 12.5 13.0
RR 100000 10000
```

## Features

Active in this build:
- profiles in EEPROM
- color schemes
- personality modes
- DPL / VU / CBI / LDPL / CSL
- sequence mode
- battery monitor
- serial CLI at `9600`
- I2C support
- watchdog support
- overflow-safe timing
- `INFO`
- `LDPLMODE`
- `CSLMODE`
- `CSLSPEED`
- `CSLBRIGHTNESS`
- `LLMODE` alias for `LDPLMODE`

This is the full-featured current project version, not a trimmed fallback.

## Serial Commands

```text
HELP
STATUS
INFO
CONFIG
EXIT
SAVE <1-5>
LOAD <1-5>
DEFAULT <0-5>
BRIGHTNESS <1-100>
SPEED <1-10>
CBISPEED <1-10>
SCHEME <name|0-7>
PERSONALITY <0-4>
SEQUENCE <ON|OFF>
VOLTAGE <ON|OFF>
COLOR <sec> <R> <G> <B>
LDPLMODE <0-3>
LLMODE <0-3>
CBIMODE <0-6>
CSLMODE <0-7>
CSLSPEED <1-10>
CSLBRIGHTNESS <1-100>
BARGRAPH <SPLIT|CLASSIC>
TOPBLOCKS <0-4>
VM <0|1>
VL <RED YELLOW GREEN CHARGE>
RR <R1 R2>
P0-P4
S0-S7
```

Examples:

```text
LDPLMODE 3
CSLMODE 4
CSLSPEED 6
CSLBRIGHTNESS 40
INFO
```

## Build Notes

Successfully compiled for:
- `esp32:esp32:lolin_s3_mini`

Latest successful build:
- flash: `397446 / 1310720`
- RAM: `25788 / 327680`

This version has comfortable headroom.

## Arduino IDE Setup

Board suggestions:
- `LOLIN S3 Mini`
- or `ESP32S3 Dev Module`

Important settings:
- `USB CDC On Boot`: enabled

Required library:
- `FastLED`

Serial monitor:
- baud rate: `9600`
- line ending: newline

## Recommendation

This is the preferred version for any ESP32-S3 hardware and for new work.

Use the Nano fallback only when the hardware is already locked to Nano / Pro Mini:
- [`../RGB_DPL-Nano_ProMini_v5.2/`](../RGB_DPL-Nano_ProMini_v5.2/)
