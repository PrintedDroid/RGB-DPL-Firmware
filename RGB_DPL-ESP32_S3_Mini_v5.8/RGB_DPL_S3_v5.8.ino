/*
 * ============================================================================
 * Enhanced RGB-DPL/CBI/LDPL Controller v5.8 (ESP32-S3 Final)
 * For Printed-Droid R2-D2 Replica Display Systems
 * ============================================================================
 *
 * DESCRIPTION:
 * This sketch controls multiple LED display panels (DPL, CBI, Large Logic)
 * for an R2-D2 replica, using only WS2811/WS2812B addressable RGB LEDs.
 * It is a complete rewrite of older MAX7219-based sketches, offering far
 * more flexibility, color control, and dynamic animations on the ESP32-S3 platform.
 *
 *
 * KEY FEATURES:
 * - [cite_start]Fully addressable RGB control for all panels. [cite: 58]
 * - [cite_start]Emulation of original "organic" and "ESB" animation styles. [cite: 59]
 * - [cite_start]User profiles, color schemes, and "personality" modes saved to EEPROM. [cite: 60]
 * - [cite_start]Extensive configuration and control via Serial Monitor. [cite: 61]
 * - [cite_start]I2C slave support for integration with other systems (e.g., Marcduino). [cite: 62]
 * - [cite_start]Door sensors to activate/deactivate displays automatically. [cite: 62]
 * - [cite_start]Advanced animation modes for DPL panels (Bargraph & Top Blocks). [cite: 75]
 * - [cite_start]"Sequence Mode" to automatically cycle through animations. [cite: 76]
 * - [cite_start]A robust, memory-efficient, and case-insensitive serial command parser. [cite: 91, 92]
 * - [cite_start]Universal 12V/24V voltage monitor. [cite: 3534, 3537]
 * - [cite_start]An enhanced, user-friendly, colorful help menu. [cite: 3540, 3544]
 *
 *
 * VERSION HISTORY (Summary):
 * - v2.0: Initial RGB version for Arduino Nano, replacing MAX7219 controllers. [cite_start]Included simple serial commands (L0-L2, C0-C3). [cite: 54, 55]
 * - [cite_start]v3.1: Major rewrite introducing user profiles, color schemes, and personalities saved to EEPROM. [cite: 57, 60]
 * - [cite_start]v4.0: Added advanced animation modes (Bargraph, Top Blocks, Smiley), expanded color schemes, and a "Sequence Mode". [cite: 75, 76]
 * - [cite_start]v5.0: Complete rewrite of the serial parser to use robust C-style string handling, fixing stability issues on Arduino Nano. [cite: 91, 95]
 * - [cite_start]v5.2: Ported to the ESP32-S3 platform with an adapted pinout and a universal 12V/24V voltage monitor. [cite: 3534, 3535, 3537]
 * - [cite_start]v5.3: Enhanced the serial help menu with user-friendly ANSI color codes. [cite: 3540, 3544]
 * - [cite_start]v5.6: Expanded the TopBlocks animation to three modes (RANDOM, HORIZONTAL, PAIRED). [cite: 3546]
 * - v5.7: Added a fifth TopBlocks animation mode (SINGLE_PIXEL).
 * VERSION 5.8 IMPROVEMENTS:
 * - Fixed PROGMEM string access for ESP32 compatibility
 * - Added I2C critical sections for thread-safe ISR operations
 * - Improved buffer overflow protection
 * - Overflow-safe millis() timeout handling
 * - Added watchdog timer support (auto-detects Core version)
 * - Converted magic numbers to named constants
 * - Enhanced error handling throughout
 * - Optimized string processing in command parser
 * - Added system health monitoring
 * - Improved memory management
 * - Fixed ANSI escape codes
 * - Corrected pattern array sizes
 *
 * ============================================================================
 * HARDWARE & PINOUT (ESP32-S3 Mini)
 * ============================================================================
 *
 * - Strip A (8 LEDs, Pin D4):  Bottom white lights (6) + Large red lights (2)
 * - Strip B (28 LEDs, Pin D5): VU-Meter display
 * - Strip C (24 LEDs, Pin D6): Right panel - Blue column (6) + Bargraph (18)
 * - Strip D (18 LEDs, Pin D7): Top panel - Yellow blocks (9) + Green blocks (9)
 * - Strip E (43 LEDs, Pin D8): Large Logic Display Panel
 * - Strip F (23 LEDs, Pin D9): CBI - Matrix (20) + Status lights (3)
 * - Sensors: Left Door on Pin 10, Right Door on Pin 11, Voltage Monitor on Pin 12
 *
 *
 * ============================================================================
 * IMPORTANT: ESP32-S3 Arduino IDE Configuration
 * ============================================================================
 *
 * Before uploading this sketch to an ESP32-S3, configure the Arduino IDE as follows:
 *
 * Tools Menu Settings:
 * - Board: "Lolon S3 Mini" or "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled"         <-- CRITICAL! [cite_start]Must be enabled for Serial. [cite: 3545]
 * - USB Mode: "Hardware CDC and JTAG"
 * - Upload Mode: "UART0 / Hardware CDC"
 * - Flash Size: "4MB" or "8MB"         (depending on your board)
 * - Partition Scheme: "Default 4MB with spiffs"
 *
 * Upload Procedure:
 * 1. Hold BOOT button while connecting USB.
 * 2. Click Upload in Arduino IDE.
 * 3. Release BOOT button when upload starts.
 * 4. Press RESET button after upload completes.
 *
 * Serial Monitor:
 * - Set baud rate to 9600.
 * - If you only see "ESP-ROM:esp32s3-20210327", check the USB CDC settings above.
 *
 *
 * ============================================================================
 * SERIAL COMMANDS (9600 baud, commands must end with a newline '\n')
 * ============================================================================
 *
 * --- System & Profiles ---
 * [cite_start]CONFIG          - Enter configuration mode. [cite: 66, 79]
 * [cite_start]EXIT            - Exit configuration mode. [cite: 66, 79]
 * [cite_start]HELP            - Show the help menu. [cite: 67, 80]
 * [cite_start]STATUS          - Show current settings. [cite: 81]
 * [cite_start]INFO            - Display system status and memory information. [cite: 72]
 * [cite_start]SAVE <1-5>        - Save current settings to a profile. [cite: 67, 82]
 * [cite_start]LOAD <1-5>        - Load a profile from EEPROM. [cite: 67, 83]
 * [cite_start]DEFAULT <1-5>     - Set the default startup profile (0 to disable). [cite: 68, 84]
 * [cite_start]TEST            - Run a full diagnostic test on LEDs and sensors. [cite: 72]
 *
 * --- Global Settings ---
 * [cite_start]BRIGHTNESS <1-100>- Set master brightness. [cite: 71, 85]
 * [cite_start]SPEED <1-10>      - Set base DPL animation speed. [cite: 71, 86]
 * CBISPEED <1-10>   - Set CBI animation speed.
 * [cite_start]SCHEME <name|num> - Set a color scheme (e.g., CLASSIC, BLUE, 0, 1...). [cite: 68, 87]
 * [cite_start]PERSONALITY <0-4> - Set a personality (0:Normal, 1:Happy...). [cite: 69, 87]
 * [cite_start]SEQUENCE <ON|OFF> - Enable/disable automatic animation cycling. [cite: 88]
 * VOLTAGE <ON|OFF>  - Toggle the CBI voltage monitor display.
 * [cite_start]COLOR <s r g b>   - Set a custom color for a specific section 's'. [cite: 70]
 *
 * --- Panel Specific Modes & Quick Keys ---
 * [cite_start]LLMODE <0-3>      - Set Large Logic mode. [cite: 89] (Also L0, L1, L2 for legacy support) [cite_start][cite: 54, 55, 64]
 * [cite_start]CBIMODE <0-6>     - Set CBI mode. [cite: 89] (Also C0, C1.. for legacy support) [cite_start][cite: 55, 64]
 * [cite_start]BARGRAPH <SPLIT|CLASSIC> - Set bargraph animation style. [cite: 89]
 * TOPBLOCKS <0-4>   - Set top block animation style (0:CLASSIC, 1:HORIZONTAL...).
 * [cite_start]P<0-4>          - Quick-change personality. [cite: 64]
 * [cite_start]S<0-4>          - Quick-change color scheme. [cite: 64]
 *
 */



// ============================================================================
// Libraries
// ============================================================================
#include <FastLED.h>
#include <EEPROM.h>
#include <Wire.h>
#include <esp_task_wdt.h>

// ============================================================================
// System Configuration Constants
// ============================================================================
#define VERSION "5.8.1-ESP32"
#define SYSTEM_VOLTAGE 24

