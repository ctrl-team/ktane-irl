#include "module_configuration.h"

ModuleConfiguration::ModuleConfiguration(): flags(0), ports(0) {
  serial[0] = '\0';
}

void ModuleConfiguration::setFlag(Flag flag) {
  flags |= flag;
}

bool ModuleConfiguration::hasFlag(Flag flag) const {
  return flags & flag;
}

void ModuleConfiguration::setPort(Port port) {
  ports |= port;
}

bool ModuleConfiguration::hasPort(Port port) const {
  return ports & port;
}

void ModuleConfiguration::clear() {
  flags = 0;
  ports = 0;
  serial[0] = '\0';
}

void ModuleConfiguration::randomize(int flagsCount, int portsCount) {
  clear();

  randomSeed(analogRead(A0));

  while (flagsCount-- > 0) {
    Flag randomFlag = static_cast<Flag>(1 << random(0, 11));
    setFlag(randomFlag);
  }

  while (portsCount-- > 0) {
    Port randomPort = static_cast<Port>(1 << random(0, 6));
    setPort(randomPort);
  }

  for (int i = 0; i <= 4; i++) {
    int chr = random(36);
    serial[i] = (chr < 26) ? ('A' + chr) : ('0' + (chr - 26));
  }

  serial[5] = '0' + random(10);
}