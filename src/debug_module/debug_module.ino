#include <Arduino.h>
#include <Wire.h>

#define SOLVE_PIN 16
#define STRIKE_PIN 17

// common-anode RGB LED
#define R_PIN 18
#define G_PIN 19
#define B_PIN 20

#define SDA_PIN 0
#define SCL_PIN 1

// 0x08 - 0x16
#define MODULE_ADDRESS 0x16

// DEBUG_MODULE
#define MODULE_TYPE 0x01

enum Module_state {
  PLAYING = 0x01,
  STRIKE = 0x02,
  SOLVED = 0x03,
  PAUSED = 0x04,
  NOT_STARTED = 0x05,
  STATE_UNKOWN = 0xF
};

// UNKNOWN
uint8_t requested_command = 0xF;
uint16_t received_data = 0x00;

int timer = 0;

bool solve_pressed = true;
bool strike_pressed = true;

Module_state MODULE_STATE = NOT_STARTED;

void handle_command() {
  switch (requested_command) {
    case 0x02:
      Wire.write(MODULE_STATE);
      if (MODULE_STATE == STRIKE)
        MODULE_STATE = PLAYING;
      break;
    case 0x04:
      Wire.write(MODULE_TYPE);
      break;
    default:
      Wire.write(0xF);
  }

  requested_command = 0xFF;
}

void on_data_request() {
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

    Serial.print("New timer: ");
    Serial.println(timer);
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

  pinMode(SOLVE_PIN, INPUT_PULLUP);
  pinMode(STRIKE_PIN, INPUT_PULLUP);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);

  Serial.print("Module 0x");
  Serial.print(MODULE_ADDRESS, HEX);
  Serial.println(" initialized");
}

void loop() {
  bool solve_state = digitalRead(SOLVE_PIN) == LOW;
  bool strike_state = digitalRead(STRIKE_PIN) == LOW;

  switch (MODULE_STATE) {
    case SOLVED:
      analogWrite(R_PIN, 255);
      analogWrite(G_PIN, 0);
      analogWrite(B_PIN, 255);
      break;
    default:
      analogWrite(R_PIN, 255);
      analogWrite(G_PIN, 255);
      analogWrite(B_PIN, 255);
      break;
  }

  if (solve_state && !solve_pressed) {
    MODULE_STATE = SOLVED;
  } 
  
  if (strike_state && !strike_pressed) {
    MODULE_STATE = STRIKE;
  }

  solve_pressed = solve_state;
  strike_pressed = strike_state;

  while (MODULE_STATE == STRIKE) {
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 255);
    analogWrite(B_PIN, 255);
    delay(50);
  }
}