/*
 * Enhanced RGB-DPL/CBI/LDPL/CSL Controller v5.2 (Experimental)
 * For Printed-Droid R2-D2 Replica Display Systems
 *
 * Controls multiple LED display panels (DPL, CBI, LDPL, CSL) for an R2-D2
 * replica using WS2811/WS2812B addressable RGB LEDs.
 *
 * Hardware Requirements:
 * - Arduino Nano or compatible
 * - 7x WS2811 LED strips (180 total LEDs)
 * - Door sensors on pins 15 & 16
 * - Optional: Voltage divider for battery monitoring on A0 (100k/10k)
 *
 * Features:
 * - Fully addressable RGB control for all panels
 * - User profiles, color schemes, and personality modes saved to EEPROM
 * - Case-insensitive serial command parser (C-style, no String objects)
 * - I2C slave support for integration with other systems (e.g., Marcduino)
 * - Door sensors to activate/deactivate displays automatically
 *
 * ============================================================================
 *
 * Serial Commands (57600 baud, commands must end with a newline '
'):
 *
 * --- System & Profiles ---
 * CONFIG, EXIT, HELP, STATUS
 * SAVE <1-5>, LOAD <1-5>, DEFAULT <1-5>
 *
 * --- Global Settings ---
 * BRIGHTNESS <1-100>, SPEED <1-10>
 * SCHEME <name|num>, PERSONALITY <0-4>
 * SEQUENCE <ON|OFF>, VOLTAGE <ON|OFF>
 * COLOR <s r g b>
 *
 * --- Panel Specific Modes ---
 * LDPLMODE <0-3>, CBIMODE <0-6>, CBISPEED <1-10>
 * CSLMODE <0-7>, CSLSPEED <1-10>, CSLBRIGHTNESS <1-100>
 * BARGRAPH <SPLIT|CLASSIC>
 * TOPBLOCKS <RANDOM|CLASSIC>
 *
 * ============================================================================
 *
 * LED Strip Assignments & Layout:
 * - Strip A (8 LEDs, Pin D2):  Bottom white (6) + Large red (2)
 * - Strip B (28 LEDs, Pin D3): VU-Meter display
 * - Strip C (24 LEDs, Pin D9): Right panel - Blue column (6) + Bargraph (18)
 * - Strip D (18 LEDs, Pin D8): Top panel - Yellow blocks (9) + Green blocks (9)
 * - Strip E (43 LEDs, Pin D7): LDPL / Logic Display Panel
 * - Strip F (23 LEDs, Pin D6): CBI - Matrix (20) + Status lights (3)
 * - Strip G (36 LEDs, Pin D5): CSL / Coin Slot Logics
 *
 * ============================================================================
 *
 * v5.2 Changes:
 * - Renamed Large Logic to LDPL throughout the CLI
 * - Re-added CSL on D5 as a best-effort Nano test build
 * - Added VCC configuration commands VM / VL / RR
 * - Default voltage divider remains 100k/10k
 *
 * v5.1.1 Changes:
 * - CRITICAL FIX: Removed CSL to fix SRAM overflow crash (leds_g[36]
 *   consumed 108 bytes, leaving only 63 bytes for stack = reset loop).
 *   CSL should run on a separate dedicated controller.
 * - Fixed cmdBuffer stack overflow (was 64 bytes on stack, now in-place)
 * - Reduced serial buffer from 32 to 24 bytes
 *
 * v5.1 Changes:
 * - Fixed compilation error (orphaned comment block without opening tag)
 * - Consolidated 4 redundant header blocks into one
 * - Fixed EEPROM wear: bootCount only written every 10th boot
 * - Added blinking red LED warning when battery voltage drops below 11.5V
 * - Formatted matrixRain() for readability
 * - Fixed voltage divider resistor values (100k/10k)
 *
 * ============================================================================
 */

// ============================================================================
// Libraries
// ============================================================================
#include <FastLED.h>
#include <EEPROM.h>

// ============================================================================
// Version and Debug
// ============================================================================
#define VERSION "5.2.0-NANO"

// ============================================================================
// Pin Definitions
// ============================================================================
#define LEFT_DOOR_PIN  16
#define RIGHT_DOOR_PIN 15
#define ANALOG_INPUT   A0

// LED Data Pins
#define DATA_PIN_A 2
#define DATA_PIN_B 3
#define DATA_PIN_C 9
#define DATA_PIN_D 8
#define DATA_PIN_E 7
#define DATA_PIN_F 6
#define DATA_PIN_G 5

// LED Counts for each strip
#define NUM_LEDS_A 8
#define NUM_LEDS_B 28
#define NUM_LEDS_C 24
#define NUM_LEDS_D 18
#define NUM_LEDS_E 43
#define NUM_LEDS_F 23
#define NUM_LEDS_G 36
#define TOTAL_LEDS (NUM_LEDS_A + NUM_LEDS_B + NUM_LEDS_C + NUM_LEDS_D + NUM_LEDS_E + NUM_LEDS_F + NUM_LEDS_G)

#define STARTUP_LED_DELAY  15
#define STARTUP_FLASH_DELAY 150

// ============================================================================
// Voltage Monitor Configuration
// ============================================================================
#define DEFAULT_LIMIT_GREEN 12.5f
#define DEFAULT_LIMIT_YELLOW 12.0f
#define DEFAULT_LIMIT_RED 11.5f
#define DEFAULT_LIMIT_CHARGE 13.0f
#define DEFAULT_R1 100000.0f
#define DEFAULT_R2 10000.0f

// ============================================================================
// Configuration Structures
// ============================================================================
enum ColorScheme { CLASSIC, BLUE, PINK, GREEN, CYBERPUNK, FOREST, SUNSET, CUSTOM, SCHEME_COUNT };
enum Personality { NORMAL, HAPPY, GRUMPY, EXCITED, SLEEPY, PERS_COUNT };
enum LEDSection { SEC_TOP_YELLOW, SEC_TOP_GREEN, SEC_BLUE_COLUMN, SEC_BARGRAPH_LOW, SEC_BARGRAPH_MID, SEC_BARGRAPH_HIGH, SEC_BOTTOM_WHITE, SEC_RED_LARGE, SEC_VU_METER, SEC_CBI_MATRIX, SEC_CBI_STATUS, SEC_LDPL, SEC_CSL, SEC_COUNT };

