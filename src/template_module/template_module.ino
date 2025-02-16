#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 0
#define SCL_PIN 1

// 0x08 - 0x16
#define MODULE_ADDRESS 0x16

// DEBUG_MODULE
#define MODULE_TYPE 0x01

enum Module_state {
  PLAYING = 0x01,
  STREAK = 0x02,
  SOLVED = 0x03,
  PAUSED = 0x04,
  NOT_STARTED = 0x05,
  STATE_UNKOWN = 0xFF
};

// UNKNOWN
uint8_t requested_command = 0xF;
uint16_t received_data = 0x00;

int timer = 0;

Module_state MODULE_STATE = NOT_STARTED;

void handle_command() {
  switch (requested_command) {
    case 0x02:
      Wire.write(MODULE_STATE);
      break;
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

  received_data = 0;

  if (numBytes == 3) {
    uint8_t high_byte = Wire.read();
    uint8_t low_byte = Wire.read();
    received_data = (high_byte << 8) | low_byte;
  }

  Serial.print("Received command 0x");
  Serial.print(requested_command, HEX);
  Serial.print(" with data 0x");
  Serial.println(received_data, HEX);

  if (requested_command == 0x3) {
    timer = received_data;
    received_data = 0x00;
    requested_command = 0xF;
  }
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