// Watchdog timer (10 seconds)
#define WDT_TIMEOUT 10

// Serial configuration
#define SERIAL_BAUD 9600
#define SERIAL_BUFFER_SIZE 64
#define SERIAL_TIMEOUT_MS 500

// Animation timing constants
#define TOPBLOCK_BASE_INTERVAL 200
#define BLUE_BASE_INTERVAL 500
#define BARGRAPH_BASE_INTERVAL 200
#define BOTTOM_BASE_INTERVAL 200
#define RED_BASE_INTERVAL 500
#define CBI_BASE_INTERVAL 100
#define CBI_ESB_INTERVAL 800
#define LARGLOGIC_BASE_INTERVAL 15
#define SEQUENCE_INTERVAL 15000

// CBI special patterns
#define SPECIAL_PATTERN_INTERVAL 200

// Startup sequence timing
#define STARTUP_LED_DELAY 5
#define STARTUP_FLASH_DELAY 50

// ============================================================================
// ANSI Color Codes (Fixed escape sequences)
// ============================================================================
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_STYLE_BOLD    "\x1b[1m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// ============================================================================
// Pin Definitions (Safe pins for ESP32-S3 Mini)
// ============================================================================
#define LEFT_DOOR_PIN  10
#define RIGHT_DOOR_PIN 11
#define ANALOG_INPUT   12

// LED Data Pins
#define DATA_PIN_A 4
#define DATA_PIN_B 5
#define DATA_PIN_C 6
#define DATA_PIN_D 7
#define DATA_PIN_E 8
#define DATA_PIN_F 9

// LED Counts per strip
#define NUM_LEDS_A 8
#define NUM_LEDS_B 28
#define NUM_LEDS_C 24
#define NUM_LEDS_D 18
#define NUM_LEDS_E 43
#define NUM_LEDS_F 23
#define TOTAL_LEDS (NUM_LEDS_A + NUM_LEDS_B + NUM_LEDS_C + NUM_LEDS_D + NUM_LEDS_E + NUM_LEDS_F)

// CBI Matrix configuration
#define CBI_MATRIX_LEDS 20
#define CBI_STATUS_LEDS 3

// ============================================================================
// Voltage Monitor Configuration (Universal 12V-24V)
// ============================================================================
float GREEN_VCC, YELLOW_VCC, RED_VCC;
#define VOLTAGE_R1 220000.0f
#define VOLTAGE_R2 33000.0f
#define ADC_MAX_VALUE 4095.0f
#define ADC_REFERENCE_VOLTAGE 3.3f

// ============================================================================
// Configuration Enumerations
// ============================================================================
enum ColorScheme { 
    CLASSIC, BLUE, PINK, GREEN, CYBERPUNK, FOREST, SUNSET, CUSTOM, 
    SCHEME_COUNT 
};

enum Personality { 
    NORMAL, HAPPY, GRUMPY, EXCITED, SLEEPY, 
    PERS_COUNT 
};

enum LEDSection { 
    SEC_TOP_YELLOW, SEC_TOP_GREEN, SEC_BLUE_COLUMN, 
    SEC_BARGRAPH_LOW, SEC_BARGRAPH_MID, SEC_BARGRAPH_HIGH, 
    SEC_BOTTOM_WHITE, SEC_RED_LARGE, SEC_VU_METER, 
    SEC_CBI_MATRIX, SEC_CBI_STATUS, SEC_LARGE_LOGIC, 
    SEC_COUNT 
};

enum TopBlocksMode {
    TOPBLOCKS_CLASSIC = 0,
    TOPBLOCKS_HORIZONTAL = 1,
    TOPBLOCKS_PAIRED = 2,
    TOPBLOCKS_CHAOS = 3,
    TOPBLOCKS_SINGLE = 4,
    TOPBLOCKS_MODE_COUNT = 5
};

// ============================================================================
// Configuration Structures
// ============================================================================
struct UserProfile {
    char name[16];
    uint8_t brightness;
    uint8_t animSpeed;
    uint8_t cbiSpeed;
    uint8_t colorScheme;
    uint8_t personality;
    uint8_t llMode;
    uint8_t cbiMode;
    bool bargraphSplit;
    bool sequenceMode;
    bool voltageMonitorEnabled;
    uint8_t topBlocksMode;
    CRGB customColors[SEC_COUNT];
    uint16_t checksum;
};

struct SystemConfig {
    uint8_t defaultProfile;
    uint16_t bootCount;
    uint16_t checksum;
};

// ============================================================================
// PROGMEM Data (Patterns & Lookup Tables) - CORRECTED SIZES
// ============================================================================
const int8_t SMILEY_PATTERN[20] PROGMEM = { 
    0, 1, 1, 1, 0, 
    1, 0, 0, 0, 1, 
    1, 1, 0, 1, 1, 
    0, 0, 1, 0, 0
};

const int8_t HEART_PATTERN[20] PROGMEM = { 
    0, 1, 0, 1, 0, 
    1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    0, 1, 1, 1, 0
};

const int8_t ESB_PATTERNS[28][20] PROGMEM = {
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,0,0},
    {1,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0, 1,1,0,0,0},
    {0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 1,1,0,0,0},
    {0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 0,1,0,0,0},
    {0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,0,0},
    {0,0,0,1,0, 0,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0},
    {0,0,0,1,0, 0,0,1,1,0, 0,0,0,0,0, 0,0,1,1,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0, 0,0,0,1,0},
    {1,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0, 1,0,1,1,0},
    {1,0,0,0,0, 0,1,0,1,0, 0,0,0,1,0, 1,0,1,0,0},
    {1,0,0,0,0, 0,1,0,1,0, 0,0,0,1,0, 1,0,0,0,0},
    {1,0,0,0,0, 0,0,1,0,0, 0,0,0,1,0, 1,1,0,0,0},
    {1,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0, 1,1,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0, 0,0,0,0,0},
    {1,0,0,0,0, 0,0,0,1,0, 0,0,0,1,0, 0,1,1,0,0},
    {1,0,0,0,0, 1,0,0,0,0, 0,1,0,1,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,1,0,0, 0,1,0,1,1, 0,1,0,0,0},
    {0,0,0,0,0, 0,0,0,1,1, 0,1,0,1,1, 0,1,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,1,0,1,1, 0,1,0,0,0},
    {1,0,0,0,1, 0,0,1,0,0, 0,1,0,0,1, 0,1,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,1, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,1,0,0,1, 0,1,0,0,0},
    {0,0,0,0,0, 0,0,0,1,0, 0,1,0,0,1, 0,1,0,0,0}
};

// Personality parameters: [speed_mult*10, cbi_mult*10, hue_shift, organic_offset]
const int8_t PERSONALITY_PARAMS[][4] PROGMEM = { 
    {10, 10, 0, 4},    // NORMAL
    {15, 12, 30, 5},   // HAPPY
    {5, 8, -30, 3},    // GRUMPY
    {20, 15, 0, 6},    // EXCITED
    {3, 5, -60, 2}     // SLEEPY
};

// Color scheme names for help menu
const char scheme_name_0[] PROGMEM = "CLASSIC";
const char scheme_name_1[] PROGMEM = "BLUE";
const char scheme_name_2[] PROGMEM = "PINK";
const char scheme_name_3[] PROGMEM = "GREEN";
const char scheme_name_4[] PROGMEM = "CYBERPUNK";
const char scheme_name_5[] PROGMEM = "FOREST";
const char scheme_name_6[] PROGMEM = "SUNSET";
const char scheme_name_7[] PROGMEM = "CUSTOM";

const char* const SCHEME_NAMES[] PROGMEM = {
    scheme_name_0, scheme_name_1, scheme_name_2, scheme_name_3,
    scheme_name_4, scheme_name_5, scheme_name_6, scheme_name_7
};

// ============================================================================
// Global Variables & Objects
// ============================================================================
CRGB leds_a[NUM_LEDS_A];
CRGB leds_b[NUM_LEDS_B];
CRGB leds_c[NUM_LEDS_C];
CRGB leds_d[NUM_LEDS_D];
CRGB leds_e[NUM_LEDS_E];
CRGB leds_f[NUM_LEDS_F];

UserProfile currentProfile;
SystemConfig sysConfig;
bool configMode = false;
CRGB colorTable[SEC_COUNT];

// Animation state variables
uint8_t gHue = 0;
uint8_t lHue = 0;
uint8_t esbCounter = 0;
byte bargraphData[2] = {3, 3};

// Serial command buffer
char serialBuffer[SERIAL_BUFFER_SIZE];
byte serialBufferPos = 0;
bool isWaitingForCommand = false;
uint32_t lastSerialActivityTime = 0;

