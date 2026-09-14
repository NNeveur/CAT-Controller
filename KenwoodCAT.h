#ifndef KENWOOD_CAT_H
#define KENWOOD_CAT_H

#include <Arduino.h>
#include "Config.h"

class KenwoodCAT {
public:
    KenwoodCAT();

    // Initialize hardware serial for CAT communication
    void begin(HardwareSerial& serialPort, uint32_t baudRate, int rxPin, int txPin);

    // Process incoming serial data (call in main loop)
    void update();

    // CAT Commands
    void setFrequency(uint32_t freqHz);
    void requestFrequency();

    void setMode(RadioMode mode);
    void requestMode();

    void requestStatus(); // Requests IF; status string

    // Getters for cached state
    uint32_t getFrequency() const { return currentFrequency; }
    RadioMode getMode() const { return currentMode; }
    bool isConnected() const { return connected; }

    // Callbacks for asynchronous state updates
    typedef void (*FreqChangeCallback)(uint32_t freq);
    typedef void (*ModeChangeCallback)(RadioMode mode);

    void onFrequencyChange(FreqChangeCallback cb) { freqCallback = cb; }
    void onModeChange(ModeChangeCallback cb) { modeCallback = cb; }

private:
    HardwareSerial* catSerial;
    uint32_t currentFrequency;
    RadioMode currentMode;
    bool connected;
    uint32_t lastResponseTime;

    String rxBuffer;

    FreqChangeCallback freqCallback;
    ModeChangeCallback modeCallback;

    void sendCommand(const String& cmd);
    void parseCommand(const String& cmd);
};

#endif // KENWOOD_CAT_H
