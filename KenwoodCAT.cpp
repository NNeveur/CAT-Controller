#include "KenwoodCAT.h"

KenwoodCAT::KenwoodCAT()
    : catSerial(nullptr), currentFrequency(14200000), currentMode(MODE_USB),
      connected(false), lastResponseTime(0), freqCallback(nullptr), modeCallback(nullptr) {
}

void KenwoodCAT::begin(HardwareSerial& serialPort, uint32_t baudRate, int rxPin, int txPin) {
    catSerial = &serialPort;
    catSerial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    rxBuffer.reserve(64);

    // Initial CAT query
    requestFrequency();
    requestMode();
}

void KenwoodCAT::update() {
    if (!catSerial) return;

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

    // Connection timeout check (5 seconds)
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
    // Format 11-digit zero-padded frequency string
    char buf[16];
    snprintf(buf, sizeof(buf), "FA%011lu", (unsigned long)freqHz);
    sendCommand(String(buf));
}

void KenwoodCAT::requestFrequency() {
    sendCommand("FA");
}

void KenwoodCAT::setMode(RadioMode mode) {
    currentMode = mode;
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

    // Parse VFO-A Frequency command response: FA00014200000
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
    // Parse Mode command response: MD1 (LSB), MD2 (USB), MD4 (FM), MD5 (AM)
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
    // Parse Information status command response: IF0001420000000000+00000000000013...
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