struct UserProfile {
    char name[8];
    uint8_t brightness;
    uint8_t animSpeed;
    uint8_t cbiSpeed;
    uint8_t colorScheme;
    uint8_t personality;
    uint8_t ldplMode;
    uint8_t cbiMode;
    uint8_t cslMode;
    bool bargraphSplit;
    bool topBlocksRandom;
    bool sequenceMode;
    bool voltageMonitorEnabled;
    uint8_t cslSpeed;
    uint8_t cslBrightness;
    CRGB customColors[SEC_COUNT];
    uint16_t checksum;
};

struct SystemConfig {
    uint8_t defaultProfile;
    uint16_t bootCount;
    float limitRed;
    float limitYellow;
    float limitGreen;
    float limitCharge;
    float r1;
    float r2;
    bool monitorVccEnabled;
    uint16_t checksum;
};

// ============================================================================
// PROGMEM Data (Stored in Flash Memory to save RAM)
// ============================================================================
const PROGMEM uint8_t PERSONALITY_PARAMS[][4] = { {10, 10, 0, 4}, {15, 12, 30, 5}, {5, 8, -30, 3}, {20, 15, 0, 6}, {3, 5, -60, 2} };

// ============================================================================
// Global Variables
// ============================================================================
CRGB leds_a[NUM_LEDS_A], leds_b[NUM_LEDS_B], leds_c[NUM_LEDS_C], leds_d[NUM_LEDS_D], leds_e[NUM_LEDS_E], leds_f[NUM_LEDS_F], leds_g[NUM_LEDS_G];
UserProfile currentProfile;
SystemConfig sysConfig;
CRGB colorTable[SEC_COUNT];
uint8_t gHue = 0;
byte bargraphData[2] = {3, 3};

class Timer {
    unsigned long lastTime = 0;
    uint16_t interval;
public:
    Timer(uint16_t ms) : interval(ms) {}
    void setInterval(uint16_t ms) { interval = ms; }
    bool ready() {
        if (millis() - lastTime >= interval) { lastTime = millis(); return true; }
        return false;
    }
};

Timer topBlockTimer(200), blueTimer(500), bargraphTimer(200), bottomTimer(200), redTimer(500), cbiTimer(50), ldplTimer(15), cslTimer(120), sequenceTimer(15000);
char serialBuffer[20];
byte serialBufferPos = 0;

bool isWaitingForCommand = false;
unsigned long lastSerialActivityTime = 0;

// ============================================================================
// EEPROM Management
// ============================================================================
#define EEPROM_MAGIC 0xDEAD
#define EEPROM_SYS_ADDR 0
#define EEPROM_PROFILE_ADDR(x) (sizeof(SystemConfig) + (x * sizeof(UserProfile)))

uint16_t calculateChecksum(void* data, size_t size) {
    uint16_t sum = 0;
    uint8_t* bytes = (uint8_t*)data;
    for (size_t i = 0; i < size - 2; i++) { sum += bytes[i]; }
    return sum ^ EEPROM_MAGIC;
}

void saveSystemConfig() {
    sysConfig.checksum = calculateChecksum(&sysConfig, sizeof(SystemConfig));
    EEPROM.put(EEPROM_SYS_ADDR, sysConfig);
}

void loadSystemConfig() {
    EEPROM.get(EEPROM_SYS_ADDR, sysConfig);
    if (sysConfig.checksum != calculateChecksum(&sysConfig, sizeof(SystemConfig))) {
        sysConfig.defaultProfile = 0;
        sysConfig.bootCount = 0;
        sysConfig.limitRed = DEFAULT_LIMIT_RED;
        sysConfig.limitYellow = DEFAULT_LIMIT_YELLOW;
        sysConfig.limitGreen = DEFAULT_LIMIT_GREEN;
        sysConfig.limitCharge = DEFAULT_LIMIT_CHARGE;
        sysConfig.r1 = DEFAULT_R1;
        sysConfig.r2 = DEFAULT_R2;
        sysConfig.monitorVccEnabled = false;
        saveSystemConfig();
    }
    sysConfig.bootCount++;
    // Only write to EEPROM every 10th boot to reduce wear (~100k write cycles)
    if (sysConfig.bootCount % 10 == 0) {
        saveSystemConfig();
    }
}

void initDefaultProfile();
void applyProfile();

void loadProfile(uint8_t profileNum) {
    if (profileNum >= 5) return;
    UserProfile temp;
    EEPROM.get(EEPROM_PROFILE_ADDR(profileNum), temp);
    if (temp.checksum == calculateChecksum(&temp, sizeof(UserProfile))) {
        currentProfile = temp;
        currentProfile.voltageMonitorEnabled = sysConfig.monitorVccEnabled;
        applyProfile();
        Serial.print(F("Loaded profile ")); Serial.println(profileNum + 1);
    } else {
        Serial.println(F("Profile empty/corrupt. Loading defaults. NOTE: Re-save profiles after sketch updates."));
        initDefaultProfile();
    }
}

void saveProfile(uint8_t profileNum) {
    if (profileNum >= 5) return;
    currentProfile.checksum = calculateChecksum(&currentProfile, sizeof(UserProfile));
    EEPROM.put(EEPROM_PROFILE_ADDR(profileNum), currentProfile);
    Serial.print(F("Saved to profile ")); Serial.println(profileNum + 1);
}

