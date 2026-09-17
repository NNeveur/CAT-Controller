/**
 * @file TS2000_MaTouch_CAT.ino
 * @brief Sketch principal pour le contrôleur CAT Kenwood TS-2000 sur MaTouch ESP32-S3 2.1".
 *
 * Ce programme gère :
 * - La communication série CAT RS-232 avec le Kenwood TS-2000
 * - La lecture tactile du panneau ST7701 (I2C CST816D / GT911)
 * - La lecture de l'encodeur rotatif et son bouton pour le réglage de fréquence
 * - L'affichage graphique dynamique de la fréquence VFO A, du mode et de la vitesse série
 * - La sauvegarde en mémoire NVS (Preferences ESP32) de la vitesse de communication série
 */

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  #if __has_include(<sdkconfig.h>)
    #include <sdkconfig.h>
  #endif
#endif

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include "Config.h"
#include "KenwoodCAT.h"
#include "RotaryEncoderDriver.h"
#include "DisplayGUI.h"

// Instances des modules matériels et logiciels
KenwoodCAT radio;
RotaryEncoderDriver encoder;
DisplayGUI gui;
Preferences preferences;

// Temporisateur pour l'interrogation périodique de l'état du TS-2000
unsigned long lastCatPoll = 0;
const unsigned long CAT_POLL_INTERVAL = 1000; // Interrogation toutes les 1000 ms (1s)

// Adresse I2C du contrôleur tactile (CST816D / GT911)
#define TOUCH_I2C_ADDR 0x15

/**
 * @brief Initialise le bus I2C pour le contrôleur tactile.
 */
void setupTouch() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
}

/**
 * @brief Lit les coordonnées d'appui tactile via I2C.
 * @param touchX Variable de sortie pour la position X
 * @param touchY Variable de sortie pour la position Y
 * @return true si un point de contact est détecté, false sinon
 */
bool readTouch(int16_t &touchX, int16_t &touchY) {
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    Wire.write(0x02); // Registre de début des données tactiles
    if (Wire.endTransmission() != 0) return false;

    if (Wire.requestFrom((uint8_t)TOUCH_I2C_ADDR, (uint8_t)6) == 6) {
        uint8_t touchPoints = Wire.read() & 0x0F;
        if (touchPoints > 0) {
            uint8_t xHigh = Wire.read() & 0x0F;
            uint8_t xLow = Wire.read();
            uint8_t yHigh = Wire.read() & 0x0F;
            uint8_t yLow = Wire.read();
            Wire.read(); // Octet de geste / drapeaux inutilisé ici

            touchX = (xHigh << 8) | xLow;
            touchY = (yHigh << 8) | yLow;
            return true;
        }
    }
    return false;
}

/**
 * @brief Rappel (callback) exécuté lors de la rotation de l'encodeur rotatif.
 * @param steps Nombre de crans tournés (positif pour horaire, négatif pour anti-horaire)
 * @param stepSize Pas de réglage actuel en Hz (ex: 1000 pour 1 kHz)
 */
void onEncoderTurn(int steps, uint32_t stepSize) {
    // Modifier la fréquence uniquement sur l'écran principal
    if (gui.getScreen() != SCREEN_MAIN) return;

    uint32_t currentFreq = radio.getFrequency();
    int64_t newFreq = (int64_t)currentFreq + ((int64_t)steps * stepSize);

    // Limitation de la plage de fréquence (30 kHz à 60 MHz)
    if (newFreq < 30000) newFreq = 30000;
    if (newFreq > 60000000) newFreq = 60000000;

    radio.setFrequency((uint32_t)newFreq);
}

/**
 * @brief Rappel (callback) exécuté lors du clic sur le bouton de l'encodeur.
 * Note : Le changement de pas de réglage est déjà géré directement dans le pilote d'encodeur.
 */
void onEncoderBtnClick() {
    // Action supplémentaire sur clic bouton si nécessaire
}

void setup() {
    Serial.begin(115200);
    Serial.println("Démarrage du contrôleur CAT Kenwood TS-2000...");

    // Chargement de la vitesse du port série enregistrée en NVS (ou 57600 bauds par défaut)
    preferences.begin("ts2000_cat", false);
    uint32_t savedBaud = preferences.getUInt("baud_rate", DEFAULT_CAT_BAUDRATE);

    // Initialisation de la liaison CAT sur Serial1 (GPIO 44 RX, GPIO 43 TX)
    radio.begin(Serial1, savedBaud, CAT_RX_PIN, CAT_TX_PIN);

    // Initialisation du pilote d'encodeur rotatif
    encoder.begin(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PIN_BTN);
    encoder.onTurn(onEncoderTurn);
    encoder.onButtonClick(onEncoderBtnClick);

    // Initialisation du contrôleur tactile I2C
    setupTouch();

    // Initialisation du pilote graphique ST7701
    gui.begin();
}

void loop() {
    // 1. Traitement des données et réponses série CAT arrivantes
    radio.update();

    // 2. Traitement de la rotation et des clics de l'encodeur rotatif
    encoder.update();

    // 3. Traitement des événements tactiles à l'écran
    int16_t tx = 0, ty = 0;
    static bool wasTouched = false;

    if (readTouch(tx, ty)) {
        if (!wasTouched) { // Déclenchement sur le front descendant / premier appui
            wasTouched = true;
            RadioMode newMode = radio.getMode();
            uint32_t newFreq = radio.getFrequency();
            uint32_t newBaud = radio.getBaudRate();

            if (gui.checkTouch(tx, ty, newMode, newFreq, newBaud)) {
                if (newMode != radio.getMode()) {
                    radio.setMode(newMode);
                }
                if (newFreq != radio.getFrequency()) {
                    radio.setFrequency(newFreq);
                }
                if (newBaud != radio.getBaudRate()) {
                    radio.setBaudRate(newBaud);
                    preferences.putUInt("baud_rate", newBaud); // Sauvegarde permanente en NVS
                }
            }
        }
    } else {
        wasTouched = false;
    }

    // 4. Demande périodique d'état (IF) au TS-2000 pour vérifier la connexion
    if (millis() - lastCatPoll > CAT_POLL_INTERVAL) {
        lastCatPoll = millis();
        if (gui.getScreen() == SCREEN_MAIN) {
            radio.requestStatus();
        }
    }

    // 5. Rafraîchissement dynamique de l'interface graphique
    gui.update(radio.getFrequency(), radio.getMode(), encoder.getStepSize(), radio.isConnected(), radio.getBaudRate());
}