// I2C with critical section protection (CORRECTED for ISR use)
portMUX_TYPE i2cMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint8_t i2cCommand = 0;
volatile bool i2cCommandReady = false;

// ============================================================================
// Overflow-Safe Timer Class
// ============================================================================
class Timer {
private:
    uint32_t lastTime;
    uint32_t interval;
    
public:
    Timer(uint32_t ms) : lastTime(0), interval(ms) {}
    
    void setInterval(uint32_t ms) {
        interval = ms;
    }
    
    // Overflow-safe timer check
    bool ready() {
        uint32_t currentTime = millis();
        uint32_t elapsed = currentTime - lastTime;
        
        if (elapsed >= interval) {
            lastTime = currentTime;
            return true;
        }
        return false;
    }
    
    void reset() {
        lastTime = millis();
    }
};

// Animation timers
Timer topBlockTimer(TOPBLOCK_BASE_INTERVAL);
Timer blueTimer(BLUE_BASE_INTERVAL);
Timer bargraphTimer(BARGRAPH_BASE_INTERVAL);
Timer bottomTimer(BOTTOM_BASE_INTERVAL);
Timer redTimer(RED_BASE_INTERVAL);
Timer cbiTimer(CBI_BASE_INTERVAL);
Timer largeLogicTimer(LARGLOGIC_BASE_INTERVAL);
Timer sequenceTimer(SEQUENCE_INTERVAL);

// ============================================================================
// EEPROM Management
// ============================================================================
#define EEPROM_MAGIC 0xDEAD
#define EEPROM_SYS_ADDR 0
#define EEPROM_PROFILE_ADDR(x) (sizeof(SystemConfig) + ((x) * sizeof(UserProfile)))
const int EEPROM_SIZE = sizeof(SystemConfig) + (5 * sizeof(UserProfile));

uint16_t calculateChecksum(void* data, size_t size) {
    uint16_t sum = 0;
    uint8_t* bytes = (uint8_t*)data;
    for (size_t i = 0; i < size - 2; i++) {
        sum += bytes[i];
    }
    return sum ^ EEPROM_MAGIC;
}

bool saveSystemConfig() {
    sysConfig.checksum = calculateChecksum(&sysConfig, sizeof(SystemConfig));
    EEPROM.put(EEPROM_SYS_ADDR, sysConfig);
    return EEPROM.commit();
}

bool loadSystemConfig() {
    EEPROM.get(EEPROM_SYS_ADDR, sysConfig);
    
    if (sysConfig.checksum != calculateChecksum(&sysConfig, sizeof(SystemConfig))) {
        Serial.println(F("System config invalid, initializing defaults"));
        sysConfig.defaultProfile = 0;
        sysConfig.bootCount = 0;
        return saveSystemConfig();
    }
    
    sysConfig.bootCount++;
    return saveSystemConfig();
}

// Forward declarations
void applyProfile();
void updateColorTable();

bool loadProfile(uint8_t profileNum) {
    if (profileNum >= 5) {
        Serial.println(F("ERROR: Invalid profile number"));
        return false;
    }
    
    UserProfile temp;
    EEPROM.get(EEPROM_PROFILE_ADDR(profileNum), temp);
    
    if (temp.checksum == calculateChecksum(&temp, sizeof(UserProfile))) {
        currentProfile = temp;
        applyProfile();
        Serial.print(F("Loaded profile "));
        Serial.println(profileNum + 1);
        return true;
    } else {
        Serial.print(F("Profile "));
        Serial.print(profileNum + 1);
        Serial.println(F(" is empty or corrupted"));
        return false;
    }
}

bool saveProfile(uint8_t profileNum) {
    if (profileNum >= 5) {
        Serial.println(F("ERROR: Invalid profile number"));
        return false;
    }
    
    currentProfile.checksum = calculateChecksum(&currentProfile, sizeof(UserProfile));
    EEPROM.put(EEPROM_PROFILE_ADDR(profileNum), currentProfile);
    
    if (EEPROM.commit()) {
        Serial.print(F("Saved to profile "));
        Serial.println(profileNum + 1);
        return true;
    } else {
        Serial.println(F("ERROR: EEPROM commit failed"));
        return false;
    }
}

void initDefaultProfile() {
    strcpy(currentProfile.name, "Default");
    currentProfile.brightness = 25;
    currentProfile.animSpeed = 5;
    currentProfile.cbiSpeed = 3;
    currentProfile.colorScheme = CLASSIC;
    currentProfile.personality = NORMAL;
    currentProfile.llMode = 0;
    currentProfile.cbiMode = 0;
    currentProfile.bargraphSplit = false;
    currentProfile.topBlocksMode = TOPBLOCKS_CLASSIC;
    currentProfile.sequenceMode = false;
    currentProfile.voltageMonitorEnabled = false;
    applyProfile();
}

// ============================================================================
// Color Management
// ============================================================================
void updateColorTable() {
    // Apply base color scheme
    switch (currentProfile.colorScheme) {
        case CLASSIC:
            colorTable[SEC_TOP_YELLOW] = CRGB::Yellow;
            colorTable[SEC_TOP_GREEN] = CRGB::Green;
            colorTable[SEC_BLUE_COLUMN] = CRGB::Blue;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::Green;
            colorTable[SEC_BARGRAPH_MID] = CRGB::Yellow;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::Red;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::White;
            colorTable[SEC_RED_LARGE] = CRGB::Red;
            colorTable[SEC_CBI_MATRIX] = CRGB::Red;
            break;
            
        case BLUE:
            colorTable[SEC_TOP_YELLOW] = CRGB::Cyan;
            colorTable[SEC_TOP_GREEN] = CRGB::Blue;
            colorTable[SEC_BLUE_COLUMN] = CRGB::DeepSkyBlue;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::Blue;
            colorTable[SEC_BARGRAPH_MID] = CRGB::Cyan;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::White;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::LightBlue;
            colorTable[SEC_RED_LARGE] = CRGB::Blue;
            colorTable[SEC_CBI_MATRIX] = CRGB::Blue;
            break;
            
        case PINK:
            colorTable[SEC_TOP_YELLOW] = CRGB::HotPink;
            colorTable[SEC_TOP_GREEN] = CRGB::Magenta;
            colorTable[SEC_BLUE_COLUMN] = CRGB::DeepPink;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::HotPink;
            colorTable[SEC_BARGRAPH_MID] = CRGB::DeepPink;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::Magenta;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::HotPink;
            colorTable[SEC_RED_LARGE] = CRGB::DeepPink;
            colorTable[SEC_CBI_MATRIX] = CRGB::Magenta;
            break;
            
        case GREEN:
            colorTable[SEC_TOP_YELLOW] = CRGB::YellowGreen;
            colorTable[SEC_TOP_GREEN] = CRGB::Green;
            colorTable[SEC_BLUE_COLUMN] = CRGB::SeaGreen;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::DarkGreen;
            colorTable[SEC_BARGRAPH_MID] = CRGB::Green;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::Lime;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::PaleGreen;
            colorTable[SEC_RED_LARGE] = CRGB::ForestGreen;
            colorTable[SEC_CBI_MATRIX] = CRGB::Green;
            break;
            
        case CYBERPUNK:
            colorTable[SEC_TOP_YELLOW] = CRGB::Magenta;
            colorTable[SEC_TOP_GREEN] = CRGB::Purple;
            colorTable[SEC_BLUE_COLUMN] = CRGB::Cyan;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::DeepPink;
            colorTable[SEC_BARGRAPH_MID] = CRGB::Cyan;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::Yellow;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::LightCyan;
            colorTable[SEC_RED_LARGE] = CRGB::Magenta;
            colorTable[SEC_CBI_MATRIX] = CRGB::Cyan;
            break;
            
        case FOREST:
            colorTable[SEC_TOP_YELLOW] = CRGB::Orange;
            colorTable[SEC_TOP_GREEN] = CRGB::DarkGreen;
            colorTable[SEC_BLUE_COLUMN] = CRGB::SeaGreen;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::Green;
            colorTable[SEC_BARGRAPH_MID] = CRGB::Brown;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::Orange;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::LightGoldenrodYellow;
            colorTable[SEC_RED_LARGE] = CRGB::DarkOrange;
            colorTable[SEC_CBI_MATRIX] = CRGB::DarkGreen;
            break;
            
        case SUNSET:
            colorTable[SEC_TOP_YELLOW] = CRGB::OrangeRed;
            colorTable[SEC_TOP_GREEN] = CRGB::DarkViolet;
            colorTable[SEC_BLUE_COLUMN] = CRGB::Orange;
            colorTable[SEC_BARGRAPH_LOW] = CRGB::Orange;
            colorTable[SEC_BARGRAPH_MID] = CRGB::Red;
            colorTable[SEC_BARGRAPH_HIGH] = CRGB::DarkViolet;
            colorTable[SEC_BOTTOM_WHITE] = CRGB::Gold;
            colorTable[SEC_RED_LARGE] = CRGB::OrangeRed;
            colorTable[SEC_CBI_MATRIX] = CRGB::DarkViolet;
            break;
            
        case CUSTOM:
            memcpy(colorTable, currentProfile.customColors, sizeof(colorTable));
            break;
    }
    
    // Apply personality hue shift
    int8_t persParams[4];
    memcpy_P(persParams, PERSONALITY_PARAMS[currentProfile.personality], sizeof(persParams));
    int8_t hueShift = persParams[2];
    
    if (hueShift != 0) {
        for (int i = 0; i < SEC_COUNT; i++) {
            CHSV hsv = rgb2hsv_approximate(colorTable[i]);
            hsv.h = (uint8_t)((int16_t)hsv.h + hueShift);
            colorTable[i] = hsv;
        }
    }
}

