# RGB-DPL Nano / Pro Mini v5.2

## Overview

This is the Nano / Pro Mini build of the RGB-DPL firmware.

It targets ATmega328P hardware and includes:
- DPL
- CBI
- LDPL
- CSL
- VCC configuration commands `VM`, `VL`, `RR`

This version is intended for existing Nano / Pro Mini hardware. It is a constrained build and sits very close to the ATmega328P flash and RAM limits.

Main sketch:
- [`RGB_DPL-Nano_ProMini_v5.2.ino`](./RGB_DPL-Nano_ProMini_v5.2.ino)

## When To Use

Use this version if:
- your controller board is an Arduino Nano or Pro Mini
- you need a Nano-based sketch with `LDPL` and `CSL`
- you accept tighter limits and reduced headroom

Do not use this version if:
- your board is ESP32-S3 based
- you want the fullest current feature set
- you want comfortable space for future expansion

## Hardware Mapping

### LED Outputs

| Pin | Strip | LEDs | Function |
|---|---|---:|---|
| `D2` | A | 8 | Bottom white + large red |
| `D3` | B | 28 | VU meter |
| `D9` | C | 24 | Blue column + bargraph |
| `D8` | D | 18 | Top yellow + green |
| `D7` | E | 43 | `LDPL` |
| `D6` | F | 23 | `CBI` matrix + status |
| `D5` | G | 36 | `CSL` |

### Inputs

| Pin | Function |
|---|---|
| `A2 / 16` | Left door sensor |
| `A1 / 15` | Right door sensor |
| `A0` | Voltage monitor |

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
- serial CLI at `57600`
- `LDPLMODE`
- `CSLMODE`
- `CSLSPEED`
- `CSLBRIGHTNESS`
- `LLMODE` alias for `LDPLMODE`

Reduced to fit on Nano:
- no active `I2C/Wire` support
- no real config-state mode
- reduced CBI feature depth
- shortened help/status output

CBI note:
- `CBIMODE 0-6` is still accepted
- several mode numbers share the same underlying implementation to save flash

## Serial Commands

```text
HELP
STATUS
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
TOPBLOCKS <RANDOM|CLASSIC>
VM <0|1>
VL <RED YELLOW GREEN CHARGE>
RR <R1 R2>
```

Examples:

```text
LDPLMODE 1
CSLMODE 3
CSLSPEED 7
CSLBRIGHTNESS 35
```

## Build Notes

Successfully compiled for:
- `arduino:avr:nano`

Latest successful build:
- flash: `30676 / 30720`
- RAM: `1825 / 2048`

This version fits, but has very little reserve left.

## Arduino IDE Setup

Board settings:
- Board: `Arduino Nano`
- Processor: `ATmega328P`
- For some clones: `ATmega328P (Old Bootloader)`

Required library:
- `FastLED`

Serial monitor:
- baud rate: `57600`
- line ending: newline

## Recommendation

Choose this version only for Nano / Pro Mini hardware that already exists.

For the preferred mainline version, use:
- [`../RGB_DPL-ESP32_S3_Mini_v6.0/`](../RGB_DPL-ESP32_S3_Mini_v6.0/)
