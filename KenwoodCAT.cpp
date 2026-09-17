/**
 * @file KenwoodCAT.cpp
 * @brief Implémentation du pilote de communication RS-232 CAT pour Kenwood TS-2000.
 */

#include "KenwoodCAT.h"

KenwoodCAT::KenwoodCAT()
    : catSerial(nullptr), currentBaudRate(DEFAULT_CAT_BAUDRATE), catRxPin(CAT_RX_PIN), catTxPin(CAT_TX_PIN),
      currentFrequency(14200000), currentMode(MODE_USB),
      connected(false), lastResponseTime(0), freqCallback(nullptr), modeCallback(nullptr) {
}

void KenwoodCAT::begin(HardwareSerial& serialPort, uint32_t baudRate, int rxPin, int txPin) {
    catSerial = &serialPort;
    currentBaudRate = baudRate;
    catRxPin = rxPin;
    catTxPin = txPin;
    catSerial->begin(currentBaudRate, SERIAL_8N1, catRxPin, catTxPin);
    rxBuffer.reserve(64);

    // Reconstitution de l'état initial par interrogations CAT
    requestFrequency();
    requestMode();
}

void KenwoodCAT::setBaudRate(uint32_t newBaudRate) {
    if (newBaudRate == currentBaudRate) return;
    currentBaudRate = newBaudRate;
    connected = false;
    if (catSerial) {
        catSerial->end();
        catSerial->begin(currentBaudRate, SERIAL_8N1, catRxPin, catTxPin);
    }
    requestFrequency();
    requestMode();
}

void KenwoodCAT::update() {
    if (!catSerial) return;

    // Lecture des caractères série jusqu'au délimiteur ';'
    while (catSerial->available()) {
        char c = (char)catSerial->read();
        if (c == ';') {
            rxBuffer.trim();
            if (rxBuffer.length() > 0) {
                parseCommand(rxBuffer);
            }
            rxBuffer = "";
        } else if (c != '\r' && c != '\n') {
            rxBuffer += c;
        }
    }

    // Vérification du délai d'expiration de la connexion (5 secondes)
    if (millis() - lastResponseTime > 5000) {
        connected = false;
    }
}

void KenwoodCAT::sendCommand(const String& cmd) {
    if (catSerial) {
        catSerial->print(cmd);
        catSerial->print(';');
    }
}

void KenwoodCAT::setFrequency(uint32_t freqHz) {
    if (freqHz == currentFrequency) return;

    currentFrequency = freqHz;
    // Formatage de la commande FA sur 11 chiffres complétés par des zéros (ex: FA00014200000)
    char buf[16];
    snprintf(buf, sizeof(buf), "FA%011lu", (unsigned long)freqHz);
    sendCommand(String(buf));
}

void KenwoodCAT::requestFrequency() {
    sendCommand("FA");
}

void KenwoodCAT::setMode(RadioMode mode) {
    currentMode = mode;
    // Formatage de la commande MD avec le numéro de mode (ex: MD2 pour USB)
    char buf[8];
    snprintf(buf, sizeof(buf), "MD%d", (int)mode);
    sendCommand(String(buf));
}

void KenwoodCAT::requestMode() {
    sendCommand("MD");
}

void KenwoodCAT::requestStatus() {
    sendCommand("IF");
}

void KenwoodCAT::parseCommand(const String& cmd) {
    lastResponseTime = millis();
    connected = true;

    // Analyse de la réponse à la commande de fréquence VFO-A : FA00014200000
    if (cmd.startsWith("FA") && cmd.length() >= 13) {
        String freqStr = cmd.substring(2, 13);
        uint32_t parsedFreq = (uint32_t)freqStr.substring(0, 11).toInt();
        if (parsedFreq > 0 && parsedFreq != currentFrequency) {
            currentFrequency = parsedFreq;
            if (freqCallback) {
                freqCallback(currentFrequency);
            }
        }
    }
    // Analyse de la réponse à la commande de mode : MD1 (LSB), MD2 (USB), MD4 (FM), MD5 (AM)
    else if (cmd.startsWith("MD") && cmd.length() >= 3) {
        int modeVal = cmd.substring(2, 3).toInt();
        RadioMode parsedMode = static_cast<RadioMode>(modeVal);
        if (parsedMode != currentMode) {
            currentMode = parsedMode;
            if (modeCallback) {
                modeCallback(currentMode);
            }
        }
    }
    // Analyse de la réponse à la commande d'information de statut IF:
    // Ex: IF0001420000000000+00000000000013...
    else if (cmd.startsWith("IF") && cmd.length() >= 31) {
        String freqStr = cmd.substring(2, 13);
        uint32_t parsedFreq = (uint32_t)freqStr.toInt();
        int modeVal = cmd.substring(30, 31).toInt();

        if (parsedFreq > 0 && parsedFreq != currentFrequency) {
            currentFrequency = parsedFreq;
            if (freqCallback) freqCallback(currentFrequency);
        }

        RadioMode parsedMode = static_cast<RadioMode>(modeVal);
        if (parsedMode != currentMode) {
            currentMode = parsedMode;
            if (modeCallback) modeCallback(currentMode);
        }
    }
}
