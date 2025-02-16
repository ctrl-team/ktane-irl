#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 0
#define SCL_PIN 1

// 0x08 - 0x16
#define MODULE_ADDRESS 0x16

// DEBUG_MODULE
#define MODULE_TYPE 0x01

// UNKNOWN
int requested_command = 0xFF;

void handle_command() {
  switch (requested_command) {
    case 0x04:
      Wire.write(MODULE_TYPE);
      break;
  }

  requested_command = 0xFF;
}

void on_data_request() {
  Serial.print("Handling command 0x");
  Serial.println(requested_command, HEX);
  handle_command();
}

void on_command_receive(int numBytes) {
  requested_command = Wire.read();

  Serial.print("Received command 0x");
  Serial.println(requested_command, HEX);
}

void setup() {
  delay(1000); // necessary, serial seems to break without it

  Serial.begin(115200);
  Serial.flush();

  // set i2c pins for the controller
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  // listen to broadcast bus
  Wire.begin(MODULE_ADDRESS);

  Wire.onReceive(on_command_receive);
  Wire.onRequest(on_data_request);

  Serial.print("Module 0x");
  Serial.print(MODULE_ADDRESS, HEX);
  Serial.println(" initialized");
}

void loop() {
  delay(100);
}