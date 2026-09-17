#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file Config.h
 * @brief Configuration globale pour l'application Kenwood TS-2000 CAT Controller.
 *
 * Ce fichier définit l'ensemble des constantes matérielles et logicielles :
 * - Broches de l'écran IPS ST7701 480x480 (MaTouch ESP32-S3 2.1")
 * - Broches du contrôleur tactile I2C (CST816D / GT911)
 * - Broches de l'encodeur rotatif et bouton poussoir
 * - Broches et vitesses de la liaison série RS-232 CAT
 * - Définition des bandes radio, modes de fonctionnement et pas de réglage
 */

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  #if __has_include(<sdkconfig.h>)
    #include <sdkconfig.h>
  #endif
#endif

#include <Arduino.h>

// ============================================================================
// Configuration de l'affichage (MaTouch ESP32-S3 2.1" ST7701 IPS 480x480)
// ============================================================================
#define TFT_WIDTH   480
#define TFT_HEIGHT  480

// Broches d'interface SPI 3-fils & Bus RGB parallèle du ST7701
#define TFT_DE      2
#define TFT_VSYNC   42
#define TFT_HSYNC   3
#define TFT_PCLK    45

#define TFT_R0      4
#define TFT_R1      41
#define TFT_R2      5
#define TFT_R3      40
#define TFT_R4      6

#define TFT_G0      39
#define TFT_G1      7
#define TFT_G2      47
#define TFT_G3      8
#define TFT_G4      48
#define TFT_G5      9

#define TFT_B0      11
#define TFT_B1      15
#define TFT_B2      12
#define TFT_B3      16
#define TFT_B4      21

#define TFT_SCLK    46
#define TFT_MOSI    0
#define TFT_CS      1
#define TFT_BLK     38 // Broche de commande du rétroéclairage (Backlight)

// ============================================================================
// Configuration du contrôleur tactile (I2C CST816D / GT911)
// ============================================================================
#define TOUCH_SDA   17
#define TOUCH_SCL   18
#define TOUCH_INT   38
#define TOUCH_RST   -1 // Broche de reset tactile non connectée / contrôlée en matériel

// ============================================================================
// Configuration des broches de l'encodeur rotatif
// ============================================================================
#define ENCODER_PIN_A   13 // Phase A de l'encodeur
#define ENCODER_PIN_B   10 // Phase B de l'encodeur
#define ENCODER_PIN_BTN 14 // Bouton poussoir central / intégré

// ============================================================================
// Liaison série RS-232 CAT Kenwood TS-2000
// ============================================================================
// Broches UART de l'ESP32-S3 reliées à l'adaptateur MAX3232 TTL->RS232
#define CAT_RX_PIN  44   // ESP32 RX <- MAX3232 TX
#define CAT_TX_PIN  43   // ESP32 TX -> MAX3232 RX
#define DEFAULT_CAT_BAUDRATE 57600 // Vitesse CAT par défaut du Kenwood TS-2000

// Vitesses de communication CAT supportées
const uint32_t CAT_BAUD_RATES[] = { 4800, 9600, 19200, 38400, 57600 };
const uint8_t NUM_BAUD_RATES = sizeof(CAT_BAUD_RATES) / sizeof(CAT_BAUD_RATES[0]);

// ============================================================================
// États des écrans de l'interface graphique (GUI)
// ============================================================================
enum AppScreen {
    SCREEN_MAIN,   // Écran principal (Fréquence VFO, Modes, Bandes)
    SCREEN_CONFIG  // Écran de configuration de la vitesse de la liaison CAT
};

// ============================================================================
// Modes de fonctionnement du Kenwood TS-2000 (Commande CAT MD<n>;)
// ============================================================================
enum RadioMode {
    MODE_LSB = 1, // Bandes inférieures (Bande Latérale Unique)
    MODE_USB = 2, // Bandes supérieures (Bande Latérale Unique)
    MODE_CW  = 3, // Télégraphie (Morse)
    MODE_FM  = 4, // Modulation de fréquence
    MODE_AM  = 5, // Modulation d'amplitude
    MODE_FSK = 6  // Radiotélétype / FSK
};

// ============================================================================
// Définition des bandes de fréquences
// ============================================================================
struct BandInfo {
    const char* name;       // Nom affiché sur le bouton (ex: "80M")
    uint32_t defaultFreqHz; // Fréquence de départ en Hz
    RadioMode defaultMode;  // Mode par défaut pour cette bande
    uint32_t minFreqHz;     // Limite inférieure de la bande en Hz
    uint32_t maxFreqHz;     // Limite supérieure de la bande en Hz
};

const BandInfo BANDS[] = {
    {"80M", 3650000,  MODE_LSB, 3500000,  3800000},
    {"40M", 7100000,  MODE_LSB, 7000000,  7200000},
    {"20M", 14200000, MODE_USB, 14000000, 14350000},
    {"10M", 28500000, MODE_USB, 28000000, 29700000}
};

const uint8_t NUM_BANDS = sizeof(BANDS) / sizeof(BANDS[0]);

// ============================================================================
// Pas de réglage de la fréquence par l'encodeur (Hz)
// ============================================================================
const uint32_t TUNING_STEPS[] = { 10, 100, 1000, 10000, 100000 };
const uint8_t NUM_TUNING_STEPS = sizeof(TUNING_STEPS) / sizeof(TUNING_STEPS[0]);

#endif // CONFIG_H
