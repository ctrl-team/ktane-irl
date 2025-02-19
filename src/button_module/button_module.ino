#define BUTTON_PIN 15  // Button connected to GPIO15
#define HOLD_TIME 1000  // Hold time in milliseconds (2 seconds)

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Enable internal pull-down resistor
    Serial.begin(115200);  // Start serial communication
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long pressTime = millis();  // Record when the button was pressed

        // Wait until button is released
        while (digitalRead(BUTTON_PIN) == LOW);

        unsigned long releaseTime = millis();  // Record when the button was released
        unsigned long duration = releaseTime - pressTime;  // Calculate press duration

        if (duration >= HOLD_TIME) {
            Serial.println("Button Held");
        } else {
            Serial.println("Button Pressed and Released");
        }
    }
}
