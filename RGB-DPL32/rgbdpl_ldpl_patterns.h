#pragma once
// Linear patterns ported from RGB-DPL AstroCan X, without its transport.
struct StandaloneLdplState {
    uint8_t animation=1, animCounter=0;
    bool playForward=true;
    CRGB baseColor=CRGB::Red;
    uint32_t lastAnimStep=0, lastEffectStep=0;
};
static StandaloneLdplState ldpl;
static CRGB patternColor(const CRGB& base, uint8_t hueOffset = 0) {
    if (base.r || base.g || base.b) return base;
    return CHSV(gHue + hueOffset, 220, 255);
}

static void patternBeatBars(CRGB* leds, uint16_t count, const CRGB& base, uint8_t rows) {
    fill_solid(leds, count, CRGB::Black);
    uint8_t beat = beatsin8(60, 0, rows + 1);
    uint8_t rowSize = (count / rows) > 0 ? (uint8_t)(count / rows) : 1;
    for (uint8_t row = 0; row < rows && row < beat; row++) {
        uint16_t start = (rows - 1 - row) * rowSize;
        for (uint8_t i = 0; i < rowSize && start + i < count; i++) leds[start + i] = patternColor(base, row * 24);
    }
}

static void patternBreathe(CRGB* leds, uint16_t count, const CRGB& base) {
    CRGB c = patternColor(base);
    c.nscale8_video(beatsin8(18, 25, 255));
    fill_solid(leds, count, c);
}

static void patternJuggle(CRGB* leds, uint16_t count, const CRGB& base) {
    fadeToBlackBy(leds, count, 22);
    for (uint8_t i = 0; i < 7; i++) leds[beatsin16((uint16_t)(i + 7), 0, count - 1)] |= patternColor(base, i * 32);
}

static void patternSparkle(CRGB* leds, uint16_t count, const CRGB& base) {
    fadeToBlackBy(leds, count, 26);
    if (random8() < 120) leds[random16(count)] += patternColor(base, random8(64));
}

static void patternComet(CRGB* leds, uint16_t count, const CRGB& base) {
    fadeToBlackBy(leds, count, 42);
    leds[beatsin16(20, 0, count - 1)] = patternColor(base);
}

static void patternAlarm(CRGB* leds, uint16_t count, const CRGB& base) {
    CRGB c = (base.r || base.g || base.b) ? base : CRGB::Red;
    fill_solid(leds, count, beatsin8(180, 0, 2) ? c : CRGB::Black);
}

