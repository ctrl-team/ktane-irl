#include "bus_controller.h"

Module modules[END_ADDRESS];

uint8_t receiveByte(uint8_t target_address) {
  Wire.requestFrom(target_address, 1);
  
  unsigned long start_time = millis();
  while (!Wire.available()) {
    // 80ms timeout
    if (millis() - start_time > 80) {
      Serial.print("Timeout waiting for response from 0x");
      Serial.println(target_address, HEX);
      return 0xF;
    }
    delay(1); // allow CPU time for other tasks
  }

  return Wire.read();
}

bool sendPacket(uint8_t target_address, uint8_t command) {
  Wire.beginTransmission(target_address);
  Wire.write(command);

  // non-zero means error
  if (Wire.endTransmission() != 0) {
    Serial.print("Failed to send 0x");
    Serial.print(command, HEX);
    Serial.print(" request to address 0x");
    Serial.println(target_address, HEX);
    return false;
  }

  return true;
}

bool sendPacket(uint8_t target_address, uint8_t command, uint16_t data) {
  Wire.beginTransmission(target_address);
  Wire.write(command);
  Wire.write((uint8_t)(data >> 8)); // high byte
  Wire.write((uint8_t)(data & 0xFF)); // low byte

  // non-zero means error
  if (Wire.endTransmission() != 0) {
    Serial.print("Failed to send 0x");
    Serial.print(command, HEX);
    Serial.print(" with data 0x");
    Serial.print(data, HEX);
    Serial.print(" to address 0x");
    Serial.println(target_address, HEX);
    return false;
  }

  return true;
}

void broadcastPacket(uint8_t command, uint16_t data) {
  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    sendPacket(module, command, data);
  }
}

ModuleType whoAreYou(uint8_t target_address) {
  if (sendPacket(target_address, 0x04))
    return static_cast<ModuleType>(receiveByte(target_address));
  return MODULE_UNKNOWN;
}

ModuleState getState(uint8_t target_address) {
  if (sendPacket(target_address, 0x02))
    return static_cast<ModuleState>(receiveByte(target_address));
  return STATE_UNKNOWN;
}

void refreshStates() {
  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    if (!modules[module].active) continue;

    ModuleState module_state = getState(modules[module].id);
    if (module_state == STATE_UNKNOWN) continue;

    modules[module].state = module_state;

    Serial.print("[rs] Address 0x");
    Serial.print(modules[module].id, HEX);
    Serial.print(" state is 0x");
    Serial.println(modules[module].state, HEX);
  }
}

void initializeDevices() {
  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    Serial.print("[id] Scanning address 0x");
    Serial.println(module, HEX);

    modules[module].id = module;
    modules[module].type = MODULE_UNKNOWN;
    modules[module].active = false;

    ModuleType module_type = whoAreYou(module);
    if (module_type == MODULE_UNKNOWN) continue;

    modules[module].type = module_type;
    modules[module].active = true;

    ModuleState module_state = getState(module);
    if (module_state == STATE_UNKNOWN) continue;

    modules[module].state = module_state;

    Serial.print("[id] Address 0x");
    Serial.print(module, HEX);
    Serial.print(" identified itself as type 0x");
    Serial.println(module_type, HEX);
  }

  refreshStates();
}