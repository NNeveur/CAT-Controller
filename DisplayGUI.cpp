/**
 * @file DisplayGUI.cpp
 * @brief Implémentation du rendu graphique de l'interface utilisateur et de la gestion tactile.
 */

#include "DisplayGUI.h"

// Palette de couleurs (Format 16-bit RGB565)
#define COLOR_BG          0x0000 // Noir
#define COLOR_CARD_BG     0x18E3 // Gris foncé / Bleuté
#define COLOR_HEADER      0x0A2D // Bleu marine profond
#define COLOR_TEXT        0xFFFF // Blanc
#define COLOR_TEXT_MUTED  0x9CD5 // Gris clair
#define COLOR_ACCENT      0x041F // Cyan / Accent Bleu
#define COLOR_FREQ        0x07E0 // Vert vif pour le VFO
#define COLOR_ACTIVE_BTN  0x03E0 // Vert bouton actif
#define COLOR_INACTIVE_BTN 0x2A54 // Ardoise foncée
#define COLOR_BORDER      0x52AA // Bordure argentée
#define COLOR_RED         0xF800 // Rouge (Indicateur déconnecté)
#define COLOR_GREEN       0x07E0 // Vert (Indicateur connecté)

DisplayGUI::DisplayGUI()
    : gfx(nullptr), currentScreen(SCREEN_MAIN), lastFreq(0), lastMode(MODE_CW),
      lastStepSize(0), lastConnected(false), lastBaud(0) {
    initButtons();
}

