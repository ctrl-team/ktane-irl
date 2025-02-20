#ifndef BUS_CONTROLLER_H
#define BUS_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>

#define START_ADDRESS 0x08 // we have to avoid reserved addresses
#define END_ADDRESS 0x18

enum Module_state {
  PLAYING = 0x01,
  STREAK = 0x02,
  SOLVED = 0x03,
  PAUSED = 0x04,
  NOT_STARTED = 0x05,
  STATE_UNKNOWN = 0xF
};

enum Module_type {
  DEBUG_MODULE = 0x01,
  BUTTON_MODULE = 0x2,
  MODULE_UNKNOWN = 0xF
};

struct Module {
  uint8_t id;
  Module_type type;
  Module_state state;
  bool active;
};

extern Module modules[END_ADDRESS];

uint8_t receive_byte(uint8_t target_address);
bool send_packet(uint8_t target_address, uint8_t command);
bool send_packet(uint8_t target_address, uint8_t command, uint16_t data);
void broadcast_packet(uint8_t command, uint16_t data);
Module_type who_are_you(uint8_t target_address);
Module_state get_state(uint8_t target_address);
void refresh_states();
void initialize_devices();

#endif