// ============================================================================
// Profile and Color Management
// ============================================================================
void updateColorTable() {
    switch (currentProfile.colorScheme) {
        case CLASSIC:    colorTable[SEC_TOP_YELLOW]=CRGB::Yellow; colorTable[SEC_TOP_GREEN]=CRGB::Green; colorTable[SEC_BLUE_COLUMN]=CRGB::Blue; colorTable[SEC_BARGRAPH_LOW]=CRGB::Green; colorTable[SEC_BARGRAPH_MID]=CRGB::Yellow; colorTable[SEC_BARGRAPH_HIGH]=CRGB::Red; colorTable[SEC_BOTTOM_WHITE]=CRGB::White; colorTable[SEC_RED_LARGE]=CRGB::Red; colorTable[SEC_CBI_MATRIX]=CRGB::Red; colorTable[SEC_CBI_STATUS]=CRGB::Red; colorTable[SEC_LDPL]=CRGB::Blue; colorTable[SEC_CSL]=CRGB::Blue; break;
        case BLUE:       colorTable[SEC_TOP_YELLOW]=CRGB::Cyan; colorTable[SEC_TOP_GREEN]=CRGB::Blue; colorTable[SEC_BLUE_COLUMN]=CRGB::DeepSkyBlue; colorTable[SEC_BARGRAPH_LOW]=CRGB::Blue; colorTable[SEC_BARGRAPH_MID]=CRGB::Cyan; colorTable[SEC_BARGRAPH_HIGH]=CRGB::White; colorTable[SEC_BOTTOM_WHITE]=CRGB::LightBlue; colorTable[SEC_RED_LARGE]=CRGB::Blue; colorTable[SEC_CBI_MATRIX]=CRGB::Blue; colorTable[SEC_CBI_STATUS]=CRGB::White; colorTable[SEC_LDPL]=CRGB::DeepSkyBlue; colorTable[SEC_CSL]=CRGB::Blue; break;
        case PINK:       colorTable[SEC_TOP_YELLOW]=CRGB::Pink; colorTable[SEC_TOP_GREEN]=CRGB::Magenta; colorTable[SEC_BLUE_COLUMN]=CRGB::Purple; colorTable[SEC_BARGRAPH_LOW]=CRGB::Pink; colorTable[SEC_BARGRAPH_MID]=CRGB::HotPink; colorTable[SEC_BARGRAPH_HIGH]=CRGB::Magenta; colorTable[SEC_BOTTOM_WHITE]=CRGB::LavenderBlush; colorTable[SEC_RED_LARGE]=CRGB::DeepPink; colorTable[SEC_CBI_MATRIX]=CRGB::Magenta; colorTable[SEC_CBI_STATUS]=CRGB::HotPink; colorTable[SEC_LDPL]=CRGB::Magenta; colorTable[SEC_CSL]=CRGB::DeepPink; break;
        case GREEN:      colorTable[SEC_TOP_YELLOW]=CRGB::YellowGreen; colorTable[SEC_TOP_GREEN]=CRGB::Green; colorTable[SEC_BLUE_COLUMN]=CRGB::SeaGreen; colorTable[SEC_BARGRAPH_LOW]=CRGB::DarkGreen; colorTable[SEC_BARGRAPH_MID]=CRGB::Green; colorTable[SEC_BARGRAPH_HIGH]=CRGB::Lime; colorTable[SEC_BOTTOM_WHITE]=CRGB::PaleGreen; colorTable[SEC_RED_LARGE]=CRGB::ForestGreen; colorTable[SEC_CBI_MATRIX]=CRGB::Green; colorTable[SEC_CBI_STATUS]=CRGB::Lime; colorTable[SEC_LDPL]=CRGB::SeaGreen; colorTable[SEC_CSL]=CRGB::Green; break;
        case CYBERPUNK:  colorTable[SEC_TOP_YELLOW]=CRGB::Magenta; colorTable[SEC_TOP_GREEN]=CRGB::Purple; colorTable[SEC_BLUE_COLUMN]=CRGB::Cyan; colorTable[SEC_BARGRAPH_LOW]=CRGB::DeepPink; colorTable[SEC_BARGRAPH_MID]=CRGB::Cyan; colorTable[SEC_BARGRAPH_HIGH]=CRGB::Yellow; colorTable[SEC_BOTTOM_WHITE]=CRGB::LightCyan; colorTable[SEC_RED_LARGE]=CRGB::Magenta; colorTable[SEC_CBI_MATRIX]=CRGB::Cyan; colorTable[SEC_CBI_STATUS]=CRGB::Yellow; colorTable[SEC_LDPL]=CRGB::Cyan; colorTable[SEC_CSL]=CRGB::DeepPink; break;
        case FOREST:     colorTable[SEC_TOP_YELLOW]=CRGB::Orange; colorTable[SEC_TOP_GREEN]=CRGB::DarkGreen; colorTable[SEC_BLUE_COLUMN]=CRGB::SeaGreen; colorTable[SEC_BARGRAPH_LOW]=CRGB::Green; colorTable[SEC_BARGRAPH_MID]=CRGB::Brown; colorTable[SEC_BARGRAPH_HIGH]=CRGB::Orange; colorTable[SEC_BOTTOM_WHITE]=CRGB::LightGoldenrodYellow; colorTable[SEC_RED_LARGE]=CRGB::DarkOrange; colorTable[SEC_CBI_MATRIX]=CRGB::DarkGreen; colorTable[SEC_CBI_STATUS]=CRGB::Orange; colorTable[SEC_LDPL]=CRGB::DarkGreen; colorTable[SEC_CSL]=CRGB::Orange; break;
        case SUNSET:     colorTable[SEC_TOP_YELLOW]=CRGB::OrangeRed; colorTable[SEC_TOP_GREEN]=CRGB::DarkViolet; colorTable[SEC_BLUE_COLUMN]=CRGB::Orange; colorTable[SEC_BARGRAPH_LOW]=CRGB::Orange; colorTable[SEC_BARGRAPH_MID]=CRGB::Red; colorTable[SEC_BARGRAPH_HIGH]=CRGB::DarkViolet; colorTable[SEC_BOTTOM_WHITE]=CRGB::Gold; colorTable[SEC_RED_LARGE]=CRGB::OrangeRed; colorTable[SEC_CBI_MATRIX]=CRGB::DarkViolet; colorTable[SEC_CBI_STATUS]=CRGB::OrangeRed; colorTable[SEC_LDPL]=CRGB::DarkViolet; colorTable[SEC_CSL]=CRGB::Orange; break;
        case CUSTOM:     memcpy(colorTable, currentProfile.customColors, sizeof(colorTable)); break;
    }
    uint8_t persParams[4];
    memcpy_P(persParams, PERSONALITY_PARAMS[currentProfile.personality], 4);
    int8_t hueShift = (int8_t)persParams[2];
    if (hueShift != 0) {
        for (int i = 0; i < SEC_COUNT; i++) {
            CHSV hsv = rgb2hsv_approximate(colorTable[i]);
            hsv.h += hueShift;
            colorTable[i] = hsv;
        }
    }
}

void applyProfile() {
    FastLED.setBrightness(map(currentProfile.brightness, 1, 100, 10, 255));
    updateColorTable();
}

void initDefaultProfile() {
    strcpy(currentProfile.name, "Default");
    currentProfile.brightness = 25;
    currentProfile.animSpeed = 5;
    currentProfile.cbiSpeed = 5;
    currentProfile.colorScheme = CLASSIC;
    currentProfile.personality = NORMAL;
    currentProfile.ldplMode = 0;
    currentProfile.cbiMode = 0;
    currentProfile.cslMode = 1;
    currentProfile.bargraphSplit = false;
    currentProfile.topBlocksRandom = true;
    currentProfile.sequenceMode = false;
    currentProfile.voltageMonitorEnabled = sysConfig.monitorVccEnabled;
    currentProfile.cslSpeed = 5;
    currentProfile.cslBrightness = 25;
    memset(currentProfile.customColors, 0, sizeof(currentProfile.customColors));
    applyProfile();
}


