# main_timer_board

The main timer board is the central controller for the game. It manages game time, connected modules, and overall game state. It also controls the main display and coordinates communication with all other modules using I2C.

This project uses the [**Waveshare 2.4" SPI ILI9341 LCD (model 18366)**](https://www.waveshare.com/2.4inch-lcd-module.htm). We provide a preconfigured header file for TFT_eSPI. If you are using the same screen, we highly recommend using our configuration to avoid display issues, the `Setup304_RP2040_ILI9341_Waveshare_2_4.h` configuration file is included in the `/include` directory. The correct display driver must be selected in the TFT_eSPI library.

## Wiring

### Display

| **Display Pin** | **RP2040 Pin (TFT_eSPI Mapping)** |
|-----------------|-----------------------------------|
| VCC             | 3.3V                              |
| GND             | GND                               |
| DIN             | GPIO11 (`TFT_MOSI`)               |
| CLK             | GPIO10 (`TFT_SCLK`)               |
| CS              | GPIO9  (`TFT_CS`)                 |
| DC              | GPIO8 (`TFT_DC`)                  |
| RST             | GPIO12 (`TFT_SCLK`)               |
| BL              | 3.3V                              |

### Buzzer

This project uses a buzzer **without a built-in generator**, so it must be connected to a **PWM-capable pin**

| **Buzzer Pin** | **RP2040 Pin (TFT_eSPI Mapping)** |
|----------------|-----------------------------------|
| +              | GPIO2                             |
| -              | GND                               |

All values are adjustable in code. You can adjust `BUZZER_INPUT` pin in `main_timer_board.ino`

## **Important Notice**

The **User_Setup_Select.h** file in the TFT_eSPI library must be modified to include the correct display configuration:

```cpp
#include "Setup304_RP2040_ILI9341_Waveshare_2_4.h"
```