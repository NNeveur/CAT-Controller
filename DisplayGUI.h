#ifndef DISPLAY_GUI_H
#define DISPLAY_GUI_H

/**
 * @file DisplayGUI.h
 * @brief Gestionnaire de l'interface graphique utilisateur (GUI) et du panneau tactile.
 *
 * Cette classe orchestre l'affichage sur le panneau IPS ST7701 480x480 via Arduino_GFX,
 * le dessin des cartes de fréquence, des pas de réglage, des boutons tactiles
 * de modes et de bandes, ainsi que l'écran de configuration du port série CAT.
 */

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include "Config.h"

/**
 * @brief Structure décrivant un bouton tactile de l'interface graphique.
 */
struct TouchButton {
    int16_t x, y, w, h;    ///< Coordonnées et dimensions (X, Y, Largeur, Hauteur)
    const char* label;     ///< Texte affiché sur le bouton
    bool isSelected;       ///< État sélectionné (actif / inactif)
    uint16_t color;        ///< Couleur de fond à l'état inactif (RGB565)
    uint16_t activeColor;  ///< Couleur de fond à l'état actif (RGB565)
};

class DisplayGUI {
public:
    DisplayGUI();

    /**
     * @brief Initialise le panneau d'affichage ST7701, le bus RGB et le rétroéclairage.
     */
    void begin();

    /**
     * @brief Met à jour dynamiquement l'écran si l'un des paramètres a changé (évite le scintillement).
     */
    void update(uint32_t currentFreq, RadioMode currentMode, uint32_t stepSize, bool connected, uint32_t currentBaud);

    /**
     * @brief Vérifie si un appui tactile touche un bouton actif sur l'écran en cours.
     * @param touchX Coordonnée X de l'appui tactile
     * @param touchY Coordonnée Y de l'appui tactile
     * @param outNewMode Mode mis à jour si un bouton Mode est pressé
     * @param outNewFreq Fréquence mise à jour si un bouton Bande est pressé
     * @param outNewBaud Vitesse mise à jour si un bouton Baud Rate est pressé
     * @return true si une action tactile a été traitée, false sinon
     */
    bool checkTouch(int16_t touchX, int16_t touchY, RadioMode& outNewMode, uint32_t& outNewFreq, uint32_t& outNewBaud);

    /**
     * @brief Obtenir l'écran actif (Main ou Config).
     */
    AppScreen getScreen() const { return currentScreen; }

private:
    Arduino_GFX* gfx;           ///< Pointeur vers l'objet d'affichage Arduino_GFX

    AppScreen currentScreen;    ///< Écran actuellement affiché

    // Variables de cache d'état pour le rafraîchissement ciblé (anti-scintillement)
    uint32_t lastFreq;
    RadioMode lastMode;
    uint32_t lastStepSize;
    bool lastConnected;
    uint32_t lastBaud;

    // Boutons de l'écran principal
    TouchButton cfgHeaderBtn;   ///< Bouton CFG dans la barre de titre
    TouchButton modeButtons[4]; ///< Boutons de mode : LSB, USB, AM, FM
    TouchButton bandButtons[4]; ///< Boutons de bande : 80M, 40M, 20M, 10M

    // Boutons de l'écran de configuration
    TouchButton backBtn;        ///< Bouton de retour à l'écran principal
    TouchButton baudButtons[5]; ///< Boutons de vitesse : 4800, 9600, 19200, 38400, 57600

    void initButtons();
    void drawHeader(bool connected);
    void drawMainScreen(uint32_t currentFreq, RadioMode currentMode, uint32_t stepSize, bool connected);
    void drawConfigScreen(uint32_t currentBaud);
    void drawFrequency(uint32_t freqHz);
    void drawStepSize(uint32_t stepSize);
    void drawModeButtons(RadioMode currentMode);
    void drawBandButtons(uint32_t currentFreq);
    void drawBaudButtons(uint32_t currentBaud);
    void drawButton(const TouchButton& btn);
    bool isPointInButton(int16_t px, int16_t py, const TouchButton& btn);

    /**
     * @brief Formate une fréquence en Hertz sous la forme "14.200.000" ou "7.100.000".
     */
    String formatFrequency(uint32_t freqHz);
};

#endif // DISPLAY_GUI_H
