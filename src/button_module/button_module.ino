#define BUTTON_PIN 15  // Button connected to GPIO15
#define HOLD_TIME 1000  // Hold time in milliseconds 
#define DEBOUNCE_DELAY 50  // Debounce delay in milliseconds

// RGB LED Pins
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18

unsigned long lastPressTime = 0;  // Time of last button press
unsigned long pressTime = 0;  // Time the button was pressed
unsigned long lastHoldCheck = 0;  // Time of last hold check
unsigned long duration = 0;  // Duration of the press

// Example mock values (replace with actual logic to read these values)
bool hasBatteryMoreThan1 = false;  // More than 1 battery on the bomb
bool hasBatteryMoreThan2 = false;  // More than 2 batteries
bool litCARIndicator = true;  // Lit CAR indicator
bool litFRKIndicator = true;  // Lit FRK indicator
String buttonLabel = "Abort";  // Button label ("Abort", "Detonate", etc.)
String buttonColor = "Blue";  // Button color ("Blue", "White", "Yellow", "Red")

// Timer variables
int minutes = 5;  // Starting minutes
int seconds = 0;  // Starting seconds
unsigned long lastTimerUpdate = 0;
bool moduleActive = true;  // Module is still active (not solved)

// Variables to track button state
bool buttonPressed = false;
bool previousButtonState = HIGH;
bool isHolding = false;  // Track if button is being held

// Variable to store the current LED strip color
String currentLEDColor = "";

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);

  Serial.begin(115200);
  Serial.println("System initialized");
}

void loop() {
  if (moduleActive) {
    updateTimer();
    handleButton();
  }
}

void updateTimer() {
  if (millis() - lastTimerUpdate >= 1000) {
    lastTimerUpdate = millis();
    
    if (seconds > 0) {
      seconds--;
    } else if (minutes > 0) {
      minutes--;
      seconds = 59;
    }
    
    Serial.print("Time remaining: ");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.println(seconds);
  }
}

void handleButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  unsigned long currentTime = millis();

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
        moduleStrike();
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

bool shouldPressAndRelease() {
  return (hasBatteryMoreThan1 && buttonLabel == "Detonate") ||
         (hasBatteryMoreThan2 && litFRKIndicator) ||
         (buttonColor == "Red" && buttonLabel == "Hold");
}

void handleHoldingSequence() {
  // Randomly select a color for the LED strip
  int colorChoice = random(0, 4);  // 0: Blue, 1: White, 2: Yellow, 3: Other

  switch (colorChoice) {
    case 0:
      currentLEDColor = "Blue";
      lightUpLED(HIGH, HIGH, LOW);  // Blue for common anode
      break;
    case 1:
      currentLEDColor = "White";
      lightUpLED(LOW, LOW, LOW);  // White for common anode
      break;
    case 2:
      currentLEDColor = "Yellow";
      lightUpLED(LOW, LOW, HIGH);  // Yellow for common anode
      break;
    default:
      currentLEDColor = "Other";
      lightUpLED(HIGH, LOW, HIGH);  // Green for common anode
      break;
  }

  Serial.print("LED strip color: ");
  Serial.println(currentLEDColor);
}

bool hasDigitInTimer(int digit) {
  int mins = minutes;
  int secs = seconds;
  
  return (secs % 10 == digit) || 
         ((secs / 10) % 10 == digit) || 
         (mins % 10 == digit) || 
         ((mins / 10) % 10 == digit);
}

void processButtonRelease() {
  if (shouldPressAndRelease()) {
    if (duration < HOLD_TIME) {
      Serial.println("Module solved! Correct quick press.");
      moduleSolved();
    }
  } else {
    if (duration >= HOLD_TIME) {
      bool correctRelease = false;
      
      if (currentLEDColor == "Blue") {
        correctRelease = hasDigitInTimer(4);
      } else if (currentLEDColor == "White") {
        correctRelease = hasDigitInTimer(1);
      } else if (currentLEDColor == "Yellow") {
        correctRelease = hasDigitInTimer(5);
      } else {
        correctRelease = hasDigitInTimer(1);
      }
      
      if (correctRelease) {
        Serial.println("Module solved! Correct hold and release.");
        moduleSolved();
      } else {
        Serial.println("Strike! Released on wrong number.");
        moduleStrike();
      }
    } else {
      Serial.println("Strike! Button wasn't held long enough.");
      moduleStrike();
    }
  }
}

void moduleSolved() {
  moduleActive = false;
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  Serial.println("Module deactivated - SUCCESS!");
}

void moduleStrike() {
  lightUpLED(LOW, HIGH, HIGH);  // Red for strike
  delay(500);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  Serial.println("STRIKE!");
}

void lightUpLED(int red, int green, int blue) {
  digitalWrite(RED_PIN, red);
  digitalWrite(GREEN_PIN, green);
  digitalWrite(BLUE_PIN, blue);
  Serial.println("LED color changed");
}