/******************************************************************************
 * @file        main_timer_board.ino
 * @brief       Main timer board logic for the game on RP2040
 *
 * @details     This file contains the core timer functionality for the game, 
 *              including timing events, display updates, and game logic. 
 *              It interfaces with the Waveshare 1.8" SPI TFT LCD (ST7735S) 
 *              using the TFT_eSPI library.
 *
 * IMPORTANT:  
 *  - This project uses the **Waveshare 2.4" SPI TFT LCD (ILI9341)**.
 *  - The correct display driver (**ILI9341**) must be selected.
 *  - The **Setup304_RP2040_ILI9341_Waveshare_2_4.h** file is provided in the same directory.
 *  - Ensure that **User_Setup_Select.h** in the TFT_eSPI library is configured 
 *    to include this setup file instead of modifying the library directly.
 ******************************************************************************/

#include "Arduino.h"
#include <TFT_eSPI.h>
#include <SPI.h>

// these are inverter since we're rotating whole screen 90 degrees
#define LCD_HEIGHT TFT_WIDTH
#define LCD_WIDTH TFT_HEIGHT

#define BOTTOM_BUTTON 19
#define ENTER_BUTTON 20
#define BACK_BUTTON 21

#define BUZZER_INPUT 2

TFT_eSPI tft = TFT_eSPI();

bool rerender = true;
bool rerender_content = true;
bool game_started = false;
bool just_started = false;
int timer = 0;
int default_timer = 10;
int last_opened = 0;
int interval_time = 0;

bool down_pressed = false;
bool enter_pressed = false;
bool back_pressed = false;

bool led_on = true;

enum Menus {
  NONE,
  MAIN_MENU,
  SETTINGS,
  INFORMATION,

  SETTINGS_TIMER
};

int opened_menu = MAIN_MENU;
int selected_item = 0;
int left_padding = 2;

void render_top_bar() {
  tft.fillRect(0, 0, LCD_WIDTH, 20, TFT_RED);
  tft.setCursor(left_padding, 2, 2);
  tft.setTextColor(TFT_WHITE);

  switch (opened_menu) {
    case MAIN_MENU:
      tft.println("Main menu");
      break;
    case SETTINGS:
      tft.println("Settings");
      break;
    case SETTINGS_TIMER:
      tft.println("Game time");
      break;
    case INFORMATION:
      tft.println("Information");
      break;
  }
}

