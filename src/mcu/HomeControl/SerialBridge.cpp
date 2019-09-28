/*
  SerialBridge.cpp - mcu <=> mpu serial bridge
  Created by Luca Paolini, April 3, 2019.
*/

#include "SerialBridge.h"

SerialBridge::SerialBridge(
    HardwareSerial &serial, long linkSpeed, int statusLed, 
    char heartbeatChar, unsigned long heartbeatWindow, 
    void (*heartbeatHandler)(HardwareSerial &serial)
) : serial(serial) {
    this->linkSpeed = linkSpeed;
    this->statusLed = statusLed;
    this->heartbeatChar = heartbeatChar;
    this->heartbeatWindow = heartbeatWindow;
    this->heartbeatHandler = heartbeatHandler;
}

void SerialBridge::start() { serial.begin(linkSpeed); }

void SerialBridge::stop() { serial.end(); }

bool SerialBridge::isActive() {
    boolean withinHeartbeatWindow = millis() - heartbeatLastMillis < heartbeatWindow;
    return heartbeatEnabled && withinHeartbeatWindow;
}

int SerialBridge::heartbeatAwareRead() {
    int c = serial.read();
    if (c == heartbeatChar) {
        heartbeatEnabled = true;
        heartbeatLastMillis = millis();
        if (heartbeatHandler) {
            heartbeatHandler(serial);
        }
        return -1;
    } else {
        return c;
    }
}

void SerialBridge::blink() {
    unsigned long currentMillis = millis();
    if (currentMillis > nextBlink) {
        nextBlink = currentMillis + BRIDGE_UP_BLINK_RATE_MS;
        ledState = isActive() && !ledState;
        digitalWrite(statusLed, ledState);
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
    blink();
}