// ============================================================================
// Startup Sequence
// ============================================================================
void startupSequence() {
    Serial.println(F("Running startup sequence..."));

    FastLED.clear();
    CRGB* strips[] = {leds_a, leds_b, leds_c, leds_d, leds_e, leds_f, leds_g};
    int counts[] = {NUM_LEDS_A, NUM_LEDS_B, NUM_LEDS_C, NUM_LEDS_D, NUM_LEDS_E, NUM_LEDS_F, NUM_LEDS_G};

    // Rainbow wave across all strips
    for (int hue = 0; hue < 255; hue += 64) {
        for (int i = 0; i < TOTAL_LEDS; i++) {
            int current = 0;
            for (int s = 0; s < 7; s++) {
                if (i >= current && i < current + counts[s]) {
                    strips[s][i - current] = CHSV(hue, 255, 255);
                }
                current += counts[s];
            }
            FastLED.show();

            for (int s = 0; s < 7; s++) {
                fadeToBlackBy(strips[s], counts[s], 64);
            }
            delay(STARTUP_LED_DELAY);
        }
    }

    // Flash all white 3x
    for (int i = 0; i < 3; i++) {
        for (int s = 0; s < 7; s++) {
            fill_solid(strips[s], counts[s], CRGB::White);
        }
        FastLED.show();
        delay(STARTUP_FLASH_DELAY);

        FastLED.clear();
        FastLED.show();
        delay(STARTUP_FLASH_DELAY);
    }

    FastLED.clear();
    FastLED.show();
    Serial.println(F("Startup complete."));
}

// ============================================================================
// Setup
// ============================================================================

void setup() {
    Serial.begin(57600);
    Serial.println(F("\n- RGB DPL v5.2 -"));

    pinMode(LEFT_DOOR_PIN, INPUT_PULLUP);
    pinMode(RIGHT_DOOR_PIN, INPUT_PULLUP);

    delay(2000);
    FastLED.addLeds<WS2811, DATA_PIN_A, GRB>(leds_a, NUM_LEDS_A);
    FastLED.addLeds<WS2811, DATA_PIN_B, GRB>(leds_b, NUM_LEDS_B);
    FastLED.addLeds<WS2811, DATA_PIN_C, GRB>(leds_c, NUM_LEDS_C);
    FastLED.addLeds<WS2811, DATA_PIN_D, GRB>(leds_d, NUM_LEDS_D);
    FastLED.addLeds<WS2811, DATA_PIN_E, GRB>(leds_e, NUM_LEDS_E);
    FastLED.addLeds<WS2811, DATA_PIN_F, GRB>(leds_f, NUM_LEDS_F);
    FastLED.addLeds<WS2811, DATA_PIN_G, GRB>(leds_g, NUM_LEDS_G);
    set_max_power_in_volts_and_milliamps(5, 2000);

    loadSystemConfig();
    if (sysConfig.defaultProfile > 0) {
        loadProfile(sysConfig.defaultProfile - 1);
    } else {
        initDefaultProfile();
    }

    startupSequence();
    Serial.println(F("Ready! Type HELP for a list of commands."));
}

// ============================================================================
// Main Loop
// ============================================================================
void updateCBILEDs();
void updateVuMeter();
void updateRedLEDs();
void updateBottomLEDs();
void updateBargraph();
void updateBlueLEDs();
void updateTopBlocks();
void processSerialCommand();
void updateLDPL();
void updateCSL();

// ============================================================================
// Main Loop (Final version with persistent state machine)
// ============================================================================
void loop() {
    // --- SERIAL HANDLING WITH PERSISTENT STATE ---
    // Check if any new serial data has arrived.
    if (Serial.available() > 0) {
        isWaitingForCommand = true; // Set the state to "receiving command"
        lastSerialActivityTime = millis(); // Reset the timeout timer

        // Process all currently available characters quickly.
        while (Serial.available() > 0) {
            char inChar = Serial.read();

            // On newline, the command is complete.
            if (inChar == '\n') {
                if (serialBufferPos > 0) {
                    serialBuffer[serialBufferPos] = '\0';
                    processSerialCommand();
                }
                // Reset everything and exit the "receiving" state.
                serialBufferPos = 0;
                memset(serialBuffer, 0, sizeof(serialBuffer));
                isWaitingForCommand = false;
            } 
            // Ignore carriage returns.
            else if (inChar == '\r') {
                // Do nothing.
            }
            // Add printable characters to the buffer.
            else if (inChar >= 32 && inChar < 127) {
                if (serialBufferPos < sizeof(serialBuffer) - 1) {
                    serialBuffer[serialBufferPos++] = inChar;
                }
            }
        }
    }

    // --- TIMEOUT LOGIC ---
    // If we've been waiting for a command for too long, cancel it.
    // This prevents the system from getting stuck if a command is incomplete.
    if (isWaitingForCommand && (millis() - lastSerialActivityTime > 500)) { // 500ms is a good timeout
        isWaitingForCommand = false;
        serialBufferPos = 0;
        memset(serialBuffer, 0, sizeof(serialBuffer));
        Serial.println(F("Command timeout."));
    }

    // --- ANIMATION & LOGIC ---
    // Only run animations if we are NOT waiting for a serial command to complete.
    if (!isWaitingForCommand) {
        if (currentProfile.sequenceMode && sequenceTimer.ready()) {
            currentProfile.cbiMode = (currentProfile.cbiMode + 1) % 7;
            currentProfile.ldplMode = (currentProfile.ldplMode + 1) % 4;
            currentProfile.cslMode = (currentProfile.cslMode % 7) + 1;
        }

        uint8_t persParams[4];
        memcpy_P(persParams, PERSONALITY_PARAMS[currentProfile.personality], 4);
        float globalSpeedMult = (persParams[0] / 10.0) * (currentProfile.animSpeed / 5.0);
        float cbiSpeedMult = (persParams[0] / 10.0) * (currentProfile.cbiSpeed / 5.0);
        
        topBlockTimer.setInterval(200 / globalSpeedMult);
        blueTimer.setInterval(500 / globalSpeedMult);
        bargraphTimer.setInterval(200 / globalSpeedMult);
        bottomTimer.setInterval(200 / globalSpeedMult);
        redTimer.setInterval(500 / globalSpeedMult);
        cbiTimer.setInterval( (currentProfile.cbiMode == 1) ? (800 / cbiSpeedMult) : (100 / cbiSpeedMult) );
        cslTimer.setInterval(map(currentProfile.cslSpeed, 1, 10, 240, 20));

        bool dplDoorOpen = (digitalRead(LEFT_DOOR_PIN) == LOW);
        if (dplDoorOpen) {
            if (topBlockTimer.ready()) updateTopBlocks();
            if (blueTimer.ready()) updateBlueLEDs();
            if (bargraphTimer.ready()) updateBargraph();
            if (bottomTimer.ready()) updateBottomLEDs();
            if (redTimer.ready()) updateRedLEDs();
            updateVuMeter();
        } else {
            fill_solid(leds_a, NUM_LEDS_A, CRGB::Black); fill_solid(leds_b, NUM_LEDS_B, CRGB::Black);
            fill_solid(leds_c, NUM_LEDS_C, CRGB::Black); fill_solid(leds_d, NUM_LEDS_D, CRGB::Black);
        }

        bool cbiDoorOpen = (digitalRead(RIGHT_DOOR_PIN) == LOW);
        if (cbiDoorOpen) {
            if (cbiTimer.ready()) {
                updateCBILEDs();
            }
        } else {
            fill_solid(leds_f, NUM_LEDS_F, CRGB::Black);
        }
        
        updateLDPL();
        if (cslTimer.ready()) updateCSL();
        FastLED.show();
        EVERY_N_MILLISECONDS(20) { gHue++; }
    }
}