void render_content() {
  tft.setCursor(left_padding, 22, 2);

  switch (opened_menu) {
    case MAIN_MENU:
      tft.setTextColor(TFT_WHITE, selected_item == 0 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print(game_started ? "End game" : "Start game");

      tft.setCursor(left_padding, 22 + tft.fontHeight() + 4, 2);  // 22 + top_bar, + 4 margin
      tft.setTextColor(game_started ? TFT_DARKGREY : TFT_WHITE, selected_item == 1 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print("Settings");

      tft.setCursor(left_padding, 22 + tft.fontHeight() * 2 + 8, 2);  // 22 + top_bar, + 4 margin (include last one)
      tft.setTextColor(game_started ? TFT_DARKGREY : TFT_WHITE, selected_item == 2 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print("Information");

      break;
    case SETTINGS:
      tft.setTextColor(TFT_WHITE, selected_item == 0 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print("Timer");
      break;
    case SETTINGS_TIMER:
      char buffer[6];
      sprintf(buffer, "%02d:%02d", default_timer / 60, default_timer % 60);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(6);
      tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
      tft.print(buffer);
      break;
    case INFORMATION:
      tft.setTextFont(2);
      tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("IN DEVELOPMENT") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
      tft.setTextColor(TFT_WHITE);
      tft.print("IN DEVELOPMENT");
      break;
  }
}

void render_bottom_bar() {
  tft.fillRect(0, LCD_HEIGHT - 20, LCD_WIDTH, LCD_HEIGHT, TFT_RED);
  tft.setTextFont(1);

  if (opened_menu == MAIN_MENU) {
    switch (selected_item) {
      case 1:
        tft.setCursor(left_padding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("Change game behaviour");
        break;
      case 2:
        tft.setCursor(left_padding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("Display connected modules");
        break;
      default:
        tft.setCursor(left_padding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("F1-ENTER");
        tft.setCursor(left_padding * 3 + tft.textWidth("F1-ENTER"), LCD_HEIGHT - 14);
        tft.setTextColor(game_started ? TFT_WHITE : TFT_DARKGREY);
        tft.print("F2-BACK");
        break;
    }
  }

  if (opened_menu == SETTINGS) {
    switch (selected_item) {
      default:
        tft.setCursor(left_padding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("F1-ENTER");
        tft.setCursor(left_padding * 3 + tft.textWidth("F1-ENTER"), LCD_HEIGHT - 14);
        tft.print("F2-BACK");
        break;
    }
  }

  if (opened_menu == SETTINGS_TIMER) {
    tft.setCursor(left_padding, LCD_HEIGHT - 14);
    tft.setTextColor(TFT_WHITE);
    tft.print("F1 <");
    tft.setCursor(left_padding * 3 + tft.textWidth("F1 <"), LCD_HEIGHT - 14);
    tft.print("F3 >");
    tft.setCursor(left_padding * 5 + tft.textWidth("F1 <") + tft.textWidth("F2 >"), LCD_HEIGHT - 14);
    tft.print("F2 - SAVE");
  }
}

void render() {
  if (!rerender && !rerender_content) return;

  if (rerender) {
    tft.fillScreen(TFT_BLACK);
    selected_item = 0;
  }

  switch (opened_menu) {
    case NONE:
      char buffer[6];
      sprintf(buffer, "%02d:%02d", timer / 60, timer % 60);

      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextFont(8);
      tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
      tft.print(buffer);
      break;
    default:
      render_top_bar();
      render_content();
      render_bottom_bar();
      break;
  }

  rerender = false;
  rerender_content = false;
}

void handle_buttons() {
  bool down_state = digitalRead(BOTTOM_BUTTON) == HIGH;
  bool enter_state = digitalRead(ENTER_BUTTON) == HIGH;
  bool back_state = digitalRead(BACK_BUTTON) == HIGH;

  if (down_state && !down_pressed) {
    tone(BUZZER_INPUT, 1000, 50);

    if (opened_menu == MAIN_MENU) {
      selected_item = ++selected_item % 3;
      rerender_content = true;
    } else if (opened_menu == SETTINGS_TIMER) {
      default_timer--;
      rerender_content = true;
    }
  }

  if (enter_state && !enter_pressed) {
    if (opened_menu != MAIN_MENU || (opened_menu == MAIN_MENU && (!game_started && selected_item != 0))) {
      tone(BUZZER_INPUT, 1000, 50);
    }

    if (opened_menu == MAIN_MENU) {
      switch (selected_item) {
        case 0:
          game_started = !game_started;
          opened_menu = game_started ? NONE : MAIN_MENU;
          rerender = true;

          if (game_started) {
            timer = default_timer;
            just_started = true;

            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextFont(6);
            tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("123") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
            tft.fillScreen(TFT_BLACK);

            tft.print("3");
            tone(BUZZER_INPUT, 1000, 100);
            delay(900);

            tft.print("2");
            tone(BUZZER_INPUT, 1500, 100);
            delay(900);

            tft.print("1");
            tone(BUZZER_INPUT, 2000, 100);
            delay(900);
          } else {
            timer = 0;
          }
          break;
        case 1:
          opened_menu = SETTINGS;
          rerender = true;
          break;
        case 2:
          opened_menu = INFORMATION;
          rerender = true;
          break;
      }
    } else if (opened_menu == SETTINGS) {
      switch (selected_item) {
        case 0:
          opened_menu = SETTINGS_TIMER;
          rerender = true;
          break;
      }
    } else if (opened_menu == NONE) {
      opened_menu = MAIN_MENU;
      rerender = true;
      last_opened = millis();
    } else if (opened_menu == SETTINGS_TIMER) {
      opened_menu = SETTINGS;
      rerender = true;
    }
  }

  if (back_state && !back_pressed) {
    tone(BUZZER_INPUT, 1000, 50);

    if (opened_menu == MAIN_MENU && game_started) {
      opened_menu = NONE;
      rerender = true;
    } else if (opened_menu == SETTINGS || opened_menu == INFORMATION) {
      opened_menu = MAIN_MENU;
      rerender = true;
    } else if (opened_menu == SETTINGS_TIMER) {
      default_timer++;
      rerender_content = true;
    }
  }

  down_pressed = down_state;
  enter_pressed = enter_state;
  back_pressed = back_state;
}

void game_logic() {
  int current_time = millis();

  if (opened_menu != NONE && current_time - last_opened >= 10000) {
    last_opened = 0;
    opened_menu = NONE;
    rerender = true;
  }

  if (opened_menu == NONE && current_time - interval_time >= 1000) {
    timer--;

    interval_time = millis();
    rerender_content = true;

    if (timer <= 5) {
      tone(BUZZER_INPUT, 1000 + 100 * (5 - timer), 50);
    }

    if (timer <= 0) {
      tft.fillScreen(TFT_BLACK);

      tone(BUZZER_INPUT, 2000);
      delay(300);
      noTone(BUZZER_INPUT);
      delay(100);
      tone(BUZZER_INPUT, 2000);
      delay(300);
      noTone(BUZZER_INPUT);
      delay(100);
      tone(BUZZER_INPUT, 2000);
      delay(300);
      noTone(BUZZER_INPUT);

      while (digitalRead(ENTER_BUTTON) == LOW) {
        tft.setTextColor(TFT_RED);
        tft.setTextFont(2);
        tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("Game over") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2) - 2);
        tft.print("Game over");
        tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("Press 'Enter' to continue", 1) / 2), (LCD_HEIGHT / 2) + (tft.fontHeight() / 2) + 2, 1);
        tft.setTextColor(TFT_WHITE);
        tft.print("Press 'Enter' to continue");
        delay(50);
      }

      enter_pressed = true;
      game_started = false;
      opened_menu = MAIN_MENU;
      rerender = true;
    }
  }
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);

  pinMode(BOTTOM_BUTTON, INPUT_PULLDOWN);
  pinMode(ENTER_BUTTON, INPUT_PULLDOWN);
  pinMode(BACK_BUTTON, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_INPUT, OUTPUT);

  rerender = true;
}

void loop() {
  handle_buttons();

  if (just_started) {
    just_started = false;

    tone(BUZZER_INPUT, 1000);
    delay(50);
    tone(BUZZER_INPUT, 1500);
    delay(50);
    tone(BUZZER_INPUT, 2000);
    delay(50);
    noTone(BUZZER_INPUT);
  }

  if (game_started) game_logic();

  render();
}