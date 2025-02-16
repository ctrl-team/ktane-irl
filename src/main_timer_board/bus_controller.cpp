#include "bus_controller.h"

Module modules[END_ADDRESS];

uint8_t receive_byte(uint8_t target_address) {
  Wire.requestFrom(target_address, 1);
  
  unsigned long start_time = millis();
  while (!Wire.available()) {
    // 80ms timeout
    if (millis() - start_time > 80) {
      Serial.print("Timeout waiting for response from 0x");
      Serial.println(target_address, HEX);
      return 0xFF;
    }
    delay(1); // allow CPU time for other tasks
  }

  return Wire.read();
}

bool send_request(uint8_t target_address, uint8_t command) {
  Wire.beginTransmission(target_address);
  Wire.write(command);

  // non-zero means error
  if (Wire.endTransmission() != 0) {
    Serial.print("Failed to send 0x");
    Serial.print(command, HEX);
    Serial.print(" request to address 0x");
    Serial.println(target_address, HEX);
    return false;
  } else
    return true;
}

bool send_request(uint8_t target_address, uint8_t command, uint16_t data) {
  Wire.beginTransmission(target_address);
  Wire.write(command);
  Wire.write((uint8_t)(data >> 8)); // high byte
  Wire.write((uint8_t)(data & 0xFF)); // low byte

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

Module_type who_are_you(uint8_t target_address) {
  if (send_request(target_address, 0x04))
    return static_cast<Module_type>(receive_byte(target_address));
  else
    return MODULE_UNKNOWN;
}

Module_state get_state(uint8_t target_address) {
  if (send_request(target_address, 0x02))
    return static_cast<Module_state>(receive_byte(target_address));
  else
    return STATE_UNKOWN;
}

void refresh_states() {
  for (Module module : modules) {
    if (!module.active) continue;

    Module_state module_state = get_state(module.id);
    if (module_state == STATE_UNKOWN) continue;

    module.state = module_state;

    Serial.print("[rs] Address 0x");
    Serial.print(module.id, HEX);
    Serial.print(" state is 0x");
    Serial.println(module.state, HEX);
  }
}

void initialize_devices() {
  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    Serial.print("[id] Scanning address 0x");
    Serial.println(module, HEX);

    modules[module].id = module;
    modules[module].type = MODULE_UNKNOWN;
    modules[module].active = false;

    Module_type module_type = who_are_you(module);
    if (module_type == MODULE_UNKNOWN) continue;

    modules[module].type = module_type;
    modules[module].active = true;

    Module_state module_state = get_state(module);
    if (module_state == STATE_UNKOWN) continue;

    modules[module].state = module_state;

    Serial.print("[id] Address 0x");
    Serial.print(module, HEX);
    Serial.print(" identified itself as type 0x");
    Serial.println(module_type, HEX);
  }

  refresh_states();
}