void applyProfile() {
    // Only update brightness if it changed
    static uint8_t lastBrightness = 255;
    if (currentProfile.brightness != lastBrightness) {
        FastLED.setBrightness(map(currentProfile.brightness, 1, 100, 10, 255));
        lastBrightness = currentProfile.brightness;
    }
    updateColorTable();
}

// ============================================================================
// I2C Handler (CORRECTED - Using Critical Sections for ISR)
// ============================================================================
void i2cReceiveEvent(int bytes) {
    if (Wire.available()) {
        portENTER_CRITICAL_ISR(&i2cMux);
        i2cCommand = Wire.read();
        i2cCommandReady = true;
        portEXIT_CRITICAL_ISR(&i2cMux);
    }
}

void processI2CCommand() {
    // The flag was already reset in a thread-safe way in loop()
    // Future: Add I2C command handling here using the value of 'i2cCommand'
}

// ============================================================================
// Help & Status Display
// ============================================================================
void printHelp() {
    Serial.println(F(""));
    Serial.println(ANSI_STYLE_BOLD ANSI_COLOR_CYAN "=== R2-D2 Display Controller v5.8 ===" ANSI_COLOR_RESET);
    Serial.println(F("Syntax: COMMAND <parameter>"));
    
    Serial.println(ANSI_STYLE_BOLD ANSI_COLOR_YELLOW "\n== SYSTEM ==" ANSI_COLOR_RESET);
    Serial.println(F("  CONFIG              - Enter configuration mode"));
    Serial.println(F("  EXIT                - Exit configuration mode"));
    Serial.println(F("  HELP                - Show this help"));
    Serial.println(F("  STATUS              - Display current settings"));
    Serial.println(F("  INFO                - System information & diagnostics"));
    Serial.println(F("  SAVE <1-5>          - Save current settings to profile"));
    Serial.println(F("  LOAD <1-5>          - Load settings from profile"));
    Serial.println(F("  DEFAULT <0-5>       - Set boot profile (0=disabled)"));
    
    Serial.println(ANSI_STYLE_BOLD ANSI_COLOR_YELLOW "\n== GLOBAL SETTINGS ==" ANSI_COLOR_RESET);
    Serial.println(F("  BRIGHTNESS <1-100>  - Master brightness"));
    Serial.println(F("  SPEED <1-10>        - DPL animation speed"));
    Serial.println(F("  CBISPEED <1-10>     - CBI animation speed"));
    Serial.println(F("  SEQUENCE <ON|OFF>   - Auto-cycle animations"));
    Serial.println(F("  VOLTAGE <ON|OFF>    - CBI voltage monitor"));
    Serial.println(F("  PERSONALITY <0-4>   - 0:Normal 1:Happy 2:Grumpy 3:Excited 4:Sleepy"));
    Serial.println(F("  SCHEME <name|num>   - Color scheme (see list below)"));
    Serial.println(F("  COLOR <s r g b>     - Custom section color"));
    
    Serial.println(ANSI_STYLE_BOLD ANSI_COLOR_YELLOW "\n-- Color Schemes --" ANSI_COLOR_RESET);
    for (int i = 0; i < SCHEME_COUNT; i++) {
        const char* namePtr = (const char*)pgm_read_ptr(&SCHEME_NAMES[i]);
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(namePtr);  // Direct print from PROGMEM - more efficient
    }
    
    Serial.println(ANSI_STYLE_BOLD ANSI_COLOR_YELLOW "\n== PANEL MODES ==" ANSI_COLOR_RESET);
    Serial.println(F("  LLMODE <0-3>        - Large Logic (0:Breathe 1:Rainbow 2:Off 3:Personality)"));
    Serial.println(F("  CBIMODE <0-6>       - CBI Display (0:Organic 1:ESB 2:Rainbow...)"));
    Serial.println(F("  BARGRAPH <SPLIT|CLASSIC> - Bargraph style"));
    Serial.println(F("  TOPBLOCKS <0-4>     - 0:Classic 1:Horizontal 2:Paired 3:Chaos 4:Single"));
    Serial.println(F("============================================"));
}

void printStatus() {
    Serial.println(F("\n=== Current Status ==="));
    Serial.print(F("Profile: ")); Serial.println(currentProfile.name);
    Serial.print(F("Brightness: ")); Serial.print(currentProfile.brightness);
    Serial.print(F("% | Speed: ")); Serial.print(currentProfile.animSpeed);
    Serial.print(F(" | CBI Speed: ")); Serial.println(currentProfile.cbiSpeed);
    
    Serial.print(F("Personality: "));
    switch(currentProfile.personality) {
        case NORMAL: Serial.print(F("Normal")); break;
        case HAPPY: Serial.print(F("Happy")); break;
        case GRUMPY: Serial.print(F("Grumpy")); break;
        case EXCITED: Serial.print(F("Excited")); break;
        case SLEEPY: Serial.print(F("Sleepy")); break;
    }
    Serial.print(F(" | Scheme: "));
    Serial.println(currentProfile.colorScheme);
    
    Serial.print(F("Sequence: "));
    Serial.print(currentProfile.sequenceMode ? "ON" : "OFF");
    Serial.print(F(" | Voltage: "));
    Serial.println(currentProfile.voltageMonitorEnabled ? "ON" : "OFF");
    
    Serial.print(F("LL Mode: ")); Serial.print(currentProfile.llMode);
    Serial.print(F(" | CBI Mode: ")); Serial.print(currentProfile.cbiMode);
    Serial.print(F(" | Bargraph: "));
    Serial.println(currentProfile.bargraphSplit ? "Split" : "Classic");
    
    Serial.print(F("Top Blocks: "));
    Serial.println(currentProfile.topBlocksMode);
    Serial.println(F("====================="));
}

