#ifndef ROTARY_ENCODER_DRIVER_H
#define ROTARY_ENCODER_DRIVER_H

/**
 * @file RotaryEncoderDriver.h
 * @brief Pilote pour encodeur rotatif quadratique avec bouton poussoir intégré.
 *
 * Ce module gère :
 * - La détection de rotation horaire et anti-horaire de l'encodeur
 * - L'anti-rebond matériel/logiciel du bouton poussoir central
 * - Le défillement cyclique des pas de réglage de fréquence (10Hz, 100Hz, 1kHz, 10kHz, 100kHz)
 */

#include <Arduino.h>
#include "Config.h"

class RotaryEncoderDriver {
public:
    RotaryEncoderDriver();

    /**
     * @brief Initialise les broches d'entrée en PULLUP et configure l'état initial.
     * @param pinA Broche signal Phase A
     * @param pinB Broche signal Phase B
     * @param pinBtn Broche du bouton poussoir
     */
    void begin(int pinA = ENCODER_PIN_A, int pinB = ENCODER_PIN_B, int pinBtn = ENCODER_PIN_BTN);

    /**
     * @brief Scrute les broches d'entrée pour détecter la rotation ou l'appui bouton. Doit être appelé dans loop().
     */
    void update();

    /**
     * @brief Obtenir le pas de réglage actuel en Hz.
     * @return Pas de réglage en Hz (ex: 1000)
     */
    uint32_t getStepSize() const { return TUNING_STEPS[currentStepIdx]; }

    /**
     * @brief Obtenir l'index du pas de réglage dans le tableau TUNING_STEPS.
     * @return Index (0 à NUM_TUNING_STEPS - 1)
     */
    uint8_t getStepIndex() const { return currentStepIdx; }

    /**
     * @brief Bascule au pas de réglage suivant de manière cyclique.
     */
    void cycleStepSize();

    // Types de rappels (callbacks)
    typedef void (*EncoderTurnCallback)(int steps, uint32_t stepSize);
    typedef void (*ButtonClickCallback)();

    /**
     * @brief Définit la fonction de rappel lors d'une rotation.
     */
    void onTurn(EncoderTurnCallback cb) { turnCallback = cb; }

    /**
     * @brief Définit la fonction de rappel lors d'un clic sur le bouton.
     */
    void onButtonClick(ButtonClickCallback cb) { buttonCallback = cb; }

private:
    int pinA;   ///< Numéro de broche GPIO Phase A
    int pinB;   ///< Numéro de broche GPIO Phase B
    int pinBtn; ///< Numéro de broche GPIO Bouton poussoir

    uint8_t lastPinAState;    ///< Dernier état logique lu sur la broche Phase A
    uint8_t currentStepIdx;   ///< Index du pas de fréquence actif

    // Variables pour l'anti-rebond logiciel du bouton
    uint8_t lastBtnState;          ///< Dernier état logique du bouton
    uint32_t lastBtnDebounceTime;  ///< Horodatage du dernier changement d'état (ms)

    EncoderTurnCallback turnCallback;
    ButtonClickCallback buttonCallback;
};

#endif // ROTARY_ENCODER_DRIVER_H
