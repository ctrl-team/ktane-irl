#ifndef BUS_RECEIVER_H
#define BUS_RECEIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "module_configuration.h"

#define SDA_PIN 0
#define SCL_PIN 1

#define START_ADDRESS 0x08 // we have to avoid reserved addresses
#define END_ADDRESS 0x18

enum ModuleState {
  PLAYING = 0x1,
  STRIKE = 0x2,
  SOLVED = 0x3,
  PAUSED = 0x4,
  NOT_STARTED = 0x5,
  STATE_UNKNOWN = 0xF
};

enum ModuleType {
  DEBUG_MODULE = 0x01,
  BUTTON_MODULE = 0x2,
  MODULE_UNKNOWN = 0xF
};

class BusReceiver {
  public:
    ModuleState state;
    ModuleType type;
    ModuleConfiguration config;
    bool justStarted;

    uint8_t moduleAddress;
    uint16_t timer;

    void begin(ModuleType type);

  private:
    static BusReceiver* instancePointer;
    static BusReceiver* getInstance();

    uint8_t requestedCommand = 0xF;
    uint16_t receivedData = 0x0;

    void receiveCallback(int numBytes);
    void requestCallback();

    static void staticReceiveCallback(int numBytes);
    static void staticRequestCallback();

    bool checkAddressAvailability(uint8_t address);
    uint8_t requestAvailableAddress();
};

#endif