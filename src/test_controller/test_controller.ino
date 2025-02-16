#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 0
#define SCL_PIN 1

#define OFFSET_ADDRESS 0x08 // we have to avoid reserved addresses
#define MAX_MODULES 0x0F

enum Module_type {
  DEBUG_MODULE = 0x01,
  UNKNOWN = 0xFF
};

struct Module {
  uint8_t id;
  Module_type type;
  bool state;
  bool active;
};

Module modules[MAX_MODULES];

uint8_t receive_byte(uint8_t target_address) {
  Wire.requestFrom(target_address + OFFSET_ADDRESS, 1);
  
  unsigned long start_time = millis();
  while (!Wire.available()) {
    // 80ms timeout
    if (millis() - start_time > 80) {
      Serial.print("Timeout waiting for response from 0x");
      Serial.println(target_address + OFFSET_ADDRESS, HEX);
      return 0xFF;
    }
    delay(1); // allow CPU time for other tasks
  }

  return Wire.read();
}

bool send_request(uint8_t target_address, uint8_t command) {
  Wire.beginTransmission(target_address + OFFSET_ADDRESS);
  Wire.write(command);

  // non-zero means error
  if (Wire.endTransmission() != 0) {
    Serial.print("Failed to send request to address 0x");
    Serial.println(target_address + OFFSET_ADDRESS, HEX);
    return false;
  } else
    return true;
}

Module_type who_are_you(uint8_t target_address) {
  if (send_request(target_address, 0x04))
    return static_cast<Module_type>(receive_byte(target_address));
  else
    return UNKNOWN;
}

void initialize_devices() {
  for (int module = 0; module < MAX_MODULES; module++) {
    Serial.print("Scanning module 0x");
    Serial.print(module, HEX);
    Serial.print(" on address 0x");
    Serial.println(module + OFFSET_ADDRESS, HEX);

    modules[module].id = module;
    modules[module].type = UNKNOWN;
    modules[module].active = false;

    Module_type module_type = who_are_you(module);
    if (module_type == UNKNOWN) continue;

    modules[module].type = module_type;
    modules[module].active = true;

    Serial.print("Module 0x");
    Serial.print(module, HEX);
    Serial.print(" identified as type 0x");
    Serial.println(module_type, HEX);

    delay(50); // prevent excessive I²C traffic
  }
}

void setup() {
  delay(1000); // necessary, serial seems to break without it

  Serial.begin(115200);
  Serial.flush();

  // set I²C pins for the controller
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  Wire.begin(); // we dont need bus address since we act as a controller

  delay(2000);

  initialize_devices();
}

void loop() {
  delay(100);
}