#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// Display Configuration (MaTouch ESP32-S3 2.1" ST7701 IPS Display 480x480)
// ============================================================================
#define TFT_WIDTH   480
#define TFT_HEIGHT  480

// ST7701 3-Wire SPI / RGB Pins for MaTouch ESP32-S3 2.1"
#define TFT_DE      40
#define TFT_VSYNC   41
#define TFT_HSYNC   42
#define TFT_PCLK    39

#define TFT_R0      45
#define TFT_R1      48
#define TFT_R2      47
#define TFT_R3      21
#define TFT_R4      14

#define TFT_G0      5
#define TFT_G1      6
#define TFT_G2      7
#define TFT_G3      15
#define TFT_G4      16
#define TFT_G5      4

#define TFT_B0      8
#define TFT_B1      3
#define TFT_B2      46
#define TFT_B3      9
#define TFT_B4      1

#define TFT_SCLK    12
#define TFT_MOSI    11
#define TFT_CS      10
#define TFT_BLK     44 // Backlight PIN

// ============================================================================
// Touch Controller Configuration (I2C CST816D / GT911)
// ============================================================================
#define TOUCH_SDA   17
#define TOUCH_SCL   18
#define TOUCH_INT   38
#define TOUCH_RST   -1

// ============================================================================
// Rotary Encoder Pinout Configuration
// ============================================================================
#define ENCODER_PIN_A   13
#define ENCODER_PIN_B   2
#define ENCODER_PIN_BTN 0  // Built-in button or center knob button

// ============================================================================
// Kenwood CAT Serial RS-232 Communication Pins & Settings
// ============================================================================
// ESP32-S3 Hardware Serial 1 / Serial 2
#define CAT_RX_PIN  19   // ESP32 RX <- MAX3232 TX
#define CAT_TX_PIN  20   // ESP32 TX -> MAX3232 RX
#define DEFAULT_CAT_BAUDRATE 57600 // Default Kenwood TS-2000 CAT Baud Rate

const uint32_t CAT_BAUD_RATES[] = { 4800, 9600, 19200, 38400, 57600 };
const uint8_t NUM_BAUD_RATES = sizeof(CAT_BAUD_RATES) / sizeof(CAT_BAUD_RATES[0]);

// ============================================================================
// GUI Screen States
// ============================================================================
enum AppScreen {
    SCREEN_MAIN,
    SCREEN_CONFIG
};

// ============================================================================
// Radio Operating Modes (Kenwood TS-2000 Command MD<n>;)
// ============================================================================
enum RadioMode {
    MODE_LSB = 1,
    MODE_USB = 2,
    MODE_CW  = 3,
    MODE_FM  = 4,
    MODE_AM  = 5,
    MODE_FSK = 6
};

// ============================================================================
// Frequency Bands Definition
// ============================================================================
struct BandInfo {
    const char* name;
    uint32_t defaultFreqHz;
    RadioMode defaultMode;
    uint32_t minFreqHz;
    uint32_t maxFreqHz;
};

const BandInfo BANDS[] = {
    {"80M", 3650000, MODE_LSB, 3500000, 3800000},
    {"40M", 7100000, MODE_LSB, 7000000, 7200000},
    {"20M", 14200000, MODE_USB, 14000000, 14350000},
    {"10M", 28500000, MODE_USB, 28000000, 29700000}
};

const uint8_t NUM_BANDS = sizeof(BANDS) / sizeof(BANDS[0]);

// ============================================================================
// Tuning Steps Definition
// ============================================================================
const uint32_t TUNING_STEPS[] = { 10, 100, 1000, 10000, 100000 };
const uint8_t NUM_TUNING_STEPS = sizeof(TUNING_STEPS) / sizeof(TUNING_STEPS[0]);

#endif // CONFIG_H
