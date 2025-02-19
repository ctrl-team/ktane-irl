#define BUTTON_PIN 15  // Button connected to GPIO15
#define HOLD_TIME 2000  // Hold time in milliseconds (2 seconds)
#define DEBOUNCE_DELAY 50  // Debounce delay in milliseconds

// RGB LED Pins
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18

unsigned long lastPressTime = 0;  // Time of last button press
unsigned long pressTime = 0;  // Time the button was pressed
unsigned long releaseTime = 0;  // Time the button was released
unsigned long duration = 0;  // Duration of the press

// Example mock values (replace with actual logic to read these values)
bool hasBatteryMoreThan1 = true;  // More than 1 battery on the bomb
bool hasBatteryMoreThan2 = false;  // More than 2 batteries
bool litCARIndicator = true;  // Lit CAR indicator
bool litFRKIndicator = false;  // Lit FRK indicator
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
bool isHolding = false;  // Track if we're holding the button

void setup() {
  // Set the button pin as input with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Set RGB LED pins as OUTPUT
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Initialize LED to OFF (HIGH for common anode)
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);

  Serial.begin(115200);  // Start serial communication
  Serial.println("System initialized");  // Confirm serial is working
}

void loop() {
  if (moduleActive) {
    updateTimer();
    handleButton();
  }
}

void updateTimer() {
  // Update timer every second
  if (millis() - lastTimerUpdate >= 1000) {
    lastTimerUpdate = millis();
    
    if (seconds > 0) {
      seconds--;
    } else if (minutes > 0) {
      minutes--;
      seconds = 59;
    }
    
    // Print current time for debugging
    Serial.print("Time remaining: ");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.println(seconds);
  }
}

void handleButton() {
  // Read the current button state
  bool currentButtonState = digitalRead(BUTTON_PIN);
  unsigned long currentTime = millis();

  // Check for button press with debounce
  if (currentButtonState == LOW && previousButtonState == HIGH) {
    if (currentTime - lastPressTime > DEBOUNCE_DELAY) {
      buttonPressed = true;
      isHolding = true;
      pressTime = currentTime;
      Serial.println("Button pressed!");
      
      // Handle immediate press rules
      if (shouldPressAndRelease()) {
        if (duration < HOLD_TIME) {
          Serial.println("Module solved! Correct quick press.");
          moduleSolved();
        } else {
          Serial.println("Strike! Should have been a quick press.");
          moduleStrike();
        }
      } else {
        // Start holding sequence for hold rules
        handleHoldingSequence();
      }
    }
  }
  
  // Check for button release
  if (currentButtonState == HIGH && previousButtonState == LOW) {
    if (buttonPressed) {
      releaseTime = currentTime;
      duration = releaseTime - pressTime;
      buttonPressed = false;
      isHolding = false;
      
      // Check if the release timing was correct for hold rules
      if (!shouldPressAndRelease()) {
        checkReleaseTime();
      }
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
  if (buttonColor == "Blue" && buttonLabel == "Abort") {
    lightUpLED(HIGH, HIGH, LOW);  // Blue for common anode
  } else if (buttonColor == "White" && litCARIndicator) {
    lightUpLED(LOW, LOW, LOW);  // White for common anode
  } else if (buttonColor == "Yellow") {
    lightUpLED(LOW, LOW, HIGH);  // Yellow for common anode
  } else {
    lightUpLED(HIGH, LOW, HIGH);  // Green for common anode
  }
}

void checkReleaseTime() {
  int currentDigit = seconds % 10;  // Get the last digit of seconds
  
  bool correctRelease = false;
  
  if (buttonColor == "Blue" && buttonLabel == "Abort") {
    correctRelease = (currentDigit == 4);
  } else if (buttonColor == "White" && litCARIndicator) {
    correctRelease = (currentDigit == 1);
  } else if (buttonColor == "Yellow") {
    correctRelease = (currentDigit == 5);
  } else {
    correctRelease = (currentDigit == 1);
  }
  
  if (duration >= HOLD_TIME && correctRelease) {
    Serial.println("Module solved! Correct hold and release.");
    moduleSolved();
  } else {
    Serial.println("Strike! Incorrect hold or release timing.");
    moduleStrike();
  }
}

void moduleSolved() {
  moduleActive = false;
  // Turn off all LEDs (HIGH for common anode)
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  Serial.println("Module deactivated - SUCCESS!");
}

void moduleStrike() {
  // Flash red LED for strike
  lightUpLED(LOW, HIGH, HIGH);  // Red for strike
  delay(500);
  // Turn off LED
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  Serial.println("STRIKE!");
}

// Function to control RGB LED color (common anode - HIGH is OFF, LOW is ON)
void lightUpLED(int red, int green, int blue) {
  digitalWrite(RED_PIN, red);
  digitalWrite(GREEN_PIN, green);
  digitalWrite(BLUE_PIN, blue);
  Serial.println("LED color changed");
}