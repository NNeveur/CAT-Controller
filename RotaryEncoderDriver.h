#ifndef ROTARY_ENCODER_DRIVER_H
#define ROTARY_ENCODER_DRIVER_H

#include <Arduino.h>
#include "Config.h"

class RotaryEncoderDriver {
public:
    RotaryEncoderDriver();

    void begin(int pinA = ENCODER_PIN_A, int pinB = ENCODER_PIN_B, int pinBtn = ENCODER_PIN_BTN);
    void update();

    // Step size management
    uint32_t getStepSize() const { return TUNING_STEPS[currentStepIdx]; }
    uint8_t getStepIndex() const { return currentStepIdx; }
    void cycleStepSize();

    // Callbacks
    typedef void (*EncoderTurnCallback)(int steps, uint32_t stepSize);
    typedef void (*ButtonClickCallback)();

    void onTurn(EncoderTurnCallback cb) { turnCallback = cb; }
    void onButtonClick(ButtonClickCallback cb) { buttonCallback = cb; }

private:
    int pinA;
    int pinB;
    int pinBtn;

    uint8_t lastPinAState;
    uint8_t currentStepIdx;

    // Button debounce state
    uint8_t lastBtnState;
    uint32_t lastBtnDebounceTime;

    EncoderTurnCallback turnCallback;
    ButtonClickCallback buttonCallback;
};

#endif // ROTARY_ENCODER_DRIVER_H
