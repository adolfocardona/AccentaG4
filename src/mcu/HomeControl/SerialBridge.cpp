/*
  SerialBridge.cpp - mcu <=> mpu serial bridge
  Created by Luca Paolini, April 3, 2019.
*/

#include "SerialBridge.h"

SerialBridge::SerialBridge(
    HardwareSerial &serial, long linkSpeed, int statusLed, 
    char enableChar, char disableChar, unsigned long enableGraceMillis, 
    void (*enableHandler)(HardwareSerial serial, char enableChar, boolean justEnabled),
    void (*disableHandler)(HardwareSerial serial, char disableChar, boolean justEnabled),
    void (*readHandler)(HardwareSerial serial, char readChar)
) : serial(serial) {
    this->linkSpeed = linkSpeed;
    this->statusLed = statusLed;
    this->enableChar = enableChar;
    this->disableChar = disableChar;
    this->enableGraceMillis = enableGraceMillis;
    this->enableHandler = enableHandler;
    this->disableHandler = disableHandler;
    this->readHandler = readHandler;
}

void SerialBridge::start() {
    serial.begin(linkSpeed);
}

void SerialBridge::stop() {
    serial.end();
}

boolean SerialBridge::isEnabled() {
    boolean withinGracePeriod = (enableGraceMillis == 0) || (millis() - lastEnabled < enableGraceMillis);
    return enabled && withinGracePeriod;
}

void SerialBridge::read() {
    int c = serial.read();
    if (c == enableChar) {
        boolean wasEnabled = enabled;
        enabled = true;
        lastEnabled = millis();
        blink();
        if (enableHandler) {
            enableHandler(serial, enableChar, wasEnabled == false);
        }
        return;
    }
    if (c == disableChar) {
        boolean wasEnabled = enabled;
        enabled = false;
        if (disableHandler) {
            disableHandler(serial, disableChar, wasEnabled == true);
        }
        return;
    }
    if (isEnabled()) {
        if (readHandler) {
            readHandler(serial, c);
        }
    };
}

void SerialBridge::blink() {
    lastBlink = millis();
    ledState = HIGH;
    digitalWrite(statusLed, ledState);
}

void SerialBridge::resetBlink() {
    unsigned long now = millis();
    unsigned long duration = now - lastBlink;
    if (ledState == HIGH && duration > BLINK_DURATION_MS) {
        ledState = LOW;
        digitalWrite(statusLed, ledState);
        lastBlink = now;
    }
}

void SerialBridge::begin() {
    pinMode(statusLed, OUTPUT);
    digitalWrite(statusLed, HIGH);
    delay(2500);
    start();
    digitalWrite(statusLed, LOW);
}

void SerialBridge::end() {
    stop();
    digitalWrite(statusLed, LOW);
}

void SerialBridge::loop() {
    read();
    resetBlink();
}