// ============================================================================
// Command Processing & UI
// ============================================================================
void printHelp();
void printStatus();

void processSerialCommand() {
    // Work directly on serialBuffer (strtok modifies in-place, buffer is reset after)
    char *command = strtok(serialBuffer, " ");

    // If no command was found after tokenizing (e.g., input was only spaces), just exit.
    // The loop() function will handle resetting the buffer.
    if (command == NULL) {
        return;
    }

    // Convert the command part to uppercase for case-insensitive comparison.
    for (int i = 0; command[i]; i++) {
        command[i] = toupper(command[i]);
    }

    // Get the first argument.
    char *arg1 = strtok(NULL, " ");
    
    // If the first argument exists, convert it to uppercase as well.
    if (arg1 != NULL) {
        for (int i = 0; arg1[i]; i++) {
            arg1[i] = toupper(arg1[i]);
        }
    }

    // --- Command Processing Logic ---
    if (strcmp(command, "CONFIG") == 0) { 
        printHelp(); 
    }
    else if (strcmp(command, "EXIT") == 0) { 
        Serial.println(F("OK")); 
    }
    else if (strcmp(command, "HELP") == 0) { 
        printHelp(); 
    }
    else if (strcmp(command, "STATUS") == 0) { 
        printStatus(); 
    }
    else if (strcmp(command, "SAVE") == 0) { 
        if (arg1 != NULL) {
            saveProfile(atoi(arg1) - 1); 
        } else {
            Serial.println(F("SAVE: 1-5"));
        }
    }
    else if (strcmp(command, "LOAD") == 0) { 
        if (arg1 != NULL) {
            loadProfile(atoi(arg1) - 1); 
        } else {
            Serial.println(F("LOAD: 1-5"));
        }
    }
    else if (strcmp(command, "DEFAULT") == 0) { 
        if (arg1 != NULL) {
            sysConfig.defaultProfile = atoi(arg1); 
            saveSystemConfig(); 
            Serial.print(F("Default profile set to: ")); 
            Serial.println(sysConfig.defaultProfile); 
        } else {
            Serial.println(F("DEFAULT: 0-5"));
        }
    }
    else if (strcmp(command, "BRIGHTNESS") == 0) { 
        if (arg1 != NULL) {
            currentProfile.brightness = constrain(atoi(arg1), 1, 100); 
            applyProfile(); 
            Serial.print(F("Brightness: ")); 
            Serial.println(currentProfile.brightness); 
        } else {
            Serial.println(F("BRIGHTNESS: 1-100"));
        }
    }
    else if (strcmp(command, "SPEED") == 0) { 
        if (arg1 != NULL) {
            currentProfile.animSpeed = constrain(atoi(arg1), 1, 10); 
            Serial.print(F("Global Speed: ")); 
            Serial.println(currentProfile.animSpeed); 
        } else {
            Serial.println(F("SPEED: 1-10"));
        }
    }
    else if (strcmp(command, "CBISPEED") == 0) { 
        if (arg1 != NULL) {
            currentProfile.cbiSpeed = constrain(atoi(arg1), 1, 10); 
            Serial.print(F("CBI Speed: ")); 
            Serial.println(currentProfile.cbiSpeed); 
        } else {
            Serial.println(F("CBISPEED: 1-10"));
        }
    }
    else if (strcmp(command, "PERSONALITY") == 0) { 
        if (arg1 != NULL) {
            currentProfile.personality = constrain(atoi(arg1), 0, PERS_COUNT - 1); 
            applyProfile(); 
            Serial.println(F("Personality updated.")); 
        } else {
            Serial.println(F("PERS: 0-4"));
        }
    }
    else if (strcmp(command, "LDPLMODE") == 0 || strcmp(command, "LLMODE") == 0) { 
        if (arg1 != NULL) {
            currentProfile.ldplMode = constrain(atoi(arg1), 0, 3); 
            Serial.println(F("LDPL mode updated.")); 
        } else {
            Serial.println(F("LDPLMODE: 0-3"));
        }
    }
    else if (strcmp(command, "CBIMODE") == 0) { 
        if (arg1 != NULL) {
            currentProfile.cbiMode = constrain(atoi(arg1), 0, 6); 
            Serial.println(F("CBI Mode updated.")); 
        } else {
            Serial.println(F("CBIMODE: 0-6"));
        }
    }
    else if (strcmp(command, "CSLMODE") == 0) {
        if (arg1 != NULL) {
            currentProfile.cslMode = constrain(atoi(arg1), 0, 7);
            Serial.println(F("CSL mode updated."));
        } else {
            Serial.println(F("CSLMODE: 0-7"));
        }
    }
    else if (strcmp(command, "CSLSPEED") == 0) {
        if (arg1 != NULL) {
            currentProfile.cslSpeed = constrain(atoi(arg1), 1, 10);
            Serial.println(F("CSL speed updated."));
        } else {
            Serial.println(F("CSLSPEED: 1-10"));
        }
    }
    else if (strcmp(command, "CSLBRIGHTNESS") == 0) {
        if (arg1 != NULL) {
            currentProfile.cslBrightness = constrain(atoi(arg1), 1, 100);
            Serial.println(F("CSL brightness updated."));
        } else {
            Serial.println(F("CSLBRIGHTNESS: 1-100"));
        }
    }
    else if (strcmp(command, "BARGRAPH") == 0) { 
        if (arg1 != NULL) {
            if (strcmp(arg1, "SPLIT")==0) {
                currentProfile.bargraphSplit = true; 
            } else if (strcmp(arg1, "CLASSIC")==0) {
                currentProfile.bargraphSplit = false; 
            }
            Serial.print(F("Bargraph: ")); 
            Serial.println(currentProfile.bargraphSplit ? "Split" : "Classic"); 
        } else {
            Serial.println(F("BARGRAPH: SPLIT|CLASSIC"));
        }
    }
    else if (strcmp(command, "TOPBLOCKS") == 0) { 
        if (arg1 != NULL) {
            if (strcmp(arg1, "RANDOM")==0) {
                currentProfile.topBlocksRandom = true; 
            } else if (strcmp(arg1, "CLASSIC")==0) {
                currentProfile.topBlocksRandom = false; 
            }
            Serial.print(F("Top Blocks: ")); 
            Serial.println(currentProfile.topBlocksRandom ? "Random" : "Classic"); 
        } else {
            Serial.println(F("TOPBLOCKS: RANDOM|CLASSIC"));
        }
    }
    else if (strcmp(command, "SEQUENCE") == 0) { 
        if (arg1 != NULL) {
            if (strcmp(arg1, "ON")==0) {
                currentProfile.sequenceMode = true; 
            } else if (strcmp(arg1, "OFF")==0) {
                currentProfile.sequenceMode = false; 
            }
            Serial.print(F("Sequence: ")); 
            Serial.println(currentProfile.sequenceMode ? "ON" : "OFF"); 
        } else {
            Serial.println(F("SEQUENCE: ON|OFF"));
        }
    }
    else if (strcmp(command, "VOLTAGE") == 0) { 
        if (arg1 != NULL) {
            if (strcmp(arg1, "ON")==0) {
                currentProfile.voltageMonitorEnabled = true; 
                sysConfig.monitorVccEnabled = true;
                saveSystemConfig();
            } else if (strcmp(arg1, "OFF")==0) {
                currentProfile.voltageMonitorEnabled = false; 
                sysConfig.monitorVccEnabled = false;
                saveSystemConfig();
            }
            Serial.print(F("Voltage Mon: ")); 
            Serial.println(currentProfile.voltageMonitorEnabled ? "ON" : "OFF"); 
        } else {
            Serial.println(F("VOLTAGE: ON|OFF"));
        }
    }
    else if (strcmp(command, "VM") == 0) {
        if (arg1 != NULL) {
            if (strcmp(arg1, "1")==0 || strcmp(arg1, "ON")==0) {
                currentProfile.voltageMonitorEnabled = true;
                sysConfig.monitorVccEnabled = true;
                saveSystemConfig();
                Serial.println(F("OK"));
            } else if (strcmp(arg1, "0")==0 || strcmp(arg1, "OFF")==0) {
                currentProfile.voltageMonitorEnabled = false;
                sysConfig.monitorVccEnabled = false;
                saveSystemConfig();
                Serial.println(F("OK"));
            } else {
                Serial.println(F("VM: 0|1"));
            }
        } else {
            Serial.println(F("VM: 0|1"));
        }
    }
    else if (strcmp(command, "VL") == 0) {
        if (arg1 != NULL) {
            char *arg2 = strtok(NULL, " ");
            char *arg3 = strtok(NULL, " ");
            char *arg4 = strtok(NULL, " ");
            if (arg2 && arg3 && arg4) {
                sysConfig.limitRed = atof(arg1);
                sysConfig.limitYellow = atof(arg2);
                sysConfig.limitGreen = atof(arg3);
                sysConfig.limitCharge = atof(arg4);
                saveSystemConfig();
                Serial.println(F("OK"));
            } else {
                Serial.println(F("VL: RED YELLOW GREEN CHARGE"));
            }
        } else {
            Serial.println(F("VL: RED YELLOW GREEN CHARGE"));
        }
    }
    else if (strcmp(command, "RR") == 0) {
        if (arg1 != NULL) {
            char *arg2 = strtok(NULL, " ");
            if (arg2) {
                sysConfig.r1 = atof(arg1);
                sysConfig.r2 = atof(arg2);
                saveSystemConfig();
                Serial.println(F("OK"));
            } else {
                Serial.println(F("RR: R1 R2"));
            }
        } else {
            Serial.println(F("RR: R1 R2"));
        }
    }
    else if (strcmp(command, "SCHEME") == 0) {
        if (arg1 != NULL) {
            int8_t newScheme = -1;
            if (strcmp(arg1, "CLASSIC") == 0) newScheme = CLASSIC;
            else if (strcmp(arg1, "BLUE") == 0) newScheme = BLUE;
            else if (strcmp(arg1, "PINK") == 0) newScheme = PINK;
            else if (strcmp(arg1, "GREEN") == 0) newScheme = GREEN;
            else if (strcmp(arg1, "CYBERPUNK") == 0) newScheme = CYBERPUNK;
            else if (strcmp(arg1, "FOREST") == 0) newScheme = FOREST;
            else if (strcmp(arg1, "SUNSET") == 0) newScheme = SUNSET;
            else if (strcmp(arg1, "CUSTOM") == 0) newScheme = CUSTOM;
            else { 
                int schemeNum = atoi(arg1); 
                if (schemeNum >= 0 && schemeNum < SCHEME_COUNT) newScheme = schemeNum; 
            }
            if (newScheme != -1) { 
                currentProfile.colorScheme = newScheme; 
                applyProfile(); 
                Serial.println(F("Scheme updated.")); 
            } else { 
                Serial.println(F("Invalid scheme.")); 
            }
        } else {
            Serial.println(F("SCHEME: 0-7 or name"));
        }
    }
    else if (strcmp(command, "COLOR") == 0) {
        if (arg1 != NULL) {
            char *arg2 = strtok(NULL, " "); 
            char *arg3 = strtok(NULL, " "); 
            char *arg4 = strtok(NULL, " ");
            if (arg2 && arg3 && arg4) {
                uint8_t section = atoi(arg1);
                if (section < SEC_COUNT) {
                    currentProfile.customColors[section] = CRGB(atoi(arg2), atoi(arg3), atoi(arg4));
                    currentProfile.colorScheme = CUSTOM; 
                    applyProfile(); 
                    Serial.println(F("Custom color set."));
                } else { 
                    Serial.println(F("Invalid section.")); 
                }
            } else { 
                Serial.println(F("COLOR: sec(0-12) R G B")); 
            }
        } else {
            Serial.println(F("COLOR: sec(0-12) R G B"));
        }
    }
    else { 
        Serial.println(F("Unknown cmd. HELP for list.")); 
    }
}