static void patternFire(CRGB* leds, uint16_t count, uint8_t* heat, uint16_t heatCount) {
    for (uint16_t i = 0; i < heatCount; i++) heat[i] = qsub8(heat[i], random8(0, ((55 * 10) / heatCount) + 2));
    for (int k = heatCount - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    if (random8() < 120) {
        uint8_t y = random8(heatCount);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }
    for (uint16_t j = 0; j < count && j < heatCount; j++) leds[j] = HeatColor(heat[j]);
}

static void patternLogicSweep(CRGB* leds, uint16_t count, const CRGB& base) {
    fadeToBlackBy(leds, count, 45);
    uint16_t pos = beatsin16(32, 0, count - 1);
    for (uint16_t i = 0; i < count; i++) {
        uint16_t dist = (i > pos) ? (i - pos) : (pos - i);
        if (dist < 5) {
            CRGB c = patternColor(base, i * 3);
            c.nscale8(255 - dist * 42);
            leds[i] += c;
        }
    }
}

static void patternRainbowBars(CRGB* leds, uint16_t count, uint8_t rows) {
    uint8_t wave = beatsin8(18, 80, 255);
    for (uint8_t row = 0; row < rows; row++) {
        CRGB c = CHSV(gHue + row * 32, 240, wave);
        const uint16_t start = uint32_t(count) * row / rows;
        const uint16_t end = uint32_t(count) * (row + 1) / rows;
        for (uint16_t i = start; i < end; i++) leds[i] = c;
    }
}

static void patternTheaterChase(CRGB* leds, uint16_t count, const CRGB& base) {
    fill_solid(leds, count, CRGB::Black);
    uint8_t phase = (millis() / 180) % 3;
    for (uint16_t i = 0; i < count; i++) {
        if ((i + phase) % 3 == 0) leds[i] = patternColor(base, i * 7);
    }
}

void renderStandaloneLdpl() {
    static uint8_t previousMode=255;
    if(previousMode!=activeLdplMode()) {
        previousMode=activeLdplMode();ldpl.animCounter=0;ldpl.playForward=true;
        fill_solid(leds_e,NUM_LEDS_E,CRGB::Black);
    }
    ldpl.animation = activeLdplMode();
    if (ldpl.animation == 0) { fill_solid(leds_e, NUM_LEDS_E, CRGB::Black); return; }
    uint32_t now = millis();

    uint32_t animSpeed;
    switch (ldpl.animation) {
        case 1: animSpeed = 30; break;
        case 2: animSpeed = 30; break;
        case 3: animSpeed = 30; break;
        case 4: animSpeed = 45; break;
        case 5: animSpeed = 35; break;
        case 6: case 7: case 8: case 9: case 10:
        case 11: case 12: case 13: case 14: case 15:
            animSpeed = 20;
            break;
        default: return;
    }

    if (now - ldpl.lastAnimStep < animSpeed) return;
    ldpl.lastAnimStep = now;
    if (ldpl.animation == 2 || ldpl.animation == 3 || ldpl.animation == 5) {
        ldpl.lastEffectStep = now;
        fadeToBlackBy(leds_e, NUM_LEDS_E, 25);
    }

    switch (ldpl.animation) {
        case 1: { // defaultAnimation — single dot bouncing (clear all first)
            for (uint8_t i = 0; i < NUM_LEDS_E; i++)
                leds_e[i] = CRGB::Black;

            leds_e[ldpl.animCounter] = ldpl.baseColor;

            if (ldpl.playForward) {
                ldpl.animCounter++;
                if (ldpl.animCounter >= NUM_LEDS_E) {
                    ldpl.playForward = false;
                    ldpl.animCounter = NUM_LEDS_E - 1;
                }
            } else {
                if (ldpl.animCounter == 0) {
                    ldpl.playForward = true;
                } else {
                    ldpl.animCounter--;
                }
            }
            break;
        }
        case 2: { // fadeOutAnimation — moving dot with fade
            leds_e[ldpl.animCounter] = ldpl.baseColor;

            if (ldpl.playForward) {
                ldpl.animCounter++;
                if (ldpl.animCounter >= NUM_LEDS_E) {
                    ldpl.playForward = false;
                    ldpl.animCounter = NUM_LEDS_E - 1;
                }
            } else {
                if (ldpl.animCounter == 0) {
                    ldpl.playForward = true;
                } else {
                    ldpl.animCounter--;
                }
            }
            break;
        }
        case 3: { // rainbowAnimation — rainbow color cycling dot
            gHue += 8;

            leds_e[ldpl.animCounter] = CHSV(gHue, 255, 192);

            if (ldpl.playForward) {
                ldpl.animCounter++;
                if (ldpl.animCounter >= NUM_LEDS_E) {
                    ldpl.playForward = false;
                    ldpl.animCounter = NUM_LEDS_E - 1;
                }
            } else {
                if (ldpl.animCounter == 0) {
                    ldpl.playForward = true;
                } else {
                    ldpl.animCounter--;
                }
            }
            break;
        }
        case 4: { // kittScannerAnimation - one clean dot bouncing
            fill_solid(leds_e, NUM_LEDS_E, CRGB::Black);
            CRGB scanColor = ldpl.baseColor;
            if (scanColor.r == 0 && scanColor.g == 0 && scanColor.b == 0) scanColor = CRGB::Red;
            leds_e[ldpl.animCounter] = scanColor;

            if (ldpl.playForward) {
                ldpl.animCounter++;
                if (ldpl.animCounter >= NUM_LEDS_E) {
                    ldpl.playForward = false;
                    ldpl.animCounter = NUM_LEDS_E - 1;
                }
            } else {
                if (ldpl.animCounter == 0) {
                    ldpl.playForward = true;
                } else {
                    ldpl.animCounter--;
                }
            }
            break;
        }
        case 5: { // cylonTrailAnimation - scanner with soft trailing glow
            CRGB scanColor = ldpl.baseColor;
            if (scanColor.r == 0 && scanColor.g == 0 && scanColor.b == 0) scanColor = CRGB::Red;
            CRGB dimColor = scanColor;
            dimColor.nscale8_video(120);
            leds_e[ldpl.animCounter] = scanColor;
            if (ldpl.animCounter > 0) leds_e[ldpl.animCounter - 1] += dimColor;
            if (ldpl.animCounter + 1 < NUM_LEDS_E) leds_e[ldpl.animCounter + 1] += dimColor;

            if (ldpl.playForward) {
                ldpl.animCounter++;
                if (ldpl.animCounter >= NUM_LEDS_E) {
                    ldpl.playForward = false;
                    ldpl.animCounter = NUM_LEDS_E - 1;
                }
            } else {
                if (ldpl.animCounter == 0) {
                    ldpl.playForward = true;
                } else {
                    ldpl.animCounter--;
                }
            }
            break;
        }
        case 6:  patternBeatBars(leds_e, NUM_LEDS_E, ldpl.baseColor, 6); break;
        case 7:  patternBreathe(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
        case 8:  patternJuggle(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
        case 9:  patternSparkle(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
        case 10: patternComet(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
        case 11: patternAlarm(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
        case 12: {
            static uint8_t ldplHeat[NUM_LEDS_E];
            patternFire(leds_e, NUM_LEDS_E, ldplHeat, NUM_LEDS_E);
            break;
        }
        case 13: patternLogicSweep(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
        case 14: patternRainbowBars(leds_e, NUM_LEDS_E, 6); break;
        case 15: patternTheaterChase(leds_e, NUM_LEDS_E, ldpl.baseColor); break;
    }
}


