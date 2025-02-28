#ifndef BUS_CONTROLLER_H
#define BUS_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>
#include "module_configuration.h"

#define START_ADDRESS 0x08 // we have to avoid reserved addresses
#define END_ADDRESS 0x18

#define SDA_PIN 0
#define SCL_PIN 1

enum ModuleState {
  PLAYING = 0x01,
  STRIKE = 0x02,
  SOLVED = 0x03,
  PAUSED = 0x04,
  NOT_STARTED = 0x05,
  STATE_UNKNOWN = 0xF
};

enum ModuleType {
  DEBUG_MODULE = 0x01,
  BUTTON_MODULE = 0x2,
  MODULE_UNKNOWN = 0xF
};

struct Module {
  uint8_t id;
  ModuleType type;
  ModuleState state;
  bool active;
};

class BusController {
public:
  int moduleCount;
  Module modules[END_ADDRESS];
  ModuleState state;
  ModuleConfiguration config;
  bool justStriked;
  bool justSolved;
  int strikes;
  int solved;

  ModuleType whoAreYou(uint8_t targetAddress);
  ModuleState getState(uint8_t targetAddress);

  void refreshStates();
  void initializeDevices();

  void updateState(ModuleState state);
  void updateTimer(uint16_t timer);
  void sendConfiguration();

  void begin();

private:
  bool checkAddressAvailability(uint8_t address);
  bool sendPacket(uint8_t targetAddress, uint8_t command);
  bool sendPacket(uint8_t targetAddress, uint8_t command, uint16_t data);
  bool sendPacket(uint8_t targetAddress, uint8_t command, char* data, size_t length);
  void broadcastPacket(uint8_t command, uint16_t data);
  void broadcastPacket(uint8_t command, char* data, size_t length);
  uint8_t receiveByte(uint8_t targetAddress);
};

#endif
