#ifndef DISPLAY_GUI_H
#define DISPLAY_GUI_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include "Config.h"

// Struct for UI Button Definition
struct TouchButton {
    int16_t x, y, w, h;
    const char* label;
    bool isSelected;
    uint16_t color;
    uint16_t activeColor;
};

class DisplayGUI {
public:
    DisplayGUI();

    void begin();
    void update(uint32_t currentFreq, RadioMode currentMode, uint32_t stepSize, bool connected);

    // Touch event handling
    bool checkTouch(int16_t touchX, int16_t touchY, RadioMode& outNewMode, uint32_t& outNewFreq);

private:
    Arduino_GFX* gfx;

    // GUI cached state to prevent redrawing unchanged UI elements (flicker reduction)
    uint32_t lastFreq;
    RadioMode lastMode;
    uint32_t lastStepSize;
    bool lastConnected;

    // UI Buttons
    TouchButton modeButtons[4]; // LSB, USB, AM, FM
    TouchButton bandButtons[4]; // 80M, 40M, 20M, 10M

    void initButtons();
    void drawHeader(bool connected);
    void drawFrequency(uint32_t freqHz);
    void drawStepSize(uint32_t stepSize);
    void drawModeButtons(RadioMode currentMode);
    void drawBandButtons(uint32_t currentFreq);
    void drawButton(const TouchButton& btn);
    bool isPointInButton(int16_t px, int16_t py, const TouchButton& btn);

    String formatFrequency(uint32_t freqHz);
};

#endif // DISPLAY_GUI_H
