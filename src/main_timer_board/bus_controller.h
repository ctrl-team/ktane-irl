#ifndef BUS_CONTROLLER_H
#define BUS_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>

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
  int strikes;

  ModuleType whoAreYou(uint8_t targetAddress);
  ModuleState getState(uint8_t targetAddress);
  void refreshStates();
  void initializeDevices();
  void updateState(ModuleState state);

  void begin();

private:
  bool checkAddressAvailability(uint8_t address);
  bool sendPacket(uint8_t targetAddress, uint8_t command);
  bool sendPacket(uint8_t targetAddress, uint8_t command, uint16_t data);
  void broadcastPacket(uint8_t command, uint16_t data);
  uint8_t receiveByte(uint8_t targetAddress);
};

#endif