// ============================================================================
// Serial Command Parser
// ============================================================================
void processSerialCommand() {
    // Work directly with serialBuffer to avoid extra copy
    char* command = strtok(serialBuffer, " ");
    if (command == NULL) return;
    
    // Convert command to uppercase in-place
    for (char* p = command; *p; p++) {
        *p = toupper(*p);
    }
    
    // Get first argument and convert to uppercase
    char* arg1 = strtok(NULL, " ");
    if (arg1 != NULL) {
        for (char* p = arg1; *p; p++) {
            *p = toupper(*p);
        }
    }
    
    // === System Commands ===
    if (strcmp(command, "CONFIG") == 0) {
        configMode = true;
        Serial.println(F("\n>>> CONFIG MODE ENABLED <<<"));
        printHelp();
        return;
    }
    
    if (strcmp(command, "EXIT") == 0) {
        configMode = false;
        fill_solid(leds_e, 3, CRGB::Black);
        Serial.println(F("<<< Config Mode Exited >>>"));
        return;
    }
    
    if (strcmp(command, "HELP") == 0) {
        printHelp();
        return;
    }
    
    if (strcmp(command, "STATUS") == 0) {
        printStatus();
        return;
    }
    
    if (strcmp(command, "INFO") == 0) {
        Serial.println(F("\n=== System Information ==="));
        Serial.print(F("Version: ")); Serial.println(VERSION);
        Serial.print(F("Free Heap: ")); Serial.print(ESP.getFreeHeap()); Serial.println(F(" bytes"));
        Serial.print(F("Total LEDs: ")); Serial.println(TOTAL_LEDS);
        Serial.print(F("Boot Count: ")); Serial.println(sysConfig.bootCount);
        return;
    }
    
    // === Profile Commands ===
    if (strcmp(command, "SAVE") == 0) {
        if (arg1 != NULL) {
            int profileNum = atoi(arg1) - 1;
            saveProfile(profileNum);
        } else {
            Serial.println(F("ERROR: SAVE requires profile number (1-5)"));
        }
        return;
    }
    
    if (strcmp(command, "LOAD") == 0) {
        if (arg1 != NULL) {
            int profileNum = atoi(arg1) - 1;
            loadProfile(profileNum);
        } else {
            Serial.println(F("ERROR: LOAD requires profile number (1-5)"));
        }
        return;
    }
    
    if (strcmp(command, "DEFAULT") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 0 && val <= 5) {
                sysConfig.defaultProfile = val;
                if (saveSystemConfig()) {
                    Serial.print(F("Default profile set to: "));
                    Serial.println(val);
                } else {
                    Serial.println(F("ERROR: Failed to save config"));
                }
            } else {
                Serial.println(F("ERROR: DEFAULT must be 0-5 (0=off)"));
            }
        } else {
            Serial.println(F("ERROR: DEFAULT requires value (0-5, 0=off)"));
        }
        return;
    }
    
    // === Global Settings ===
    if (strcmp(command, "BRIGHTNESS") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 1 && val <= 100) {
                currentProfile.brightness = val;
                applyProfile();
                Serial.print(F("Brightness: ")); Serial.println(val);
            } else {
                Serial.println(F("ERROR: Brightness must be 1-100"));
            }
        } else {
            Serial.println(F("ERROR: BRIGHTNESS requires value (1-100)"));
        }
        return;
    }
    
    if (strcmp(command, "SPEED") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 1 && val <= 10) {
                currentProfile.animSpeed = val;
                Serial.print(F("DPL Speed: ")); Serial.println(val);
            } else {
                Serial.println(F("ERROR: Speed must be 1-10"));
            }
        } else {
            Serial.println(F("ERROR: SPEED requires value (1-10)"));
        }
        return;
    }
    
    if (strcmp(command, "CBISPEED") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 1 && val <= 10) {
                currentProfile.cbiSpeed = val;
                Serial.print(F("CBI Speed: ")); Serial.println(val);
            } else {
                Serial.println(F("ERROR: CBI Speed must be 1-10"));
            }
        } else {
            Serial.println(F("ERROR: CBISPEED requires value (1-10)"));
        }
        return;
    }
    
    if (strcmp(command, "PERSONALITY") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 0 && val < PERS_COUNT) {
                currentProfile.personality = val;
                applyProfile();
                Serial.println(F("Personality updated"));
            } else {
                Serial.println(F("ERROR: Personality must be 0-4"));
            }
        } else {
            Serial.println(F("ERROR: PERSONALITY requires value (0-4)"));
        }
        return;
    }
    
    if (strcmp(command, "SCHEME") == 0) {
        if (arg1 != NULL) {
            int8_t newScheme = -1;
            
            // Check named schemes
            if (strcmp(arg1, "CLASSIC") == 0) newScheme = CLASSIC;
            else if (strcmp(arg1, "BLUE") == 0) newScheme = BLUE;
            else if (strcmp(arg1, "PINK") == 0) newScheme = PINK;
            else if (strcmp(arg1, "GREEN") == 0) newScheme = GREEN;
            else if (strcmp(arg1, "CYBERPUNK") == 0) newScheme = CYBERPUNK;
            else if (strcmp(arg1, "FOREST") == 0) newScheme = FOREST;
            else if (strcmp(arg1, "SUNSET") == 0) newScheme = SUNSET;
            else if (strcmp(arg1, "CUSTOM") == 0) newScheme = CUSTOM;
            else {
                // Try numeric
                int schemeNum = atoi(arg1);
                if (schemeNum >= 0 && schemeNum < SCHEME_COUNT) {
                    newScheme = schemeNum;
                }
            }
            
            if (newScheme != -1) {
                currentProfile.colorScheme = newScheme;
                applyProfile();
                Serial.println(F("Scheme updated"));
            } else {
                Serial.println(F("ERROR: Invalid scheme. Type HELP for list"));
            }
        } else {
            Serial.println(F("ERROR: SCHEME requires name or number"));
        }
        return;
    }
    
    if (strcmp(command, "COLOR") == 0) {
        if (arg1 != NULL) {
            char* arg2 = strtok(NULL, " ");
            char* arg3 = strtok(NULL, " ");
            char* arg4 = strtok(NULL, " ");
            
            if (arg2 && arg3 && arg4) {
                uint8_t section = atoi(arg1);
                if (section < SEC_COUNT) {
                    currentProfile.customColors[section] = CRGB(atoi(arg2), atoi(arg3), atoi(arg4));
                    currentProfile.colorScheme = CUSTOM;
                    applyProfile();
                    Serial.println(F("Custom color set"));
                } else {
                    Serial.println(F("ERROR: Invalid section"));
                }
            } else {
                Serial.println(F("ERROR: COLOR requires: section(0-11) R(0-255) G(0-255) B(0-255)"));
            }
        } else {
            Serial.println(F("ERROR: COLOR requires: section(0-11) R(0-255) G(0-255) B(0-255)"));
        }
        return;
    }
    
    if (strcmp(command, "SEQUENCE") == 0) {
        if (arg1 != NULL) {
            if (strcmp(arg1, "ON") == 0) {
                currentProfile.sequenceMode = true;
                Serial.println(F("Sequence: ON"));
            } else if (strcmp(arg1, "OFF") == 0) {
                currentProfile.sequenceMode = false;
                Serial.println(F("Sequence: OFF"));
            } else {
                Serial.println(F("ERROR: Use ON or OFF"));
            }
        } else {
            Serial.println(F("ERROR: SEQUENCE requires ON or OFF"));
        }
        return;
    }
    
    if (strcmp(command, "VOLTAGE") == 0) {
        if (arg1 != NULL) {
            if (strcmp(arg1, "ON") == 0) {
                currentProfile.voltageMonitorEnabled = true;
                Serial.println(F("Voltage Monitor: ON"));
            } else if (strcmp(arg1, "OFF") == 0) {
                currentProfile.voltageMonitorEnabled = false;
                Serial.println(F("Voltage Monitor: OFF"));
            } else {
                Serial.println(F("ERROR: Use ON or OFF"));
            }
        } else {
            Serial.println(F("ERROR: VOLTAGE requires ON or OFF"));
        }
        return;
    }
    
    // === Panel Modes ===
    if (strcmp(command, "LLMODE") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 0 && val <= 3) {
                currentProfile.llMode = val;
                Serial.println(F("Large Logic mode updated"));
            } else {
                Serial.println(F("ERROR: LLMODE must be 0-3"));
            }
        } else {
            Serial.println(F("ERROR: LLMODE requires value (0-3)"));
        }
        return;
    }
    
    if (strcmp(command, "CBIMODE") == 0) {
        if (arg1 != NULL) {
            int val = atoi(arg1);
            if (val >= 0 && val <= 6) {
                currentProfile.cbiMode = val;
                Serial.println(F("CBI mode updated"));
            } else {
                Serial.println(F("ERROR: CBIMODE must be 0-6"));
            }
        } else {
            Serial.println(F("ERROR: CBIMODE requires value (0-6)"));
        }
        return;
    }
    
    if (strcmp(command, "BARGRAPH") == 0) {
        if (arg1 != NULL) {
            if (strcmp(arg1, "SPLIT") == 0) {
                currentProfile.bargraphSplit = true;
                Serial.println(F("Bargraph: Split"));
            } else if (strcmp(arg1, "CLASSIC") == 0) {
                currentProfile.bargraphSplit = false;
                Serial.println(F("Bargraph: Classic"));
            } else {
                Serial.println(F("ERROR: Use SPLIT or CLASSIC"));
            }
        } else {
            Serial.println(F("ERROR: BARGRAPH requires SPLIT or CLASSIC"));
        }
        return;
    }
    
    if (strcmp(command, "TOPBLOCKS") == 0) {
        if (arg1 != NULL) {
            int mode = atoi(arg1);
            if (mode >= 0 && mode < TOPBLOCKS_MODE_COUNT) {
                currentProfile.topBlocksMode = mode;
                Serial.print(F("Top Blocks mode: "));
                Serial.println(mode);
            } else {
                Serial.println(F("ERROR: TOPBLOCKS must be 0-4"));
            }
        } else {
            Serial.println(F("ERROR: TOPBLOCKS requires value (0-4)"));
        }
        return;
    }
    
    // === Legacy Quick Commands ===
    if (command[0] == 'L' && strlen(command) == 2) {
        int mode = command[1] - '0';
        if (mode >= 0 && mode <= 3) {
            currentProfile.llMode = mode;
            Serial.print(F("LL Mode: ")); Serial.println(mode);
            return;
        }
    }
    
    if (command[0] == 'C' && strlen(command) == 2) {
        int mode = command[1] - '0';
        if (mode >= 0 && mode <= 6) {
            currentProfile.cbiMode = mode;
            Serial.print(F("CBI Mode: ")); Serial.println(mode);
            return;
        }
    }
    
    if (command[0] == 'P' && strlen(command) == 2) {
        int pers = command[1] - '0';
        if (pers >= 0 && pers < PERS_COUNT) {
            currentProfile.personality = pers;
            applyProfile();
            Serial.print(F("Personality: ")); Serial.println(pers);
            return;
        }
    }
    
    if (command[0] == 'S' && strlen(command) == 2) {
        int scheme = command[1] - '0';
        if (scheme >= 0 && scheme < SCHEME_COUNT) {
            currentProfile.colorScheme = scheme;
            applyProfile();
            Serial.print(F("Scheme: ")); Serial.println(scheme);
            return;
        }
    }
    
    // Unknown command
    Serial.println(F("Unknown command. Type HELP for list."));
}

