#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 0
#define SCL_PIN 1

#define BROADCAST_ADDR 0x10
#define MAX_MODULES 15

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

uint8_t receive_byte() {
  Wire.requestFrom(BROADCAST_ADDR, 1);
  return Wire.available() ? Wire.read() : 0xFF;
}

void send_request(uint8_t target_id, uint8_t command) {
  Wire.beginTransmission(BROADCAST_ADDR);
  Wire.write(target_id);
  Wire.write(command);
  Wire.endTransmission();
}

Module_type who_are_you(uint8_t target_id) {
  send_request(target_id, 0x04);
  return static_cast<Module_type>(receive_byte());
}

void initialize_devices() {
  for (int module = 0; module < MAX_MODULES; module++) {
    Serial.print("Scanning module ");
    Serial.println(module, HEX);

    modules[module].id = module;
    modules[module].type = UNKNOWN; 
    modules[module].active = false;

    Module_type module_type = who_are_you(module);
    if (module_type == UNKNOWN) continue;

    modules[module].type = module_type;
    modules[module].active = true;

    Serial.print("Module ");
    Serial.print(module, HEX);
    Serial.print(" identified as type ");
    Serial.println(module_type);
  }
}

void setup() {
  delay(1000); // necessary, serial seems to break without it

  Serial.begin(115200);
  Serial.flush();

  // set i2c pins for the controller
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  // we dont need bus address since we act as a controller
  Wire.begin();

  delay(2000);

  initialize_devices();
}

void loop() {
  delay(100);
}