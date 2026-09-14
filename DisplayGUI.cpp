#include "DisplayGUI.h"

// Color Palette (16-bit RGB565)
#define COLOR_BG          0x0000 // Black
#define COLOR_CARD_BG     0x18E3 // Dark Gray / Blue-gray
#define COLOR_HEADER      0x0A2D // Deep Navy
#define COLOR_TEXT        0xFFFF // White
#define COLOR_TEXT_MUTED  0x9CD5 // Light Gray
#define COLOR_ACCENT      0x041F // Cyan / Blue Accent
#define COLOR_FREQ        0x07E0 // Bright Green VFO text
#define COLOR_ACTIVE_BTN  0x03E0 // Active Green
#define COLOR_INACTIVE_BTN 0x2A54 // Dark Slate
#define COLOR_BORDER      0x52AA // Silver Border
#define COLOR_RED         0xF800 // Red (Disconnected)
#define COLOR_GREEN       0x07E0 // Green (Connected)

DisplayGUI::DisplayGUI()
    : gfx(nullptr), lastFreq(0), lastMode(MODE_CW), lastStepSize(0), lastConnected(false) {
    initButtons();
}

void DisplayGUI::initButtons() {
    // Mode Buttons (4 buttons: LSB, USB, AM, FM) arranged in row 1
    // 480x480 screen, center content inside round/square display
    const int16_t modeY = 270;
    const int16_t btnWidth = 90;
    const int16_t btnHeight = 50;
    const int16_t spacing = 15;
    const int16_t startX = 35;

    modeButtons[0] = { startX,                             modeY, btnWidth, btnHeight, "LSB", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    modeButtons[1] = { startX + (btnWidth + spacing),     modeY, btnWidth, btnHeight, "USB", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    modeButtons[2] = { startX + 2 * (btnWidth + spacing), modeY, btnWidth, btnHeight, "AM",  false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    modeButtons[3] = { startX + 3 * (btnWidth + spacing), modeY, btnWidth, btnHeight, "FM",  false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };

    // Band Buttons (4 buttons: 80M, 40M, 20M, 10M) arranged in row 2
    const int16_t bandY = 345;
    bandButtons[0] = { startX,                             bandY, btnWidth, btnHeight, "80m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
    bandButtons[1] = { startX + (btnWidth + spacing),     bandY, btnWidth, btnHeight, "40m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
    bandButtons[2] = { startX + 2 * (btnWidth + spacing), bandY, btnWidth, btnHeight, "20m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
    bandButtons[3] = { startX + 3 * (btnWidth + spacing), bandY, btnWidth, btnHeight, "10m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
}

void DisplayGUI::begin() {
    // Arduino_GFX ST7701 RGB Display Bus & Device Setup
    Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
        TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4,
        TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
        TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4,
        1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
        1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */,
        1 /* pclk_active_neg */, 9000000 /* prefer_speed */
    );

    gfx = new Arduino_ST7701_RGBPanel(
        rgbpanel, TFT_WIDTH, TFT_HEIGHT,
        TFT_SCLK, TFT_MOSI, TFT_CS, TFT_BLK
    );

    gfx->begin();
    gfx->fillScreen(COLOR_BG);

    // Initial Full Screen UI Render
    drawHeader(false);
    drawFrequency(14200000);
    drawStepSize(1000);
    drawModeButtons(MODE_USB);
    drawBandButtons(14200000);
}

void DisplayGUI::update(uint32_t currentFreq, RadioMode currentMode, uint32_t stepSize, bool connected) {
    if (!gfx) return;

    if (connected != lastConnected) {
        lastConnected = connected;
        drawHeader(connected);
    }

    if (currentFreq != lastFreq) {
        drawFrequency(currentFreq);
        drawBandButtons(currentFreq);
        lastFreq = currentFreq;
    }

    if (currentMode != lastMode) {
        drawModeButtons(currentMode);
        lastMode = currentMode;
    }

    if (stepSize != lastStepSize) {
        drawStepSize(stepSize);
        lastStepSize = stepSize;
    }
}

void DisplayGUI::drawHeader(bool connected) {
    // Top Bar Header
    gfx->fillRect(0, 0, TFT_WIDTH, 50, COLOR_HEADER);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(2);
    gfx->setCursor(30, 16);
    gfx->print("KENWOOD TS-2000 CAT");

    // CAT Connection Indicator Dot
    uint16_t statusColor = connected ? COLOR_GREEN : COLOR_RED;
    gfx->fillCircle(430, 25, 8, statusColor);
    gfx->drawCircle(430, 25, 10, COLOR_TEXT);
}

String DisplayGUI::formatFrequency(uint32_t freqHz) {
    // Convert 14200000 -> 14.200.000 MHz
    uint32_t mhz = freqHz / 1000000;
    uint32_t khz = (freqHz % 1000000) / 1000;
    uint32_t hz = freqHz % 1000;

    char buf[32];
    if (mhz >= 100) {
        snprintf(buf, sizeof(buf), "%3lu.%03lu.%03lu", (unsigned long)mhz, (unsigned long)khz, (unsigned long)hz);
    } else {
        snprintf(buf, sizeof(buf), "%2lu.%03lu.%03lu", (unsigned long)mhz, (unsigned long)khz, (unsigned long)hz);
    }
    return String(buf);
}

void DisplayGUI::drawFrequency(uint32_t freqHz) {
    // Main Display Card for Frequency
    gfx->fillRoundRect(30, 65, 420, 110, 12, COLOR_CARD_BG);
    gfx->drawRoundRect(30, 65, 420, 110, 12, COLOR_BORDER);

    gfx->setTextColor(COLOR_TEXT_MUTED);
    gfx->setTextSize(2);
    gfx->setCursor(45, 78);
    gfx->print("VFO A FREQUENCY (MHz)");

    // Large Frequency Text
    gfx->setTextColor(COLOR_FREQ);
    gfx->setTextSize(4);
    gfx->setCursor(50, 115);
    gfx->print(formatFrequency(freqHz));
}

void DisplayGUI::drawStepSize(uint32_t stepSize) {
    // Tuning Step Card
    gfx->fillRoundRect(30, 185, 420, 70, 10, COLOR_CARD_BG);
    gfx->drawRoundRect(30, 185, 420, 70, 10, COLOR_BORDER);

    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(2);
    gfx->setCursor(45, 210);
    gfx->print("Tuning Step: ");

    gfx->setTextColor(COLOR_ACCENT);
    gfx->setTextSize(3);
    if (stepSize >= 1000) {
        gfx->print(stepSize / 1000);
        gfx->print(" kHz");
    } else {
        gfx->print(stepSize);
        gfx->print(" Hz");
    }
}

void DisplayGUI::drawButton(const TouchButton& btn) {
    uint16_t bgColor = btn.isSelected ? btn.activeColor : btn.color;
    gfx->fillRoundRect(btn.x, btn.y, btn.w, btn.h, 8, bgColor);
    gfx->drawRoundRect(btn.x, btn.y, btn.w, btn.h, 8, COLOR_BORDER);

    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(2);

    // Center text in button
    int16_t textLen = strlen(btn.label);
    int16_t textX = btn.x + (btn.w - (textLen * 12)) / 2;
    int16_t textY = btn.y + (btn.h - 16) / 2;

    gfx->setCursor(textX, textY);
    gfx->print(btn.label);
}

void DisplayGUI::drawModeButtons(RadioMode currentMode) {
    modeButtons[0].isSelected = (currentMode == MODE_LSB);
    modeButtons[1].isSelected = (currentMode == MODE_USB);
    modeButtons[2].isSelected = (currentMode == MODE_AM);
    modeButtons[3].isSelected = (currentMode == MODE_FM);

    for (int i = 0; i < 4; i++) {
        drawButton(modeButtons[i]);
    }
}

void DisplayGUI::drawBandButtons(uint32_t currentFreq) {
    for (int i = 0; i < NUM_BANDS; i++) {
        bool inBand = (currentFreq >= BANDS[i].minFreqHz && currentFreq <= BANDS[i].maxFreqHz);
        bandButtons[i].isSelected = inBand;
        drawButton(bandButtons[i]);
    }
}

bool DisplayGUI::isPointInButton(int16_t px, int16_t py, const TouchButton& btn) {
    return (px >= btn.x && px <= (btn.x + btn.w) && py >= btn.y && py <= (btn.y + btn.h));
}

bool DisplayGUI::checkTouch(int16_t touchX, int16_t touchY, RadioMode& outNewMode, uint32_t& outNewFreq) {
    // Check Mode Buttons
    if (isPointInButton(touchX, touchY, modeButtons[0])) { outNewMode = MODE_LSB; return true; }
    if (isPointInButton(touchX, touchY, modeButtons[1])) { outNewMode = MODE_USB; return true; }
    if (isPointInButton(touchX, touchY, modeButtons[2])) { outNewMode = MODE_AM; return true; }
    if (isPointInButton(touchX, touchY, modeButtons[3])) { outNewMode = MODE_FM; return true; }

    // Check Band Buttons
    if (isPointInButton(touchX, touchY, bandButtons[0])) { outNewFreq = BANDS[0].defaultFreqHz; outNewMode = BANDS[0].defaultMode; return true; }
    if (isPointInButton(touchX, touchY, bandButtons[1])) { outNewFreq = BANDS[1].defaultFreqHz; outNewMode = BANDS[1].defaultMode; return true; }
    if (isPointInButton(touchX, touchY, bandButtons[2])) { outNewFreq = BANDS[2].defaultFreqHz; outNewMode = BANDS[2].defaultMode; return true; }
    if (isPointInButton(touchX, touchY, bandButtons[3])) { outNewFreq = BANDS[3].defaultFreqHz; outNewMode = BANDS[3].defaultMode; return true; }

    return false;
}
