#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 0
#define SCL_PIN 1

#define BROADCAST_ADDR 0x10
#define MODULE_ID 0x0

// DEBUG_MODULE
#define MODULE_TYPE 0x01

void acknowledge(uint8_t data) {
  Wire.beginTransmission(BROADCAST_ADDR);
  Wire.write(data);
  Wire.endTransmission();
}

void handle_command(int command) {
  switch (command) {
    case 0x04:
      acknowledge(MODULE_TYPE);
      break;
  }
}

void on_command_receive(int numBytes) {
  int id = Wire.read();

  Serial.print("Received command packet for ");
  Serial.println(id, HEX);

  if (id != MODULE_ID) return;

  int command = Wire.read();

  Serial.print("Received command ");
  Serial.println(command, HEX);

  delay(10);

  handle_command(command);
}

void setup() {
  delay(1000); // necessary, serial seems to break without it

  Serial.begin(115200);
  Serial.flush();

  // set i2c pins for the controller
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  // listen to broadcast bus
  Wire.begin(BROADCAST_ADDR);

  Wire.onReceive(on_command_receive);
}

void loop() {
  delay(100);
}