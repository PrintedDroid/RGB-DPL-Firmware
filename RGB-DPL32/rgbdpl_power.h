#pragma once

#include <Preferences.h>

enum RgbDplPanel : uint8_t {
    PANEL_BOTTOM, PANEL_RED, PANEL_BLUE, PANEL_BARGRAPH, PANEL_TOPBLOCKS,
    PANEL_VU, PANEL_LDPL, PANEL_CBI, PANEL_CSL, PANEL_COUNT
};

static const char* const rgbdplPanelNames[PANEL_COUNT] = {
    "bottom", "red", "bluecolumn", "bargraph", "topblocks",
    "vu", "ldpl", "cbi", "csl"
};
static const uint16_t rgbdplAllPanels = (1u << (PANEL_COUNT - !RGBDPL_HAS_CSL)) - 1u;
static uint16_t rgbdplPanelMask = rgbdplAllPanels;
static uint8_t rgbdplStartupStyle = 1;

static bool rgbdplPanelEnabled(RgbDplPanel panel) {
    return (rgbdplPanelMask & (1u << panel)) != 0;
}

static void rgbdplLoadPower() {
    Preferences prefs;
    if (!prefs.begin("rgbdpl-power", true)) return;
    rgbdplPanelMask = prefs.getUShort("panels", rgbdplAllPanels) & rgbdplAllPanels;
    rgbdplStartupStyle = prefs.getUChar("style", 1) == 0 ? 0 : 1;
    prefs.end();
}

static bool rgbdplSaveMask(uint16_t mask) {
    Preferences prefs;
    if (!prefs.begin("rgbdpl-power", false)) return false;
    bool saved = prefs.putUShort("panels", mask) == sizeof(mask);
    prefs.end();
    if (saved) rgbdplPanelMask = mask;
    return saved;
}

static bool rgbdplSaveStyle(uint8_t style) {
    Preferences prefs;
    if (!prefs.begin("rgbdpl-power", false)) return false;
    bool saved = prefs.putUChar("style", style) == sizeof(style);
    prefs.end();
    if (saved) rgbdplStartupStyle = style;
    return saved;
}

static void rgbdplMaskFrame() {
    if (!rgbdplPanelEnabled(PANEL_BOTTOM)) fill_solid(leds_a, 6, CRGB::Black);
    if (!rgbdplPanelEnabled(PANEL_RED)) fill_solid(leds_a + 6, 2, CRGB::Black);
    if (!rgbdplPanelEnabled(PANEL_BLUE)) {
        for (uint8_t i = 0; i < 6; ++i) leds_c[i * 4] = CRGB::Black;
    }
    if (!rgbdplPanelEnabled(PANEL_BARGRAPH)) {
        for (uint8_t i = 0; i < 6; ++i) fill_solid(leds_c + i * 4 + 1, 3, CRGB::Black);
    }
    if (!rgbdplPanelEnabled(PANEL_TOPBLOCKS)) fill_solid(leds_d, NUM_LEDS_D, CRGB::Black);
    if (!rgbdplPanelEnabled(PANEL_VU)) fill_solid(leds_b, NUM_LEDS_B, CRGB::Black);
    if (!rgbdplPanelEnabled(PANEL_LDPL)) fill_solid(leds_e, NUM_LEDS_E, CRGB::Black);
    if (!rgbdplPanelEnabled(PANEL_CBI)) fill_solid(leds_f, NUM_LEDS_F, CRGB::Black);
#if RGBDPL_HAS_CSL
    if (!rgbdplPanelEnabled(PANEL_CSL)) fill_solid(leds_g, NUM_LEDS_G, CRGB::Black);
#endif
}

static void rgbdplShowMasked() {
    rgbdplMaskFrame();
    FastLED.show();
}

static void rgbdplPrintPowerStatus() {
    commandOutput->print(F("Startup style: "));
    commandOutput->println(rgbdplStartupStyle);
    commandOutput->print(F("Panels:"));
    for (uint8_t i = 0; i < PANEL_COUNT - !RGBDPL_HAS_CSL; ++i) {
        commandOutput->print(' ');
        commandOutput->print(rgbdplPanelNames[i]);
        commandOutput->print('=');
        commandOutput->print(rgbdplPanelMask & (1u << i) ? F("ON") : F("OFF"));
    }
    commandOutput->println();
}

