#include <Arduino.h>
#include <../../lib/i2c_receiver/i2c_receiver.h>

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

#define MODULE_ADDRESS 0x8 // 0x8 - 0x16
#define MODULE_TYPE 0x2 // BUTTON_MODULE

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
};

I2CReceiver busReceiver = I2CReceiver(BUTTON_MODULE);

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
Color stripColor = static_cast<Color>(random(1, 5));

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
  setLEDColor(STATE_RED_PIN, STATE_GREEN_PIN, STATE_BLUE_PIN, red, green, blue);
}

void setStripColor(int red, int green, int blue) {
  setLEDColor(STRIP_RED_PIN, STRIP_GREEN_PIN, STRIP_BLUE_PIN, red, green, blue);
}

bool hasDigitInTimer(int digit) {
  int mins = busReceiver.timer / 60;
  int secs = busReceiver.timer % 60;

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

void setup() {
  delay(1000);

  Serial.begin(115200);
  Serial.flush();

  busReceiver.begin();

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
  Serial.print(busReceiver.getAddress(), HEX);
  Serial.println(" initialized");

  Serial.print("Button color: ");
  Serial.println(buttonColor);

  Serial.print("Button label: ");
  Serial.println(buttonLabel);

  Serial.print("Strip color: ");
  Serial.println(stripColor);
}

void loop() {
  if (busReceiver.currentState == PLAYING)
    handleButton();

  switch (busReceiver.currentState) {
    case SOLVED:
      setStateColor(LOW, HIGH, LOW);
      break;
    case STRIKE:
      setStateColor(HIGH, LOW, LOW);
      break;
    default:
      setStateColor(LOW, LOW, LOW);
  }

  while (busReceiver.currentState == STRIKE || busReceiver.currentState == SOLVED) {
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
        busReceiver.currentState = STRIKE;
        buttonPressed = false;  // Reset button state after strike
      } else if (!shouldPressAndRelease() && !isHolding && duration >= HOLD_TIME) {
        handleHoldingSequence();
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

void processButtonRelease() {
  if (shouldPressAndRelease()) {
    if (duration < HOLD_TIME) {
      Serial.println("Module solved! Correct quick press.");
      busReceiver.currentState = SOLVED;
    }
  } else {
    if (duration >= HOLD_TIME) {
      bool correctRelease = false;
      
      if (stripColor == BLUE) {
        correctRelease = hasDigitInTimer(4);
      } else if (stripColor == YELLOW) {
        correctRelease = hasDigitInTimer(5);
      } else {
        correctRelease = hasDigitInTimer(1);
      }
      
      if (correctRelease) {
        Serial.println("Module solved! Correct hold and release.");
        busReceiver.currentState = SOLVED;
      } else {
        Serial.println("Strike! Released on wrong number.");
        busReceiver.currentState = STRIKE;
      }
    } else {
      Serial.println("Strike! Button wasn't held long enough.");
      busReceiver.currentState = STRIKE;
    }
  }

  setStripColor(LOW, LOW, LOW);
}

void handleHoldingSequence() {
  switch (stripColor) {
    case BLUE:
      setStripColor(LOW, LOW, HIGH);
      break;
    case WHITE:
      setStripColor(HIGH, HIGH, HIGH);
      break;
    case YELLOW:
      setStripColor(HIGH, HIGH, LOW);
      break;
    case RED:
      setStripColor(HIGH, LOW, LOW);
      break;
    case GREEN:
      setStripColor(LOW, HIGH, LOW);
      break;
    case MAGENTA:
      setStripColor(HIGH, LOW, HIGH);
      break;
    case CYAN:
      setStripColor(LOW, HIGH, HIGH);
      break;
  }

  Serial.print("LED strip color: ");
  Serial.println(stripColor);
}