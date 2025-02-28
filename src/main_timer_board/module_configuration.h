#ifndef MODULE_CONFIGURATION_H
#define MODULE_CONFIGURATION_H

#include <Arduino.h>

enum Flag {
  SND = 1 << 0,
  CLR = 1 << 1,
  CAR = 1 << 2,
  IND = 1 << 3,
  FRQ = 1 << 4,
  SIG = 1 << 5,
  NSA = 1 << 6,
  MSA = 1 << 7,
  TRN = 1 << 8,
  BOB = 1 << 9,
  FRK = 1 << 10
};

enum Port {
  DVID     = 1 << 0,
  PARALLEL = 1 << 1,
  PS2      = 1 << 2,
  RJ45     = 1 << 3,
  SERIALP  = 1 << 4,
  RCA      = 1 << 5
};

struct ModuleConfiguration {
  uint16_t flags;
  uint8_t ports;
  uint8_t batteries;
  char serial[6]; // 6 chars

  ModuleConfiguration();

  // flags
  void setFlag(Flag flag);
  bool hasFlag(Flag flag) const;

  // ports
  void setPort(Port port);
  bool hasPort(Port port) const;

  // configuration
  void clear();
  void randomize(int flags, int ports, int batteries);
};

#endif