void printHelp() {
    Serial.println(F("\nRGB-DPL Nano v5.2"));
    Serial.println(F("CFG/EXIT HELP STATUS SAVE LOAD DEFAULT"));
    Serial.println(F("BRIGHTNESS SPEED CBISPEED SCHEME PERSONALITY"));
    Serial.println(F("SEQUENCE VOLTAGE VM VL RR COLOR"));
    Serial.println(F("LDPLMODE/LLMODE CBIMODE CSLMODE CSLSPEED CSLBRIGHTNESS"));
    Serial.println(F("BARGRAPH TOPBLOCKS"));
}

void printStatus() {
    Serial.println(F("\nSTATUS"));
    Serial.print(F("P ")); Serial.print(currentProfile.name);
    Serial.print(F(" B ")); Serial.print(currentProfile.brightness);
    Serial.print(F(" S ")); Serial.print(currentProfile.animSpeed);
    Serial.print(F(" CBI ")); Serial.println(currentProfile.cbiSpeed);
    Serial.print(F("PER ")); Serial.print(currentProfile.personality);
    Serial.print(F(" SCH ")); Serial.print(currentProfile.colorScheme);
    Serial.print(F(" SEQ ")); Serial.print(currentProfile.sequenceMode ? "1" : "0");
    Serial.print(F(" V ")); Serial.println(currentProfile.voltageMonitorEnabled ? "1" : "0");
    Serial.print(F("LDPL ")); Serial.print(currentProfile.ldplMode);
    Serial.print(F(" CBI ")); Serial.print(currentProfile.cbiMode);
    Serial.print(F(" CSL ")); Serial.print(currentProfile.cslMode);
    Serial.print(F(" CS ")); Serial.print(currentProfile.cslSpeed);
    Serial.print(F(" CB ")); Serial.println(currentProfile.cslBrightness);
    Serial.print(F("R ")); Serial.print(sysConfig.r1, 0);
    Serial.print(F("/")); Serial.print(sysConfig.r2, 0);
    Serial.print(F(" L ")); Serial.print(sysConfig.limitRed, 1);
    Serial.print(F("/")); Serial.print(sysConfig.limitYellow, 1);
    Serial.print(F("/")); Serial.print(sysConfig.limitGreen, 1);
    Serial.print(F("/")); Serial.println(sysConfig.limitCharge, 1);
}

