# Button Module

Recreation of the Button module from "Keep Talking and Nobody Explodes". The module features a physical button with an RGB LED strip and follows the same rules as the original game.

## Hardware Requirements

* Arduino-compatible board 
* 1x Push button
* 1x Common anode RGB LED
* 3x 220Ω resistors
* Jumper wires
* Breadboard (optional)

## Pin Configuration

```
Button: GPIO15
RGB LED:
- Red: GPIO16
- Green: GPIO17
- Blue: GPIO18
```

## Wiring Diagram

```
Button:
- One pin to GPIO15
- Other pin to GND

RGB LED (Common Anode):
- Common pin to 3.3V/5V
- Red pin through 220Ω resistor to GPIO16
- Green pin through 220Ω resistor to GPIO17
- Blue pin through 220Ω resistor to GPIO18
```

## Features

* Real-time bomb timer countdown
* LED color feedback based on button state
* Strike system for incorrect inputs
* Serial monitor feedback for debugging
* Proper button debouncing
* Follows official KTANE rules for the Button module

## Game Rules

The module implements the following rules from the original game:

### Follow these rules in the order they are listed. Perform the first action that applies:

1. If the button is $${\color{#1589F0}Blue}$$ and the button says **"Abort"**, hold the button and refer
to "Releasing a Held Button".
2. If there is **more than 1 battery** on the bomb and the button says **"Detonate"**,
press and immediately release the button.
3. If the button is $${\color{#FFFFFF}White}$$ and there is a **lit indicator with label CAR**, hold the
button and refer to "Releasing a Held Button".
4. If there are **more than 2 batteries** on the bomb and there is a **lit indicator
with label FRK**, press and immediately release the button.
5. If the button is $${\color{#FFCC00}Yellow}$$, hold the button and refer to "Releasing a Held
Button".
6. If the button is $${\color{#FF0000}Red}$$ and the button says **"Hold"**, press and immediately
release the button.
7. If none of the above apply, hold the button and refer to "Releasing a Held
Button".

### Releasing a Held Button

If you start holding the button down, a colored strip will light up on the right
side of the module. Based on its color, you must release the button at a specific
point in time:
- $${\color{#1589F0}Blue}$$ strip: release when the countdown timer has a 4 in any position.
- $${\color{#FFFFFF}White}$$ strip: release when the countdown timer has a 1 in any position.
- $${\color{#FFCC00}Yellow}$$ strip: release when the countdown timer has a 5 in any position.
- Any other color strip: release when the countdown timer has a 1 in any
position.

## Configuration Variables

You can modify these variables at the top of the code to test different scenarios:

```cpp
bool hasBatteryMoreThan1 = true;   // More than 1 battery
bool hasBatteryMoreThan2 = false;  // More than 2 batteries
bool litCARIndicator = true;       // Lit CAR indicator
bool litFRKIndicator = false;      // Lit FRK indicator
String buttonLabel = "Abort";      // Button text
String buttonColor = "Blue";       // Button color
```

## Installation

1. Connect the hardware according to the pin configuration
2. Open the Arduino IDE
4. Upload the code to your board
5. Open Serial Monitor at 115200 baud to see debug information

## Serial Monitor Output

The Serial Monitor provides real-time feedback:
* Current timer value
* Button press and release events
* Module states (solved/strike)
* LED color changes

## Known Limitations FOR NOW

* Timer does not sync with other modules
* Mock values must be changed in code (no external inputs)
* No physical display for timer values

## Credits

Based on the Button module from "Keep Talking and Nobody Explodes" by Steel Crate Games.
