#ifndef BUS_CONTROLLER_H
#define BUS_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>

#define START_ADDRESS 0x08 // we have to avoid reserved addresses
#define END_ADDRESS 0x18

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

extern Module modules[END_ADDRESS];

uint8_t receiveByte(uint8_t target_address);
bool sendPacket(uint8_t target_address, uint8_t command);
bool sendPacket(uint8_t target_address, uint8_t command, uint16_t data);
void broadcastPacket(uint8_t command, uint16_t data);
ModuleType whoAreYou(uint8_t target_address);
ModuleState getState(uint8_t target_address);
void refreshStates();
void initializeDevices();

#endif