// ============================================================================
// Config Mode Handler
// ============================================================================
void handleConfigMode() {
    static uint8_t blinkState = 0;
    EVERY_N_MILLISECONDS(500) {
        blinkState = !blinkState;
        fill_solid(leds_e, 3, blinkState ? CRGB::Blue : CRGB::Black);
        FastLED.show();
    }
}

// ============================================================================
// Animation Functions - DPL Panels
// ============================================================================
byte organicRandom(byte mode) {
    int8_t persParams[4];
    memcpy_P(persParams, PERSONALITY_PARAMS[currentProfile.personality], sizeof(persParams));
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
    fill_solid(leds_d, NUM_LEDS_D, CRGB::Black);
    
    auto getPixelIndex = [](int col, int row) -> int {
        return (col < 3) ? (col + row * 3) : ((col - 3) + 9 + row * 3);
    };
    
    switch(currentProfile.topBlocksMode) {
        case TOPBLOCKS_CLASSIC: {
            int mask = random(64);
            if (bitRead(mask, 0)) { leds_d[0] = colorTable[SEC_TOP_YELLOW]; leds_d[3] = colorTable[SEC_TOP_YELLOW]; }
            if (bitRead(mask, 1)) { leds_d[3] = colorTable[SEC_TOP_YELLOW]; leds_d[6] = colorTable[SEC_TOP_YELLOW]; }
            if (bitRead(mask, 2)) { leds_d[1] = colorTable[SEC_TOP_YELLOW]; leds_d[4] = colorTable[SEC_TOP_YELLOW]; }
            if (bitRead(mask, 3)) { leds_d[4] = colorTable[SEC_TOP_YELLOW]; leds_d[7] = colorTable[SEC_TOP_YELLOW]; }
            if (bitRead(mask, 4)) { leds_d[2] = colorTable[SEC_TOP_YELLOW]; leds_d[5] = colorTable[SEC_TOP_YELLOW]; }
            if (bitRead(mask, 5)) { leds_d[5] = colorTable[SEC_TOP_YELLOW]; leds_d[8] = colorTable[SEC_TOP_YELLOW]; }
            
            int mask2 = random(64);
            if (bitRead(mask2, 0)) { leds_d[9]  = colorTable[SEC_TOP_GREEN]; leds_d[12] = colorTable[SEC_TOP_GREEN]; }
            if (bitRead(mask2, 1)) { leds_d[12] = colorTable[SEC_TOP_GREEN]; leds_d[15] = colorTable[SEC_TOP_GREEN]; }
            if (bitRead(mask2, 2)) { leds_d[10] = colorTable[SEC_TOP_GREEN]; leds_d[13] = colorTable[SEC_TOP_GREEN]; }
            if (bitRead(mask2, 3)) { leds_d[13] = colorTable[SEC_TOP_GREEN]; leds_d[16] = colorTable[SEC_TOP_GREEN]; }
            if (bitRead(mask2, 4)) { leds_d[11] = colorTable[SEC_TOP_GREEN]; leds_d[14] = colorTable[SEC_TOP_GREEN]; }
            if (bitRead(mask2, 5)) { leds_d[14] = colorTable[SEC_TOP_GREEN]; leds_d[17] = colorTable[SEC_TOP_GREEN]; }
            break;
        }
        
        case TOPBLOCKS_HORIZONTAL: {
            int row = random(3);
            for (int col = 0; col < 6; col++) {
                leds_d[getPixelIndex(col, row)] = (col < 3) ? 
                    colorTable[SEC_TOP_YELLOW] : colorTable[SEC_TOP_GREEN];
            }
            break;
        }
        
        case TOPBLOCKS_PAIRED: {
            int pair = random(2);
            for (int col = 0; col < 6; col++) {
                CRGB color = (col < 3) ? colorTable[SEC_TOP_YELLOW] : colorTable[SEC_TOP_GREEN];
                leds_d[getPixelIndex(col, pair)] = color;
                leds_d[getPixelIndex(col, pair + 1)] = color;
            }
            break;
        }
        
        case TOPBLOCKS_CHAOS: {
            for (int col = 0; col < 6; col++) {
                CRGB color = (col < 3) ? colorTable[SEC_TOP_YELLOW] : colorTable[SEC_TOP_GREEN];
                int pattern = random(6);
                switch(pattern) {
                    case 1: leds_d[getPixelIndex(col, 0)] = color; break;
                    case 2: leds_d[getPixelIndex(col, 1)] = color; break;
                    case 3: leds_d[getPixelIndex(col, 2)] = color; break;
                    case 4:
                        leds_d[getPixelIndex(col, 0)] = color;
                        leds_d[getPixelIndex(col, 1)] = color;
                        break;
                    case 5:
                        leds_d[getPixelIndex(col, 1)] = color;
                        leds_d[getPixelIndex(col, 2)] = color;
                        break;
                }
            }
            break;
        }
        
        case TOPBLOCKS_SINGLE: {
            leds_d[random(9)] = colorTable[SEC_TOP_YELLOW];
            leds_d[9 + random(9)] = colorTable[SEC_TOP_GREEN];
            break;
        }
    }
}

void updateBlueLEDs() {
    byte pattern = organicRandom(4);
    for (int i = 0; i < 6; i++) {
        leds_c[i * 4] = bitRead(pattern, i) ? colorTable[SEC_BLUE_COLUMN] : CRGB::Black;
    }
}

byte updateBar(byte* data, byte maxVal) {
    *data = constrain((int)(*data) + (random(3) - 1), 1, maxVal);
    return *data;
}

void updateBargraph() {
    if (currentProfile.bargraphSplit) {
        for (int bar = 0; bar < 2; bar++) {
            updateBar(&bargraphData[bar], 6);
        }
    } else {
        bargraphData[1] = updateBar(&bargraphData[0], 6);
    }
    
    for (int level = 0; level < 6; level++) {
        int baseIndex = (5 - level) * 4 + 1;
        
        CRGB colorL = (level < bargraphData[0]) ? 
            (level >= 4 ? colorTable[SEC_BARGRAPH_HIGH] : 
             level >= 2 ? colorTable[SEC_BARGRAPH_MID] : 
             colorTable[SEC_BARGRAPH_LOW]) : CRGB::Black;
        
        CRGB colorR = (level < bargraphData[1]) ? 
            (level >= 4 ? colorTable[SEC_BARGRAPH_HIGH] : 
             level >= 2 ? colorTable[SEC_BARGRAPH_MID] : 
             colorTable[SEC_BARGRAPH_LOW]) : CRGB::Black;
        
        leds_c[baseIndex] = colorL;
        leds_c[baseIndex + 1] = colorR;
        leds_c[baseIndex + 2] = colorL;
    }
}

void updateBottomLEDs() {
    byte pattern = organicRandom(4);
    for (int i = 0; i < 6; i++) {
        leds_a[i] = bitRead(pattern, i) ? colorTable[SEC_BOTTOM_WHITE] : CRGB::Black;
    }
}

void updateRedLEDs() {
    leds_a[6] = random(2) ? colorTable[SEC_RED_LARGE] : CRGB::Black;
    leds_a[7] = random(2) ? colorTable[SEC_RED_LARGE] : CRGB::Black;
}

