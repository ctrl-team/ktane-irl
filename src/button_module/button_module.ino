#include <Arduino.h>
#include <Wire.h>

#define RGB_COMMON_ANODE // affects all diodes

#define BUTTON_PIN 15  // Button connected to GPIO15
#define HOLD_TIME 1500  // Hold time in milliseconds 
#define DEBOUNCE_DELAY 50  // Debounce delay in milliseconds

// RGB STATE LED Pins
#define STATE_RED_PIN 19
#define STATE_GREEN_PIN 20
#define STATE_BLUE_PIN 21

// RGB STRIP LED Pins
#define STRIP_RED_PIN 16
#define STRIP_GREEN_PIN 17
#define STRIP_BLUE_PIN 18

// I2C BUS Pins
#define SDA_PIN 0
#define SCL_PIN 1

#define MODULE_ADDRESS 0x8 // 0x8 - 0x16
#define MODULE_TYPE 0x2 // BUTTON_MODULE

enum Module_state {
  PLAYING = 0x1,
  STRIKE = 0x2,
  SOLVED = 0x3,
  PAUSED = 0x4,
  NOT_STARTED = 0x5,
  STATE_UNKNOWN = 0xF
};

enum ButtonLabel {
  ABORT = 1,
  DETONATE = 2,
  HOLD = 3
};

enum Color {
  BLUE = 0,
  WHITE = 1,
  YELLOW = 2,
  RED = 3,
  GREEN = 4,
  MAGENTA = 5,
  CYAN = 6
}

uint8_t requestedCommand = 0xF;
uint16_t receivedData = 0x00;

ModuleState MODULE_STATE = NOT_STARTED;

uint16_t lastPressTime = 0;  // Time of last button press
uint16_t pressTime = 0;  // Time the button was pressed
uint16_t lastHoldCheck = 0;  // Time of last hold check
uint16_t duration = 0;  // Duration of the press

// Example mock values (replace with actual logic to read these values)
bool hasBatteryMoreThan1 = false;  // More than 1 battery on the bomb
bool hasBatteryMoreThan2 = false;  // More than 2 batteries
bool litCARIndicator = true;  // Lit CAR indicator
bool litFRKIndicator = true;  // Lit FRK indicator
ButtonLabel buttonLabel = ABORT;  // Button label ("Abort", "Detonate", etc.)
Color buttonColor = BLUE;  // Button color ("Blue", "White", "Yellow", "Red")
Color stripColor = random(1, 5);

// Timer variables
uint16_t timer = 0;

// Variables to track button state
bool buttonPressed = false;
bool previousButtonState = HIGH;
bool isHolding = false;  // Track if button is being held

void setLEDColor(int redPin, int greenPin, int bluePin, int red, int green, int blue) {
  #ifdef RGB_COMMON_ANODE
    digitalWrite(redPin, !red);
    digitalWrite(greenPin, !green);
    digitalWrite(bluePin, !blue);
  #else
    digitalWrite(redPin, red);
    digitalWrite(greenPin, green);
    digitalWrite(bluePin, blue);
  #endif
}

void setStateColor(int red, int green, int blue) {
  setLEDColor(STATE_RED_PIN, red);
  setLEDColor(STATE_GREEN_PIN, green);
  setLEDColor(STATE_BLUE_PIN, blue);
}

void setStripColor(int red, int green, int blue) {
  setLEDColor(STRIP_RED_PIN, red);
  setLEDColor(STRIP_GREEN_PIN, green);
  setLEDColor(STRIP_BLUE_PIN, blue);
}

bool hasDigitInTimer(int digit) {
  int mins = timer / 60;
  int secs = timer % 60;

  return (secs % 10 == digit) ||
         ((secs / 10) % 10 == digit) ||
         (mins % 10 == digit) ||
         ((mins / 10) % 10 == digit);
}

bool shouldPressAndRelease() {
  return (hasBatteryMoreThan1 && buttonLabel == DETONATE) ||
         (hasBatteryMoreThan2 && litFRKIndicator) ||
         (buttonColor == RED && buttonLabel == HOLD);
}

void handleCommand() {
  switch (requestedCommand) {
    case 0x2:
      Wire.write(MODULE_STATE);
      if (MODULE_STATE == STRIKE)
        MODULE_STATE = PLAYING;
      break;
    case 0x4:
      Wire.write(MODULE_TYPE);
      break;
    default:
      Wire.write(0xF);
  }

  requestedCommand = 0xF;
}

void onDataRequest() {
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
  
  if (requestedCommand == 0x3) {
    timer = receivedData;
    receivedData = 0x0;
    requestedCommand = 0xF;

    char buffer[6];
    sprintf(buffer, "%02d:%02d", timer / 60, timer % 60);
    
    Serial.print("New time: ");
    Serial.println(buffer);
  }
}

void setup() {
  delay(1000);

  Serial.begin(115200);
  Serial.flush();

  // set I2C pins for the controller
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  Wire.begin(MODULE_ADDRESS);

  Wire.onReceive(onCommandReceive);
  Wire.onRequest(onDataRequest);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATE_RED_PIN, OUTPUT);
  pinMode(STATE_GREEN_PIN, OUTPUT);
  pinMode(STATE_BLUE_PIN, OUTPUT);
  pinMode(STRIP_RED_PIN, OUTPUT);
  pinMode(STRIP_GREEN_PIN, OUTPUT);
  pinMode(STRIP_BLUE_PIN, OUTPUT);

  setStateColor(LOW, LOW, LOW);
  setStripColor(LOW, LOW, LOW);

  Serial.print("Module 0x");
  Serial.print(MODULE_ADDRESS, HEX);
  Serial.println(" initialized");
}

void loop() {
  if (MODULE_STATE == PLAYING) {
    updateTimer();
    handleButton();
  }

  switch (MODULE_STATE) {
    case SOLVED:
      setStateColor(LOW, HIGH, LOW);
      break;
    case STRIKE:
      setStateColor(HIGH, LOW, LOW);
      break;
    }

    while (MODULE_STATE == STRIKE) {
      delay(500);
    }
}

void handleButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  uint16_t currentTime = millis();

  // Button press handling
  if (currentButtonState == LOW && previousButtonState == HIGH) {
    if (currentTime - lastPressTime > DEBOUNCE_DELAY) {
      buttonPressed = true;
      isHolding = false;
      pressTime = currentTime;
      lastHoldCheck = currentTime;
      Serial.println("Button pressed!");
      
      if (!shouldPressAndRelease()) {
        handleHoldingSequence();
      }
    }
  }
  
  // Continuous hold checking
  if (buttonPressed && currentButtonState == LOW) {
    duration = currentTime - pressTime;
    
    // Check for hold duration every 100ms
    if (currentTime - lastHoldCheck >= 100) {
      lastHoldCheck = currentTime;
      
      if (shouldPressAndRelease() && duration >= HOLD_TIME) {
        Serial.println("Strike! Held when should have been quick press");
        MODULE_STATE = STRIKE;
        buttonPressed = false;  // Reset button state after strike
      } else if (!shouldPressAndRelease() && !isHolding && duration >= HOLD_TIME) {
        isHolding = true;
        Serial.println("Hold threshold reached");
      }
    }
  }
  
  // Button release handling
  if (currentButtonState == HIGH && previousButtonState == LOW) {
    if (buttonPressed) {
      duration = currentTime - pressTime;
      buttonPressed = false;
      processButtonRelease();
    }
  }

  previousButtonState = currentButtonState;
}