// ============================================================================
// I2C & Animation Functions
// ============================================================================

byte organicRandom(byte mode) {
    uint8_t persParams[4];
    memcpy_P(persParams, PERSONALITY_PARAMS[currentProfile.personality], 4);
    mode = constrain(mode + (persParams[3] - 4), 0, 6);
    switch (mode) {
        case 0: return (random(256) & random(256) & random(256) & random(256));
        case 1: return (random(256) & random(256) & random(256));
        case 2: return (random(256) & random(256));
        case 3: return random(256);
        case 4: return (random(256) | random(256));
        case 5: return (random(256) | random(256) | random(256));
        case 6: return (random(256) | random(256) | random(256) | random(256));
        default: return random(256);
    }
}

void updateTopBlocks() {
    if (currentProfile.topBlocksRandom) {
        byte yellowPattern = organicRandom(4);
        byte greenPattern = organicRandom(4);
        for (int i = 0; i < 9; i++) leds_d[i] = bitRead(yellowPattern, i % 8) ? colorTable[SEC_TOP_YELLOW] : CRGB::Black;
        for (int i = 9; i < 18; i++) leds_d[i] = bitRead(greenPattern, (i-9) % 8) ? colorTable[SEC_TOP_GREEN] : CRGB::Black;
    } else {
        for(int i=0; i<18; i++) leds_d[i] = CRGB::Black;
        int block = random(9); leds_d[block] = colorTable[SEC_TOP_YELLOW];
        block = random(9); leds_d[block + 9] = colorTable[SEC_TOP_GREEN];
    }
}

void updateBlueLEDs() {
    byte pattern = organicRandom(4);
    for (int i = 0; i < 6; i++) { leds_c[i * 4] = bitRead(pattern, i) ? colorTable[SEC_BLUE_COLUMN] : CRGB::Black; }
}

byte updateBar(byte* data, byte maxVal) {
    int value = (int)(*data) + (random(3) - 1);
    *data = constrain(value, 1, maxVal);
    return *data;
}

void updateBargraph() {
    if (currentProfile.bargraphSplit) {
        for (int bar = 0; bar < 2; bar++) updateBar(&bargraphData[bar], 6);
    } else {
        byte height = updateBar(&bargraphData[0], 6);
        bargraphData[1] = height;
    }
    for (int level = 0; level < 6; level++) {
        int baseIndex = (5 - level) * 4 + 1;
        CRGB colorL = (level < bargraphData[0]) ? (level >= 4 ? colorTable[SEC_BARGRAPH_HIGH] : (level >= 2 ? colorTable[SEC_BARGRAPH_MID] : colorTable[SEC_BARGRAPH_LOW])) : CRGB::Black;
        CRGB colorR = (level < bargraphData[1]) ? (level >= 4 ? colorTable[SEC_BARGRAPH_HIGH] : (level >= 2 ? colorTable[SEC_BARGRAPH_MID] : colorTable[SEC_BARGRAPH_LOW])) : CRGB::Black;
        leds_c[baseIndex] = colorL;
        leds_c[baseIndex + 1] = colorR;
        leds_c[baseIndex + 2] = colorL;
    }
}

void updateBottomLEDs() {
    byte pattern = organicRandom(4);
    for (int i = 0; i < 6; i++) leds_a[i] = bitRead(pattern, i) ? colorTable[SEC_BOTTOM_WHITE] : CRGB::Black;
}

void updateRedLEDs() {
    leds_a[6] = random(2) ? colorTable[SEC_RED_LARGE] : CRGB::Black;
    leds_a[7] = random(2) ? colorTable[SEC_RED_LARGE] : CRGB::Black;
}

void updateVuMeter() {
    if (currentProfile.personality == HAPPY) { fill_rainbow(leds_b, NUM_LEDS_B, gHue, 7); }
    else if (currentProfile.personality == SLEEPY) { fill_solid(leds_b, NUM_LEDS_B, CHSV(160, 255, beatsin8(10, 30, 100))); }
    else {
        uint8_t beat = beatsin8(62, 64, 255);
        for (int i = 0; i < NUM_LEDS_B / 2; i++) leds_b[i] = leds_b[NUM_LEDS_B - 1 - i] = CHSV(gHue + (i * 4), 255, beat);
    }
}