void updateVuMeter() {
    if (currentProfile.colorScheme == PINK) {
        const TProgmemRGBPalette16 pinkPalette_p PROGMEM = {
            CRGB::DeepPink, CRGB::HotPink, CRGB::HotPink, CRGB::White,
            CRGB::HotPink, CRGB::DeepPink, CRGB::Red, CRGB::DeepPink,
            CRGB::DeepPink, CRGB::HotPink, CRGB::White, CRGB::White,
            CRGB::HotPink, CRGB::Red, CRGB::Red, CRGB::DeepPink
        };
        uint8_t pulse = beatsin8(45, 60, 255);
        uint8_t colorIndex = millis() / 15;
        fill_palette(leds_b, NUM_LEDS_B, colorIndex, 8, pinkPalette_p, pulse, LINEARBLEND);
    }
    else if (currentProfile.personality == HAPPY) {
        fill_rainbow(leds_b, NUM_LEDS_B, gHue, 7);
    }
    else if (currentProfile.personality == SLEEPY) {
        fill_solid(leds_b, NUM_LEDS_B, CHSV(160, 255, beatsin8(10, 30, 100)));
    }
    else {
        uint8_t beat = beatsin8(62, 64, 255);
        for (int i = 0; i < NUM_LEDS_B / 2; i++) {
            leds_b[i] = leds_b[NUM_LEDS_B - 1 - i] = CHSV(gHue + (i * 4), 255, beat);
        }
    }
}

