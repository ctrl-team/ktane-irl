# ktane-irl

## Overview
This project is a real-life adaptation of the game "Keep Talking and Nobody Explodes.".
Modules may use Arduino or different microcontrollers, with each module documented in its own directory.

## Features
* Real-life bomb-defusal gameplay
* Advanced component system

## Software Requirements
- Arduino IDE
- TFT_eSPI library (included in lib/)

## Installation

### 1. Clone the Repository
```sh
git clone https://github.com/ctrl-team/ktane-irl.git
cd ktane-irl
```
### 2. Configure TFT_eSPI
TFT_eSPI requires a proper `User_Setup_Select.h` configuration. Users must manually edit:
```sh
TFT_eSPI/User_Setup_Select.h
```
#### to include:
for main timer board
```cpp
#include "User_Setups/Setup304_RP2040_ILI9341_Waveshare_2_4"
```
for button module
```cpp
#include <User_Setups/Setup305_RP2040_GC9A01.h>
```

## License
This project is licensed under GNU General Public License v3.0. See the LICENSE file for details.

## Acknowledgments
- [TFT_eSPI by Bodmer](https://github.com/Bodmer/TFT_eSPI)
- [RPI Pico Schematic and footprint by ncarandini](https://github.com/ncarandini/KiCad-RP-Pico)

## Contributing
Feel free to submit pull requests or open issues for bug reports and feature requests.

