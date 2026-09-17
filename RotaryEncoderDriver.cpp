/**
 * @file RotaryEncoderDriver.cpp
 * @brief Implémentation du pilote de l'encodeur rotatif et du bouton poussoir.
 */

#include "RotaryEncoderDriver.h"

RotaryEncoderDriver::RotaryEncoderDriver()
    : pinA(ENCODER_PIN_A), pinB(ENCODER_PIN_B), pinBtn(ENCODER_PIN_BTN),
      lastPinAState(HIGH), currentStepIdx(2), // Pas par défaut de 1 kHz (index 2 dans TUNING_STEPS)
      lastBtnState(HIGH), lastBtnDebounceTime(0),
      turnCallback(nullptr), buttonCallback(nullptr) {
}

void RotaryEncoderDriver::begin(int pinA, int pinB, int pinBtn) {
    this->pinA = pinA;
    this->pinB = pinB;
    this->pinBtn = pinBtn;

    // Configuration des broches avec résistance de tirage vers le haut interne (PULLUP)
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(pinBtn, INPUT_PULLUP);

    lastPinAState = digitalRead(pinA);
    lastBtnState = digitalRead(pinBtn);
}

void RotaryEncoderDriver::cycleStepSize() {
    currentStepIdx = (currentStepIdx + 1) % NUM_TUNING_STEPS;
}

void RotaryEncoderDriver::update() {
    // 1. Lecture de la rotation de l'encodeur rotatif (Signal de quadrature)
    uint8_t aState = digitalRead(pinA);
    if (aState != lastPinAState) {
        if (aState == LOW) {
            // Front descendant détecté sur la Phase A
            if (digitalRead(pinB) == HIGH) {
                // Rotation dans le sens horaire (incrementation)
                if (turnCallback) turnCallback(1, getStepSize());
            } else {
                // Rotation dans le sens anti-horaire (decrementation)
                if (turnCallback) turnCallback(-1, getStepSize());
            }
        }
        lastPinAState = aState;
    }

    // 2. Lecture du bouton poussoir de l'encodeur avec anti-rebond logiciel
    uint8_t btnReading = digitalRead(pinBtn);
    if (btnReading != lastBtnState) {
        if ((millis() - lastBtnDebounceTime) > 50) { // Anti-rebond de 50 ms
            if (btnReading == LOW) { // Bouton enfoncé (actif à l'état bas)
                cycleStepSize();
                if (buttonCallback) buttonCallback();
            }
            lastBtnDebounceTime = millis();
            lastBtnState = btnReading;
        }
    }
}