// ============================================================================
// CBI Panel Functions
// ============================================================================
void getVCC() {
    int rawValue = analogRead(ANALOG_INPUT);
    float vout = (rawValue * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
    float vin = vout / (VOLTAGE_R2 / (VOLTAGE_R1 + VOLTAGE_R2));
    
    leds_f[20] = (vin >= GREEN_VCC) ? CRGB::Green : CRGB::Black;
    leds_f[21] = (vin < GREEN_VCC && vin >= YELLOW_VCC) ? CRGB::Yellow : CRGB::Black;
    leds_f[22] = (vin < YELLOW_VCC) ? CRGB::Red : CRGB::Black;
}

void drawHeart() {
    int8_t pattern[20];
    memcpy_P(pattern, HEART_PATTERN, sizeof(pattern));
    for (int i = 0; i < 20; i++) {
        leds_f[i] = pattern[i] ? CRGB::Red : CRGB::Black;
    }
}

void drawSmiley() {
    int8_t pattern[20];
    memcpy_P(pattern, SMILEY_PATTERN, sizeof(pattern));
    for (int i = 0; i < 20; i++) {
        leds_f[i] = pattern[i] ? CRGB::Yellow : CRGB::Black;
    }
}

void matrixRain() {
    uint8_t count = currentProfile.voltageMonitorEnabled ? CBI_MATRIX_LEDS : NUM_LEDS_F;
    fadeToBlackBy(leds_f, count, 20);
    
    if (random8() < 30) {
        leds_f[random(5)] = CRGB::Green;
    }
    
    for (int r = 3; r > 0; r--) {
        for (int c = 0; c < 5; c++) {
            int srcIdx = (r - 1) * 5 + c;
            int dstIdx = r * 5 + c;
            if (leds_f[srcIdx].g > 0) {
                leds_f[dstIdx] = leds_f[srcIdx];
                leds_f[srcIdx].fadeToBlackBy(100);
            }
        }
    }
}

void updateCBIESB() {
    int8_t pattern[20];
    memcpy_P(pattern, ESB_PATTERNS[esbCounter], sizeof(pattern));
    for (int i = 0; i < 20; i++) {
        leds_f[i] = pattern[i] ? colorTable[SEC_CBI_MATRIX] : CRGB::Black;
    }
    esbCounter = (esbCounter + 1) % 28;
}

void updateCBILEDs() {
    static uint16_t specialCounter = 0;
    bool specialActive = false;
    
    uint8_t matrixCount = currentProfile.voltageMonitorEnabled ? CBI_MATRIX_LEDS : NUM_LEDS_F;
    
    if (currentProfile.personality == HAPPY && (specialCounter++ % SPECIAL_PATTERN_INTERVAL == 0)) {
        drawSmiley();
        specialActive = true;
    }
    else if (currentProfile.colorScheme == PINK && (specialCounter++ % SPECIAL_PATTERN_INTERVAL == 0)) {
        drawHeart();
        specialActive = true;
    }
    
    if (!specialActive) {
        switch (currentProfile.cbiMode) {
            case 0: // Organic
                for (int r = 0; r < 4; r++) {
                    byte p = organicRandom(random(4, 6));
                    for (int c = 0; c < 5; c++) {
                        leds_f[r * 5 + c] = bitRead(p, c) ? 
                            colorTable[SEC_CBI_MATRIX] : CRGB::Black;
                    }
                }
                break;
                
            case 1: // ESB
                updateCBIESB();
                break;
                
            case 2: // Rainbow
                fill_rainbow(leds_f, matrixCount, gHue, 7);
                break;
                
            case 3: // Sparkle
                fadeToBlackBy(leds_f, matrixCount, 10);
                leds_f[random16(matrixCount)] += CHSV(gHue + random8(64), 200, 255);
                break;
                
            case 4: // Heart
                drawHeart();
                break;
                
            case 5: // Smiley
                drawSmiley();
                break;
                
            case 6: // Matrix Rain
                matrixRain();
                break;
        }
    }
    
    if (currentProfile.voltageMonitorEnabled) {
        getVCC();
    }
}

// ============================================================================
// Large Logic Functions
// ============================================================================
void largeLogicBreathing() {
    uint8_t beat = beatsin8(5, 30, 255);
    CHSV color = rgb2hsv_approximate(colorTable[SEC_LARGE_LOGIC]);
    fill_solid(leds_e, NUM_LEDS_E, CHSV(color.h, color.s, beat));
}

void largeLogicPersonality() {
    switch (currentProfile.personality) {
        case HAPPY:
            fadeToBlackBy(leds_e, NUM_LEDS_E, 20);
            for (int i = 0; i < 8; i++) {
                leds_e[beatsin16(i + 7, 0, NUM_LEDS_E - 1)] |= CHSV(gHue + i * 32, 200, 255);
            }
            break;
            
        case GRUMPY:
            fill_solid(leds_e, NUM_LEDS_E, CHSV(0, 255, beatsin8(30, 10, 100)));
            break;
            
        case EXCITED:
            fill_rainbow(leds_e, NUM_LEDS_E, gHue * 3, 10);
            break;
            
        case SLEEPY:
            fill_solid(leds_e, NUM_LEDS_E, CHSV(160, 255, beatsin8(5, 10, 60)));
            break;
            
        default:
            largeLogicBreathing();
            break;
    }
}

void updateLargeLogic() {
    switch (currentProfile.llMode) {
        case 0: // Breathing
            largeLogicBreathing();
            break;
            
        case 1: // Rainbow
            fill_rainbow(leds_e, NUM_LEDS_E, gHue, 7);
            break;
            
        case 2: // Off
            fill_solid(leds_e, NUM_LEDS_E, CRGB::Black);
            break;
            
        case 3: // Personality
            largeLogicPersonality();
            break;
    }
}

// ============================================================================
// Startup Sequence
// ============================================================================
void startupSequence() {
    Serial.println(F("Running startup sequence..."));
    
    FastLED.clear();
    CRGB* strips[] = {leds_a, leds_b, leds_c, leds_d, leds_e, leds_f};
    int counts[] = {NUM_LEDS_A, NUM_LEDS_B, NUM_LEDS_C, NUM_LEDS_D, NUM_LEDS_E, NUM_LEDS_F};
    
    // Rainbow wave
    for (int hue = 0; hue < 255; hue += 64) {
        esp_task_wdt_reset();  // Feed watchdog during long animation
        for (int i = 0; i < TOTAL_LEDS; i++) {
            int current = 0;
            for (int s = 0; s < 6; s++) {
                if (i >= current && i < current + counts[s]) {
                    strips[s][i - current] = CHSV(hue, 255, 255);
                }
                current += counts[s];
            }
            FastLED.show();
            
            for (int s = 0; s < 6; s++) {
                fadeToBlackBy(strips[s], counts[s], 64);
            }
            delay(STARTUP_LED_DELAY);
        }
    }
    
    esp_task_wdt_reset();  // Feed watchdog before flash sequence
    
    // Flash all white
    for (int i = 0; i < 3; i++) {
        for (int s = 0; s < 6; s++) {
            fill_solid(strips[s], counts[s], CRGB::White);
        }
        FastLED.show();
        delay(STARTUP_FLASH_DELAY);
        
        FastLED.clear();
        FastLED.show();
        delay(STARTUP_FLASH_DELAY);
    }
    
    applyProfile();
    FastLED.show();
    Serial.println(F("Startup sequence complete!"));
}

// ============================================================================
// Setup Function
// ============================================================================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(100);
    
    Serial.println(F("\n============================================"));
    Serial.print(F("RGB DPL Controller v"));
    Serial.println(VERSION);
    Serial.println(F("============================================"));
    
    // Configure voltage monitor thresholds
    if (SYSTEM_VOLTAGE == 24) {
        GREEN_VCC = 23.5f;
        YELLOW_VCC = 22.0f;
        RED_VCC = 21.0f;
        Serial.println(F("Voltage monitor: 24V system"));
    } else {
        GREEN_VCC = 12.5f;
        YELLOW_VCC = 12.0f;
        RED_VCC = 11.5f;
        Serial.println(F("Voltage monitor: 12V system"));
    }
    
    // Initialize watchdog timer
    Serial.println(F("Enabling watchdog timer..."));
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // New API for ESP32 Arduino Core 3.x
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL);
    #else
    // Old API for ESP32 Arduino Core 2.x
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    #endif
    
    // Initialize EEPROM
    Serial.println(F("Initializing EEPROM..."));
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println(F("ERROR: EEPROM initialization failed!"));
        while(1) {
            delay(1000);
        }
    }
    
    // Configure GPIO
    pinMode(LEFT_DOOR_PIN, INPUT_PULLUP);
    pinMode(RIGHT_DOOR_PIN, INPUT_PULLUP);
    
    // Initialize I2C (no mutex needed with critical sections)
    Serial.println(F("Initializing I2C..."));
    Wire.begin(0x20);
    Wire.onReceive(i2cReceiveEvent);
    
    // Initialize FastLED
    Serial.println(F("Initializing LED strips..."));
    delay(100);
    
    FastLED.addLeds<WS2811, DATA_PIN_A, GRB>(leds_a, NUM_LEDS_A);
    FastLED.addLeds<WS2811, DATA_PIN_B, GRB>(leds_b, NUM_LEDS_B);
    FastLED.addLeds<WS2811, DATA_PIN_C, GRB>(leds_c, NUM_LEDS_C);
    FastLED.addLeds<WS2811, DATA_PIN_D, GRB>(leds_d, NUM_LEDS_D);
    FastLED.addLeds<WS2811, DATA_PIN_E, GRB>(leds_e, NUM_LEDS_E);
    FastLED.addLeds<WS2811, DATA_PIN_F, GRB>(leds_f, NUM_LEDS_F);
    
    set_max_power_in_volts_and_milliamps(5, 4000);
    
    // Load configuration
    Serial.println(F("Loading configuration..."));
    if (!loadSystemConfig()) {
        Serial.println(F("WARNING: System config load failed"));
    }
    
    Serial.print(F("Boot count: "));
    Serial.println(sysConfig.bootCount);
    
    if (sysConfig.defaultProfile > 0 && sysConfig.defaultProfile <= 5) {
        Serial.print(F("Loading default profile "));
        Serial.println(sysConfig.defaultProfile);
        if (!loadProfile(sysConfig.defaultProfile - 1)) {
            Serial.println(F("WARNING: Profile load failed, using defaults"));
            initDefaultProfile();
        }
    } else {
        Serial.println(F("No default profile, using defaults"));
        initDefaultProfile();
    }
    
    // Run startup sequence
    startupSequence();
    
    // System ready
    Serial.println(F("\n============================================"));
    Serial.println(F("System ready! Type HELP for commands."));
    Serial.println(F("============================================\n"));
    
    // Print health info
    Serial.print(F("Free heap: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
}

// ============================================================================
// Main Loop
// ============================================================================
void loop() {
    // Feed watchdog
    esp_task_wdt_reset();
    
    // === Serial Input Handling ===
    if (Serial.available() > 0) {
        isWaitingForCommand = true;
        lastSerialActivityTime = millis();
        
        while (Serial.available() > 0) {
            char inChar = Serial.read();
            
            if (inChar == '\n') {
                if (serialBufferPos > 0) {
                    serialBuffer[serialBufferPos] = '\0';
                    processSerialCommand();
                }
                serialBufferPos = 0;
                memset(serialBuffer, 0, sizeof(serialBuffer));
                isWaitingForCommand = false;
            }
            else if (inChar == '\r') {
                // Ignore carriage return
            }
            else if (inChar >= 32 && inChar < 127) {
                if (serialBufferPos < SERIAL_BUFFER_SIZE - 1) {
                    serialBuffer[serialBufferPos++] = inChar;
                } else {
                    // Buffer full - discard command
                    Serial.println(F("ERROR: Command too long"));
                    serialBufferPos = 0;
                    memset(serialBuffer, 0, sizeof(serialBuffer));
                    isWaitingForCommand = false;
                }
            }
        }
    }
    
    // Overflow-safe timeout check
    if (isWaitingForCommand) {
        uint32_t elapsed = millis() - lastSerialActivityTime;
        if (elapsed > SERIAL_TIMEOUT_MS) {
            isWaitingForCommand = false;
            serialBufferPos = 0;
            memset(serialBuffer, 0, sizeof(serialBuffer));
            Serial.println(F("ERROR: Command timeout"));
        }
    }
    
    // === Main Animation Logic ===
    if (!isWaitingForCommand) {
        // Process I2C commands (with critical section protection)
        bool localI2CCommandReady = false;
        portENTER_CRITICAL(&i2cMux);
        if (i2cCommandReady) {
            localI2CCommandReady = true;
            i2cCommandReady = false;
        }
        portEXIT_CRITICAL(&i2cMux);
        
        if (localI2CCommandReady) {
            processI2CCommand();
        }
        
        // Config mode takes priority
        if (configMode) {
            handleConfigMode();
            return;
        }
        
        // Sequence mode auto-cycling
        if (currentProfile.sequenceMode && sequenceTimer.ready()) {
            currentProfile.cbiMode = (currentProfile.cbiMode + 1) % 7;
            currentProfile.llMode = (currentProfile.llMode + 1) % 4;
        }
        
        // Calculate speed multipliers from personality
        int8_t persParams[4];
        memcpy_P(persParams, PERSONALITY_PARAMS[currentProfile.personality], sizeof(persParams));
        float globalSpeedMult = (persParams[0] / 10.0f) * (currentProfile.animSpeed / 5.0f);
        float cbiSpeedMult = (persParams[0] / 10.0f) * (currentProfile.cbiSpeed / 5.0f);
        
        // Update timer intervals
        topBlockTimer.setInterval(TOPBLOCK_BASE_INTERVAL / globalSpeedMult);
        blueTimer.setInterval(BLUE_BASE_INTERVAL / globalSpeedMult);
        bargraphTimer.setInterval(BARGRAPH_BASE_INTERVAL / globalSpeedMult);
        bottomTimer.setInterval(BOTTOM_BASE_INTERVAL / globalSpeedMult);
        redTimer.setInterval(RED_BASE_INTERVAL / globalSpeedMult);
        
        uint32_t cbiInterval = (currentProfile.cbiMode == 1) ? 
                               (CBI_ESB_INTERVAL / cbiSpeedMult) : 
                               (CBI_BASE_INTERVAL / cbiSpeedMult);
        cbiTimer.setInterval(cbiInterval);
        
        // === Left Door Panels ===
        if (digitalRead(LEFT_DOOR_PIN) == LOW) {
            if (topBlockTimer.ready()) updateTopBlocks();
            if (blueTimer.ready()) updateBlueLEDs();
            if (bargraphTimer.ready()) updateBargraph();
            if (bottomTimer.ready()) updateBottomLEDs();
            if (redTimer.ready()) updateRedLEDs();
            updateVuMeter();
        } else {
            // Door closed - turn off panels
            fill_solid(leds_a, NUM_LEDS_A, CRGB::Black);
            fill_solid(leds_b, NUM_LEDS_B, CRGB::Black);
            fill_solid(leds_c, NUM_LEDS_C, CRGB::Black);
            fill_solid(leds_d, NUM_LEDS_D, CRGB::Black);
        }
        
        // === Right Door Panel (CBI) ===
        if (digitalRead(RIGHT_DOOR_PIN) == LOW) {
            if (cbiTimer.ready()) {
                updateCBILEDs();
            }
        } else {
            fill_solid(leds_f, NUM_LEDS_F, CRGB::Black);
        }
        
        // === Large Logic (always active) ===
        updateLargeLogic();
        
        // Update LEDs
        FastLED.show();
        
        // Global hue increment
        EVERY_N_MILLISECONDS(20) {
            gHue++;
            lHue++;
        }
    }
}