static bool rgbdplPowerCommand(const char* command, const char* arg) {
    if (!strcmp(command, "STARTUP")) {
        if (!arg || (strcmp(arg, "0") && strcmp(arg, "1"))) {
            commandOutput->println(F("ERROR: STARTUP 0|1"));
        } else if (rgbdplSaveStyle(arg[0] - '0')) {
            commandOutput->println(F("OK,STARTUP (takes effect next boot)"));
        } else commandOutput->println(F("ERROR: NVS save failed"));
        return true;
    }
    if (!strcmp(command, "ALLON") || !strcmp(command, "ALLOFF")) {
        uint16_t mask = !strcmp(command, "ALLON") ? rgbdplAllPanels : 0;
        if (!rgbdplSaveMask(mask)) commandOutput->println(F("ERROR: NVS save failed"));
        else {
            if (!mask) { FastLED.clear(); FastLED.show(); }
            commandOutput->println(F("OK"));
        }
        return true;
    }
    if (strcmp(command, "ON") && strcmp(command, "OFF")) return false;
    if (!arg) { commandOutput->println(F("ERROR: ON/OFF <panel>")); return true; }
    int8_t panel = -1;
    for (uint8_t i = 0; i < PANEL_COUNT - !RGBDPL_HAS_CSL; ++i) {
        if (!strcasecmp(arg, rgbdplPanelNames[i])) { panel = i; break; }
    }
    if (panel < 0) { commandOutput->println(F("ERROR: Unknown panel; see HELP")); return true; }
    uint16_t bit = 1u << panel;
    uint16_t mask = !strcmp(command, "ON") ? rgbdplPanelMask | bit : rgbdplPanelMask & ~bit;
    if (!rgbdplSaveMask(mask)) commandOutput->println(F("ERROR: NVS save failed"));
    else {
        if (!strcmp(command, "OFF")) rgbdplShowMasked();
        commandOutput->println(F("OK"));
    }
    return true;
}

// Adapted from D-DPL AstroCan X startupAnimation (animation 3).
static void rgbdplProxStartup() {
    FastLED.clear();
    for (uint8_t frame = 0; frame < 15; ++frame) {
        esp_task_wdt_reset();
        fill_solid(leds_b, frame * NUM_LEDS_B / 14, colorTable[SEC_VU_METER]);
        rgbdplShowMasked(); delay(200);
    }
    for (uint8_t frame = 0; frame < 30; ++frame) {
        esp_task_wdt_reset();
        fill_solid(leds_a, 6, random(2) ? colorTable[SEC_BOTTOM_WHITE] : CRGB::Black);
        rgbdplShowMasked(); delay(75);
    }
    fill_solid(leds_a, 6, colorTable[SEC_BOTTOM_WHITE]);
    for (uint8_t frame = 0; frame < 10; ++frame) {
        esp_task_wdt_reset();
        bool lit = random(2);
        fill_solid(leds_d, 9, lit ? colorTable[SEC_TOP_YELLOW] : CRGB::Black);
        fill_solid(leds_d + 9, 9, lit ? colorTable[SEC_TOP_GREEN] : CRGB::Black);
        rgbdplShowMasked(); delay(100);
    }
    fill_solid(leds_d, 9, colorTable[SEC_TOP_YELLOW]);
    fill_solid(leds_d + 9, 9, colorTable[SEC_TOP_GREEN]);
    for (uint8_t frame = 0; frame < 10; ++frame) {
        esp_task_wdt_reset();
        fill_solid(leds_a + 6, 2, random(2) ? colorTable[SEC_RED_LARGE] : CRGB::Black);
        rgbdplShowMasked(); delay(50);
    }
    fill_solid(leds_a + 6, 2, colorTable[SEC_RED_LARGE]);
    for (uint8_t level = 1; level <= 6; ++level) {
        esp_task_wdt_reset();
        uint8_t base = (6 - level) * 4 + 1;
        fill_solid(leds_c + base, 3, colorTable[SEC_BARGRAPH_LOW]);
        rgbdplShowMasked(); delay(300);
    }
    for (uint8_t i = 0; i < 6; ++i) leds_c[i * 4] = colorTable[SEC_BLUE_COLUMN];
    rgbdplShowMasked(); delay(150);
    fill_solid(leds_e, NUM_LEDS_E, colorTable[SEC_LDPL]);
    rgbdplShowMasked(); delay(150);
    fill_solid(leds_f, NUM_LEDS_F, colorTable[SEC_CBI_MATRIX]);
    rgbdplShowMasked(); delay(150);
#if RGBDPL_HAS_CSL
    fill_solid(leds_g, NUM_LEDS_G, colorTable[SEC_CSL]);
    rgbdplShowMasked(); delay(150);
#endif
    esp_task_wdt_reset();
    FastLED.clear(); FastLED.show();
}
