#include "RotaryEncoderDriver.h"

RotaryEncoderDriver::RotaryEncoderDriver()
    : pinA(ENCODER_PIN_A), pinB(ENCODER_PIN_B), pinBtn(ENCODER_PIN_BTN),
      lastPinAState(HIGH), currentStepIdx(2), // Default to 1 kHz step
      lastBtnState(HIGH), lastBtnDebounceTime(0),
      turnCallback(nullptr), buttonCallback(nullptr) {
}

void RotaryEncoderDriver::begin(int pinA, int pinB, int pinBtn) {
    this->pinA = pinA;
    this->pinB = pinB;
    this->pinBtn = pinBtn;

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
    // 1. Read Rotary Encoder Rotation
    uint8_t aState = digitalRead(pinA);
    if (aState != lastPinAState) {
        if (aState == LOW) {
            // Falling edge on Pin A
            if (digitalRead(pinB) == HIGH) {
                // Clockwise rotation
                if (turnCallback) turnCallback(1, getStepSize());
            } else {
                // Counter-clockwise rotation
                if (turnCallback) turnCallback(-1, getStepSize());
            }
        }
        lastPinAState = aState;
    }

    // 2. Read Rotary Encoder Push Button
    uint8_t btnReading = digitalRead(pinBtn);
    if (btnReading != lastBtnState) {
        if ((millis() - lastBtnDebounceTime) > 50) { // 50ms debounce
            if (btnReading == LOW) { // Button Pressed
                cycleStepSize();
                if (buttonCallback) buttonCallback();
            }
            lastBtnDebounceTime = millis();
            lastBtnState = btnReading;
        }
    }
}