void DisplayGUI::initButtons() {
    // Bouton CFG situé dans la barre d'en-tête
    cfgHeaderBtn = { 350, 10, 60, 30, "CFG", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };

    // Boutons de mode (4 boutons : LSB, USB, AM, FM) disposés en ligne 1
    const int16_t modeY = 270;
    const int16_t btnWidth = 90;
    const int16_t btnHeight = 50;
    const int16_t spacing = 15;
    const int16_t startX = 35;

    modeButtons[0] = { startX,                             modeY, btnWidth, btnHeight, "LSB", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    modeButtons[1] = { startX + (btnWidth + spacing),     modeY, btnWidth, btnHeight, "USB", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    modeButtons[2] = { startX + 2 * (btnWidth + spacing), modeY, btnWidth, btnHeight, "AM",  false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    modeButtons[3] = { startX + 3 * (btnWidth + spacing), modeY, btnWidth, btnHeight, "FM",  false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };

    // Boutons de bande (4 boutons : 80M, 40M, 20M, 10M) disposés en ligne 2
    const int16_t bandY = 345;
    bandButtons[0] = { startX,                             bandY, btnWidth, btnHeight, "80m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
    bandButtons[1] = { startX + (btnWidth + spacing),     bandY, btnWidth, btnHeight, "40m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
    bandButtons[2] = { startX + 2 * (btnWidth + spacing), bandY, btnWidth, btnHeight, "20m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };
    bandButtons[3] = { startX + 3 * (btnWidth + spacing), bandY, btnWidth, btnHeight, "10m", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };

    // Bouton de retour sur l'écran de configuration
    backBtn = { 35, 400, 410, 50, "< RETOUR ECRAN PRINCIPAL", false, COLOR_INACTIVE_BTN, COLOR_ACCENT };

    // Boutons de débit bauds sur l'écran de configuration
    baudButtons[0] = { 40,  150, 120, 50, "4800",  false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    baudButtons[1] = { 180, 150, 120, 50, "9600",  false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    baudButtons[2] = { 320, 150, 120, 50, "19200", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    baudButtons[3] = { 100, 230, 120, 50, "38400", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
    baudButtons[4] = { 260, 230, 120, 50, "57600", false, COLOR_INACTIVE_BTN, COLOR_ACTIVE_BTN };
}

void DisplayGUI::begin() {
    // Initialisation de l'affichage Arduino_GFX ST7701 RGB avec bus SPI 3-fils pour les registres.
    // st7701_type5_init_operations est la séquence Makerfabs pour l'écran 2.1" 480x480.
    Arduino_DataBus *initBus = new Arduino_SWSPI(
        GFX_NOT_DEFINED /* DC */, TFT_CS,
        TFT_SCLK, TFT_MOSI, GFX_NOT_DEFINED /* MISO */
    );

    Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
        TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
        TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4,
        TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
        TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4,
        1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
        1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */,
        1 /* pclk_active_neg */, 9000000 /* prefer_speed */
    );

    gfx = new Arduino_RGB_Display(
        TFT_WIDTH, TFT_HEIGHT, rgbpanel, 0 /* rotation */, true /* auto_flush */,
        initBus, GFX_NOT_DEFINED /* RST */,
        st7701_type5_init_operations, sizeof(st7701_type5_init_operations)
    );

    gfx->begin();
    gfx->fillScreen(COLOR_BG);

    // Activation du rétroéclairage
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, HIGH);

    drawMainScreen(14200000, MODE_USB, 1000, false);
}

void DisplayGUI::drawMainScreen(uint32_t currentFreq, RadioMode currentMode, uint32_t stepSize, bool connected) {
    gfx->fillScreen(COLOR_BG);
    drawHeader(connected);
    drawFrequency(currentFreq);
    drawStepSize(stepSize);
    drawModeButtons(currentMode);
    drawBandButtons(currentFreq);

    lastFreq = currentFreq;
    lastMode = currentMode;
    lastStepSize = stepSize;
    lastConnected = connected;
}

void DisplayGUI::drawConfigScreen(uint32_t currentBaud) {
    gfx->fillScreen(COLOR_BG);

    // Barre d'en-tête
    gfx->fillRect(0, 0, TFT_WIDTH, 50, COLOR_HEADER);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(2);
    gfx->setCursor(30, 16);
    gfx->print("CAT SETUP CONFIG");

    // Carte de titre
    gfx->fillRoundRect(30, 70, 420, 50, 10, COLOR_CARD_BG);
    gfx->drawRoundRect(30, 70, 420, 50, 10, COLOR_BORDER);
    gfx->setTextColor(COLOR_TEXT_MUTED);
    gfx->setTextSize(2);
    gfx->setCursor(45, 85);
    gfx->print("Select Serial Baud Rate:");

    drawBaudButtons(currentBaud);
    drawButton(backBtn);

    lastBaud = currentBaud;
}

void DisplayGUI::update(uint32_t currentFreq, RadioMode currentMode, uint32_t stepSize, bool connected, uint32_t currentBaud) {
    if (!gfx) return;

    if (currentScreen == SCREEN_MAIN) {
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
    } else if (currentScreen == SCREEN_CONFIG) {
        if (currentBaud != lastBaud) {
            drawBaudButtons(currentBaud);
            lastBaud = currentBaud;
        }
    }
}

void DisplayGUI::drawHeader(bool connected) {
    // Barre supérieure
    gfx->fillRect(0, 0, TFT_WIDTH, 50, COLOR_HEADER);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setTextSize(2);
    gfx->setCursor(30, 16);
    gfx->print("TS-2000 CAT");

    // Bouton CFG dans l'en-tête
    drawButton(cfgHeaderBtn);

    // Témoin lumineux d'état de connexion CAT (Vert = connecté, Rouge = déconnecté)
    uint16_t statusColor = connected ? COLOR_GREEN : COLOR_RED;
    gfx->fillCircle(430, 25, 8, statusColor);
    gfx->drawCircle(430, 25, 10, COLOR_TEXT);
}

String DisplayGUI::formatFrequency(uint32_t freqHz) {
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
    gfx->fillRoundRect(30, 65, 420, 110, 12, COLOR_CARD_BG);
    gfx->drawRoundRect(30, 65, 420, 110, 12, COLOR_BORDER);

    gfx->setTextColor(COLOR_TEXT_MUTED);
    gfx->setTextSize(2);
    gfx->setCursor(45, 78);
    gfx->print("VFO A FREQUENCY (MHz)");

    gfx->setTextColor(COLOR_FREQ);
    gfx->setTextSize(4);
    gfx->setCursor(50, 115);
    gfx->print(formatFrequency(freqHz));
}

void DisplayGUI::drawStepSize(uint32_t stepSize) {
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

void DisplayGUI::drawBaudButtons(uint32_t currentBaud) {
    for (int i = 0; i < NUM_BAUD_RATES; i++) {
        baudButtons[i].isSelected = (currentBaud == CAT_BAUD_RATES[i]);
        drawButton(baudButtons[i]);
    }
}

bool DisplayGUI::isPointInButton(int16_t px, int16_t py, const TouchButton& btn) {
    return (px >= btn.x && px <= (btn.x + btn.w) && py >= btn.y && py <= (btn.y + btn.h));
}

bool DisplayGUI::checkTouch(int16_t touchX, int16_t touchY, RadioMode& outNewMode, uint32_t& outNewFreq, uint32_t& outNewBaud) {
    if (currentScreen == SCREEN_MAIN) {
        // Appui sur le bouton CFG dans l'en-tête
        if (isPointInButton(touchX, touchY, cfgHeaderBtn)) {
            currentScreen = SCREEN_CONFIG;
            drawConfigScreen(outNewBaud);
            return true;
        }

        // Appui sur les boutons de mode
        if (isPointInButton(touchX, touchY, modeButtons[0])) { outNewMode = MODE_LSB; return true; }
        if (isPointInButton(touchX, touchY, modeButtons[1])) { outNewMode = MODE_USB; return true; }
        if (isPointInButton(touchX, touchY, modeButtons[2])) { outNewMode = MODE_AM; return true; }
        if (isPointInButton(touchX, touchY, modeButtons[3])) { outNewMode = MODE_FM; return true; }

        // Appui sur les boutons de bande
        if (isPointInButton(touchX, touchY, bandButtons[0])) { outNewFreq = BANDS[0].defaultFreqHz; outNewMode = BANDS[0].defaultMode; return true; }
        if (isPointInButton(touchX, touchY, bandButtons[1])) { outNewFreq = BANDS[1].defaultFreqHz; outNewMode = BANDS[1].defaultMode; return true; }
        if (isPointInButton(touchX, touchY, bandButtons[2])) { outNewFreq = BANDS[2].defaultFreqHz; outNewMode = BANDS[2].defaultMode; return true; }
        if (isPointInButton(touchX, touchY, bandButtons[3])) { outNewFreq = BANDS[3].defaultFreqHz; outNewMode = BANDS[3].defaultMode; return true; }

    } else if (currentScreen == SCREEN_CONFIG) {
        // Appui sur le bouton Retour
        if (isPointInButton(touchX, touchY, backBtn)) {
            currentScreen = SCREEN_MAIN;
            drawMainScreen(outNewFreq, outNewMode, lastStepSize, lastConnected);
            return true;
        }

        // Appui sur un bouton de vitesse de transmission
        for (int i = 0; i < NUM_BAUD_RATES; i++) {
            if (isPointInButton(touchX, touchY, baudButtons[i])) {
                outNewBaud = CAT_BAUD_RATES[i];
                drawBaudButtons(outNewBaud);
                return true;
            }
        }
    }

    return false;
}