void getVCC() {
    int rawValue = analogRead(ANALOG_INPUT);
    float vout = (rawValue * 5.0f) / 1024.0f;
    float vin = vout / (sysConfig.r2 / (sysConfig.r1 + sysConfig.r2));
    if (vin < sysConfig.limitRed) {
        static bool blinkState = false;
        EVERY_N_MILLISECONDS(300) { blinkState = !blinkState; }
        CRGB warn = blinkState ? CRGB::Red : CRGB::Black;
        leds_f[20] = warn;
        leds_f[21] = warn;
        leds_f[22] = warn;
    } else {
        leds_f[20] = (vin >= sysConfig.limitGreen) ? CRGB::Green : CRGB::Black;
        leds_f[21] = (vin < sysConfig.limitGreen && vin >= sysConfig.limitYellow) ? CRGB::Yellow : CRGB::Black;
        leds_f[22] = (vin >= sysConfig.limitRed) ? CRGB::Red : CRGB::Black;
    }
}

void matrixRain(); 

void updateCBILEDs() {
    uint8_t matrixLedCount = currentProfile.voltageMonitorEnabled ? 20 : NUM_LEDS_F;

    switch (currentProfile.cbiMode) {
        case 0:
        case 4:
            for (int r = 0; r < 4; r++) {
                byte p = organicRandom(random(4, 6));
                for (int c = 0; c < 5; c++) leds_f[r * 5 + c] = bitRead(p, c) ? colorTable[SEC_CBI_MATRIX] : CRGB::Black;
            }
            break;
        case 1:
        case 5:
            fill_rainbow(leds_f, matrixLedCount, gHue, 7);
            break;
        case 2:
            fadeToBlackBy(leds_f, matrixLedCount, 10);
            leds_f[random16(matrixLedCount)] += CHSV(gHue + random8(64), 200, 255);
            break;
        case 3:
        case 6:
            matrixRain();
            break;
    }
    
    if (currentProfile.voltageMonitorEnabled) {
        getVCC();
    }
}
void matrixRain() {
    uint8_t count = currentProfile.voltageMonitorEnabled ? 20 : NUM_LEDS_F;
    fadeToBlackBy(leds_f, count, 20);

    // Randomly spawn new raindrop in top row
    if (random8() < 30) {
        leds_f[random(5)] = CRGB::Green;
    }

    // Shift pixels downward (bottom to top to avoid overwriting)
    for (int r = 3; r > 0; r--) {
        for (int c = 0; c < 5; c++) {
            if (leds_f[(r - 1) * 5 + c].g > 0) {
                leds_f[r * 5 + c] = leds_f[(r - 1) * 5 + c];
                leds_f[(r - 1) * 5 + c].fadeToBlackBy(100);
            }
        }
    }
}

CRGB scaleModuleColor(const CRGB& color, uint8_t percent) {
    CRGB scaled = color;
    scaled.nscale8_video(map(percent, 1, 100, 10, 255));
    return scaled;
}

void updateCSL() {
    static uint8_t cslOffset = 0;
    fadeToBlackBy(leds_g, NUM_LEDS_G, 32);

    CRGB cslBase = scaleModuleColor(colorTable[SEC_CSL], currentProfile.cslBrightness);
    uint8_t glow = beatsin8(10 + currentProfile.cslSpeed, 20, 255);

    switch (currentProfile.cslMode) {
        case 0:
            fill_solid(leds_g, NUM_LEDS_G, CRGB::Black);
            break;
        case 1:
            for (int i = 0; i < NUM_LEDS_G; i += 6) leds_g[(i + cslOffset) % NUM_LEDS_G] = cslBase;
            break;
        case 2:
            fill_solid(leds_g, NUM_LEDS_G, cslBase);
            break;
        case 3: {
            CRGB glowColor = cslBase;
            glowColor.nscale8_video(glow);
            fill_solid(leds_g, NUM_LEDS_G, glowColor);
            break;
        }
        case 4:
            for (int i = 0; i < NUM_LEDS_G; i += 3) leds_g[(i + cslOffset) % NUM_LEDS_G] = cslBase;
            break;
        case 5:
            leds_g[cslOffset % NUM_LEDS_G] = cslBase;
            leds_g[(NUM_LEDS_G - 1 - cslOffset) % NUM_LEDS_G] = cslBase;
            break;
        case 6:
            for (int i = 0; i < NUM_LEDS_G; i++) if (((i + cslOffset) % 8) < 3) leds_g[i] = cslBase;
            break;
        case 7: {
            int pos = beatsin16(10 + currentProfile.cslSpeed * 3, 0, NUM_LEDS_G - 1);
            leds_g[pos] = cslBase;
            if (pos > 0) leds_g[pos - 1] = scaleModuleColor(cslBase, currentProfile.cslBrightness / 2 + 1);
            if (pos < NUM_LEDS_G - 1) leds_g[pos + 1] = scaleModuleColor(cslBase, currentProfile.cslBrightness / 2 + 1);
            break;
        }
    }

    cslOffset = (cslOffset + 1) % NUM_LEDS_G;
}

void ldplBreathing() {
    uint8_t beat = beatsin8(5, 30, 255);
    CHSV color = rgb2hsv_approximate(colorTable[SEC_LDPL]);
    fill_solid(leds_e, NUM_LEDS_E, CHSV(color.h, color.s, beat));
}

void ldplPersonality() {
    switch (currentProfile.personality) {
        case HAPPY: fadeToBlackBy(leds_e, NUM_LEDS_E, 20); for(int i=0; i<8; i++) leds_e[beatsin16(i+7, 0, NUM_LEDS_E-1)] |= CHSV(gHue+i*32, 200, 255); break;
        case GRUMPY: fill_solid(leds_e, NUM_LEDS_E, CHSV(0, 255, beatsin8(30, 10, 100))); break;
        case EXCITED: fill_rainbow(leds_e, NUM_LEDS_E, gHue * 3, 10); break;
        case SLEEPY: fill_solid(leds_e, NUM_LEDS_E, CHSV(160, 255, beatsin8(5, 10, 60))); break;
        default: ldplBreathing();
    }
}

void updateLDPL() {
    switch (currentProfile.ldplMode) {
        case 0: ldplBreathing(); break;
        case 1: fill_rainbow(leds_e, NUM_LEDS_E, gHue, 7); break;
        case 2: fill_solid(leds_e, NUM_LEDS_E, CRGB::Black); break;
        case 3: ldplPersonality(); break;
    }
}

