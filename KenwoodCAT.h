#ifndef KENWOOD_CAT_H
#define KENWOOD_CAT_H

/**
 * @file KenwoodCAT.h
 * @brief Pilote de communication RS-232 selon le protocole CAT Kenwood TS-2000.
 *
 * Cette classe gère l'envoi des commandes ASCII Kenwood (ex: FA, MD, IF), le découpage
 * des réponses terminées par un point-virgule (';'), ainsi que le suivi de l'état
 * de la connexion série.
 */

#include <Arduino.h>
#include "Config.h"

class KenwoodCAT {
public:
    KenwoodCAT();

    /**
     * @brief Initialise le port série matériel pour la communication CAT.
     * @param serialPort Référence vers le port série Hardware (ex: Serial1)
     * @param baudRate Vitesse en bauds (ex: 57600)
     * @param rxPin Broche RX ESP32
     * @param txPin Broche TX ESP32
     */
    void begin(HardwareSerial& serialPort, uint32_t baudRate, int rxPin, int txPin);

    /**
     * @brief Modifie la vitesse de communication de la liaison série CAT.
     * @param newBaudRate Nouvelle vitesse en bauds (4800 à 57600)
     */
    void setBaudRate(uint32_t newBaudRate);

    /**
     * @brief Obtenir la vitesse de communication actuelle.
     * @return Vitesse en bauds
     */
    uint32_t getBaudRate() const { return currentBaudRate; }

    /**
     * @brief Traite les données reçues sur le port série CAT. Doit être appelé régulièrement dans loop().
     */
    void update();

    /**
     * @brief Définit la fréquence du VFO A sur le TS-2000.
     * @param freqHz Fréquence en Hertz (ex: 14200000 pour 14.200 MHz)
     */
    void setFrequency(uint32_t freqHz);

    /**
     * @brief Demande la fréquence actuelle du VFO A (commande FA;).
     */
    void requestFrequency();

    /**
     * @brief Définit le mode de fonctionnement (commande MD<n>;).
     * @param mode Mode souhaité (LSB, USB, CW, FM, AM, FSK)
     */
    void setMode(RadioMode mode);

    /**
     * @brief Demande le mode actuel du TS-2000 (commande MD;).
     */
    void requestMode();

    /**
     * @brief Demande le statut général du transceiver (commande IF;).
     */
    void requestStatus();

    /**
     * @brief Obtenir la dernière fréquence connue.
     * @return Fréquence en Hertz
     */
    uint32_t getFrequency() const { return currentFrequency; }

    /**
     * @brief Obtenir le dernier mode connu.
     * @return Mode de fonctionnement RadioMode
     */
    RadioMode getMode() const { return currentMode; }

    /**
     * @brief Indique si la liaison CAT est active (réponse reçue dans les 5 dernières secondes).
     * @return true si connecté, false sinon
     */
    bool isConnected() const { return connected; }

    // Types de fonctions de rappel (callbacks) pour les événements asymptotiques
    typedef void (*FreqChangeCallback)(uint32_t freq);
    typedef void (*ModeChangeCallback)(RadioMode mode);

    /**
     * @brief Enregistre une fonction de rappel appelée lors du changement de fréquence.
     */
    void onFrequencyChange(FreqChangeCallback cb) { freqCallback = cb; }

    /**
     * @brief Enregistre une fonction de rappel appelée lors du changement de mode.
     */
    void onModeChange(ModeChangeCallback cb) { modeCallback = cb; }

private:
    HardwareSerial* catSerial;   ///< Pointeur vers l'instance HardwareSerial utilisée
    uint32_t currentBaudRate;    ///< Vitesse de transmission actuelle
    int catRxPin;               ///< Numéro de broche RX
    int catTxPin;               ///< Numéro de broche TX
    uint32_t currentFrequency;   ///< Cache de la fréquence courante
    RadioMode currentMode;       ///< Cache du mode courant
    bool connected;              ///< État de la connexion CAT
    uint32_t lastResponseTime;   ///< Horodatage (ms) de la dernière réponse CAT reçue

    String rxBuffer;             ///< Tampon de réception des caractères série

    FreqChangeCallback freqCallback;
    ModeChangeCallback modeCallback;

    /**
     * @brief Envoie une commande ASCII terminée par un point-virgule ';'.
     * @param cmd Commande sans le point-virgule final
     */
    void sendCommand(const String& cmd);

    /**
     * @brief Analyse une chaîne de réponse CAT complète.
     * @param cmd Réponse sans le point-virgule
     */
    void parseCommand(const String& cmd);
};

#endif // KENWOOD_CAT_H
