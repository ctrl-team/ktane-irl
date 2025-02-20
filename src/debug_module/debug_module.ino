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

enum ModuleState {
  PLAYING = 0x01,
  STRIKE = 0x02,
  SOLVED = 0x03,
  PAUSED = 0x04,
  NOT_STARTED = 0x05,
  STATE_UNKNOWN = 0xF
};

// UNKNOWN
uint8_t requestedCommand = 0xF;
uint16_t receivedData = 0x00;

int timer = 0;

bool solvePressed = true;
bool strikePressed = true;

ModuleState MODULE_STATE = NOT_STARTED;

void handleCommand() {
  switch (requestedCommand) {
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

  requestedCommand = 0xFF;
}

void onDataRequest() {
  Serial.println(requestedCommand, HEX);
  handleCommand();
}

void onCommandReceive(int numBytes) {
  requestedCommand = Wire.read();

  receivedData = 0;

  if (numBytes == 3) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();
    receivedData = (highByte << 8) | lowByte;
  }

  Serial.print("Received command 0x");
  Serial.print(requestedCommand, HEX);
  Serial.print(" with data 0x");
  Serial.println(receivedData, HEX);

  if (requestedCommand == 0x3) {
    timer = receivedData;
    receivedData = 0x00;
    requestedCommand = 0xF;

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

  Wire.onReceive(onCommandReceive);
  Wire.onRequest(onDataRequest);

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
  bool solveState = digitalRead(SOLVE_PIN) == LOW;
  bool strikeState = digitalRead(STRIKE_PIN) == LOW;

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

  if (solveState && !solvePressed) {
    MODULE_STATE = SOLVED;
  } 
  
  if (strikeState && !strikePressed) {
    MODULE_STATE = STRIKE;
  }

  solvePressed = solveState;
  strikePressed = strikeState;

  while (MODULE_STATE == STRIKE) {
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 255);
    analogWrite(B_PIN, 255);
    delay(50);
  }
}