/*
  Kenwood TS-2000 CAT Controller with MaTouch ESP32-S3 2.1" ST7701 Display & Rotary Encoder
  ------------------------------------------------------------------------------------------
  Features:
  - High resolution 480x480 UI for ST7701 IPS Display
  - Full RS232 CAT protocol control for Kenwood TS-2000 (Frequency & Mode)
  - Rotary Encoder tuning with selectable step sizes (10Hz, 100Hz, 1kHz, 10kHz, 100kHz)
  - Touchscreen buttons for Mode switching: LSB, USB, AM, FM
  - Touchscreen buttons for Band switching: 80m, 40m, 20m, 10m
  - Configuration Screen for CAT Serial Baud Rate selection (4800 to 57600 baud) with NVS persistence
  - Real-time status update & CAT connection monitoring
*/

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  #if __has_include(<sdkconfig.h>)
    #include <sdkconfig.h>
  #endif
#endif

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include "Config.h"
#include "KenwoodCAT.h"
#include "RotaryEncoderDriver.h"
#include "DisplayGUI.h"

// Hardware modules
KenwoodCAT radio;
RotaryEncoderDriver encoder;
DisplayGUI gui;
Preferences preferences;

// Poll timer for CAT status updates
unsigned long lastCatPoll = 0;
const unsigned long CAT_POLL_INTERVAL = 1000; // Poll radio every 1s

// I2C Touch Controller Address (CST816D / GT911)
#define TOUCH_I2C_ADDR 0x15

void setupTouch() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
}

bool readTouch(int16_t &touchX, int16_t &touchY) {
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    Wire.write(0x02); // Touch data start register
    if (Wire.endTransmission() != 0) return false;

    if (Wire.requestFrom((uint8_t)TOUCH_I2C_ADDR, (uint8_t)6) == 6) {
        uint8_t touchPoints = Wire.read() & 0x0F;
        if (touchPoints > 0) {
            uint8_t xHigh = Wire.read() & 0x0F;
            uint8_t xLow = Wire.read();
            uint8_t yHigh = Wire.read() & 0x0F;
            uint8_t yLow = Wire.read();
            Wire.read(); // Gestures / flags

            touchX = (xHigh << 8) | xLow;
            touchY = (yHigh << 8) | yLow;
            return true;
        }
    }
    return false;
}

void onEncoderTurn(int steps, uint32_t stepSize) {
    // Only adjust frequency on main screen
    if (gui.getScreen() != SCREEN_MAIN) return;

    uint32_t currentFreq = radio.getFrequency();
    int64_t newFreq = (int64_t)currentFreq + ((int64_t)steps * stepSize);

    if (newFreq < 30000) newFreq = 30000;
    if (newFreq > 60000000) newFreq = 60000000;

    radio.setFrequency((uint32_t)newFreq);
}

void onEncoderBtnClick() {
    // Handled internally in encoder driver for step size cycling
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Kenwood TS-2000 CAT Controller...");

    // Load saved baud rate preference (or default 57600)
    preferences.begin("ts2000_cat", false);
    uint32_t savedBaud = preferences.getUInt("baud_rate", DEFAULT_CAT_BAUDRATE);

    // Setup CAT Serial connection
    radio.begin(Serial1, savedBaud, CAT_RX_PIN, CAT_TX_PIN);

    // Setup Rotary Encoder
    encoder.begin(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PIN_BTN);
    encoder.onTurn(onEncoderTurn);
    encoder.onButtonClick(onEncoderBtnClick);

    // Setup Touch Screen
    setupTouch();

    // Setup GUI Display
    gui.begin();
}

void loop() {
    // 1. Process CAT Serial messages
    radio.update();

    // 2. Process Rotary Encoder turns/clicks
    encoder.update();

    // 3. Process Touch Screen touches
    int16_t tx = 0, ty = 0;
    static bool wasTouched = false;

    if (readTouch(tx, ty)) {
        if (!wasTouched) { // Single touch trigger
            wasTouched = true;
            RadioMode newMode = radio.getMode();
            uint32_t newFreq = radio.getFrequency();
            uint32_t newBaud = radio.getBaudRate();

            if (gui.checkTouch(tx, ty, newMode, newFreq, newBaud)) {
                if (newMode != radio.getMode()) {
                    radio.setMode(newMode);
                }
                if (newFreq != radio.getFrequency()) {
                    radio.setFrequency(newFreq);
                }
                if (newBaud != radio.getBaudRate()) {
                    radio.setBaudRate(newBaud);
                    preferences.putUInt("baud_rate", newBaud);
                }
            }
        }
    } else {
        wasTouched = false;
    }

    // 4. Periodically poll TS-2000 status
    if (millis() - lastCatPoll > CAT_POLL_INTERVAL) {
        lastCatPoll = millis();
        if (gui.getScreen() == SCREEN_MAIN) {
            radio.requestStatus();
        }
    }

    // 5. Refresh GUI display state
    gui.update(radio.getFrequency(), radio.getMode(), encoder.getStepSize(), radio.isConnected(), radio.getBaudRate());
}
