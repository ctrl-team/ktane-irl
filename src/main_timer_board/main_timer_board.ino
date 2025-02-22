#include "Arduino.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include "bus_controller.h"

// swapping width and height because the screen is used in landscape mode
#define LCD_HEIGHT TFT_WIDTH
#define LCD_WIDTH TFT_HEIGHT

// I/O definitions
#define TOP_BUTTON 18
#define BOTTOM_BUTTON 19
#define ENTER_BUTTON 20
#define BACK_BUTTON 21

#define BUZZER_PIN 2 // Buzzer + pin

// I2C BUS Pins
#define SDA_PIN 0
#define SCL_PIN 1

enum Menus {
  NONE,
  MAIN_MENU,
  SETTINGS,
  SETTINGS_TIMER,
  INFORMATION
};

TFT_eSPI tft = TFT_eSPI();

bool rerender = true;
bool rerenderContent = true;
Menus openedMenu = MAIN_MENU;
int selectedItem = 0;

// timing variables
int lastOpened = 0;
int intervalTime = 0;
int stateTimer = 0;

bool upPressed = false;
bool downPressed = false;
bool enterPressed = false;
bool backPressed = false;
bool ledState = false;

bool gameStarted = false;
bool justStarted = false;
int strike = 0;
uint16_t timer = 120; // just enough time for 18 hours gameplay
uint16_t defaultTimer = 10; // 5 minutes
int leftPadding = 2;

void renderTopBar() {
  tft.fillRect(0, 0, LCD_WIDTH, 20, TFT_RED);
  tft.setCursor(leftPadding, 2, 2);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  switch (openedMenu) {
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

void renderContent() {
  tft.setCursor(leftPadding, 22, 2);
  tft.setTextSize(2);

  switch (openedMenu) {
    case MAIN_MENU:
      tft.setTextColor(TFT_WHITE, selectedItem == 0 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print(gameStarted ? "End game" : "Start game");

      tft.setCursor(leftPadding, 22 + tft.fontHeight() + 4, 2);  // 22 + top_bar, + 4 margin
      tft.setTextColor(gameStarted ? TFT_DARKGREY : TFT_WHITE, selectedItem == 1 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print("Settings");

      tft.setCursor(leftPadding, 22 + tft.fontHeight() * 2 + 8, 2);  // 22 + top_bar, + 4 margin (include last one)
      tft.setTextColor(gameStarted ? TFT_DARKGREY : TFT_WHITE, selectedItem == 2 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print("Information");

      break;
    case SETTINGS:
      tft.setTextColor(TFT_WHITE, selectedItem == 0 ? TFT_DARKGREEN : TFT_BLACK);
      tft.print("Timer");
      break;
    case SETTINGS_TIMER:
      char buffer[6];
      sprintf(buffer, "%02d:%02d", defaultTimer / 60, defaultTimer % 60);
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

void renderBottomBar() {
  tft.fillRect(0, LCD_HEIGHT - 20, LCD_WIDTH, LCD_HEIGHT, TFT_RED);
  tft.setTextFont(1);
  tft.setTextSize(1);

  if (openedMenu == MAIN_MENU) {
    switch (selectedItem) {
      case 1:
        tft.setCursor(leftPadding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("Change game behaviour");
        break;
      case 2:
        tft.setCursor(leftPadding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("Display connected modules");
        break;
      default:
        tft.setCursor(leftPadding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("F1-ENTER");
        tft.setCursor(leftPadding * 3 + tft.textWidth("F1-ENTER"), LCD_HEIGHT - 14);
        tft.setTextColor(gameStarted ? TFT_WHITE : TFT_DARKGREY);
        tft.print("F2-BACK");
        break;
    }
  }

  if (openedMenu == SETTINGS) {
    switch (selectedItem) {
      default:
        tft.setCursor(leftPadding, LCD_HEIGHT - 14);
        tft.setTextColor(TFT_WHITE);
        tft.print("F1-ENTER");
        tft.setCursor(leftPadding * 3 + tft.textWidth("F1-ENTER"), LCD_HEIGHT - 14);
        tft.print("F2-BACK");
        break;
    }
  }

  if (openedMenu == SETTINGS_TIMER) {
    tft.setCursor(leftPadding, LCD_HEIGHT - 14);
    tft.setTextColor(TFT_WHITE);
    tft.print("F1 <");
    tft.setCursor(leftPadding * 3 + tft.textWidth("F1 <"), LCD_HEIGHT - 14);
    tft.print("F3 >");
    tft.setCursor(leftPadding * 5 + tft.textWidth("F1 <") + tft.textWidth("F2 >"), LCD_HEIGHT - 14);
    tft.print("F2 - SAVE");
  }
}

void render() {
  if (!rerender && !rerenderContent) return;

  if (rerender) {
    tft.fillScreen(TFT_BLACK);
    selectedItem = 0;
  }

  switch (openedMenu) {
    case NONE:
      char buffer[6];
      sprintf(buffer, "%02d:%02d", timer / 60, timer % 60);

      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextFont(8);
      tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
      tft.print(buffer);
      break;
    default:
      renderTopBar();
      renderContent();
      renderBottomBar();
      break;
  }

  rerender = false;
  rerenderContent = false;
}

void handleButtons() {
  bool upState = digitalRead(TOP_BUTTON) == LOW;
  bool downState = digitalRead(BOTTOM_BUTTON) == LOW;
  bool enterState = digitalRead(ENTER_BUTTON) == LOW;
  bool backState = digitalRead(BACK_BUTTON) == LOW;

  if(upState && !upPressed) {
    tone(BUZZER_PIN, 1000);
    delay(50);
    noTone(BUZZER_PIN);

    if (openedMenu == MAIN_MENU) {
      selectedItem = (selectedItem == 0) ? 2 : selectedItem - 1;
      rerenderContent = true;
    } else if (openedMenu == SETTINGS_TIMER) {
      if(defaultTimer > 0){
        defaultTimer++;
      }
      rerenderContent = true;
    }
  }

  if (downState && !downPressed) {
    tone(BUZZER_PIN, 1000, 50);

    if (openedMenu == MAIN_MENU) {
      selectedItem = ++selectedItem % 3;
      rerenderContent = true;
    } else if (openedMenu == SETTINGS_TIMER) {
      if(defaultTimer > 0){
        defaultTimer--;
      }
      rerenderContent = true;
    }
  }

  if (enterState && !enterPressed) {
    if (openedMenu != MAIN_MENU || (openedMenu == MAIN_MENU && (!gameStarted && selectedItem != 0))) {
      tone(BUZZER_PIN, 1000, 50);
    }

    if (openedMenu == MAIN_MENU) {
      switch (selectedItem) {
        case 0:
          gameStarted = !gameStarted;
          openedMenu = gameStarted ? NONE : MAIN_MENU;
          rerender = true;

          if (gameStarted) {
            timer = defaultTimer;
            justStarted = true;

            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextFont(8);
            tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("123") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
            tft.fillScreen(TFT_BLACK);

            tft.print("3");
            tone(BUZZER_PIN, 1000, 100);
            delay(900);

            tft.print("2");
            tone(BUZZER_PIN, 1500, 100);
            delay(900);

            tft.print("1");
            tone(BUZZER_PIN, 2000, 100);
            delay(900);
          } else {
            timer = 0;
            broadcastPacket(0x1, NOT_STARTED);
          }
          break;
        case 1:
          openedMenu = SETTINGS;
          rerender = true;
          break;
        case 2:
          openedMenu = INFORMATION;
          rerender = true;
          break;
      }
    } else if (openedMenu == SETTINGS) {
      switch (selectedItem) {
        case 0:
          openedMenu = SETTINGS_TIMER;
          rerender = true;
          break;
      }
    } else if (openedMenu == NONE) {
      broadcastPacket(0x1, PAUSED);
      openedMenu = MAIN_MENU;
      rerender = true;
      lastOpened = millis();
    } else if (openedMenu == SETTINGS_TIMER) {
      openedMenu = SETTINGS;
      rerender = true;
    }
  }

  if (backState && !backPressed) {
    tone(BUZZER_PIN, 1000, 50);

    if (openedMenu == MAIN_MENU && gameStarted) {
      broadcastPacket(0x1, PLAYING);
      openedMenu = NONE;
      rerender = true;
    } else if (openedMenu == SETTINGS || openedMenu == INFORMATION) {
      openedMenu = MAIN_MENU;
      rerender = true;
    } else if (openedMenu == SETTINGS_TIMER) {
      defaultTimer++;
      rerenderContent = true;
    }
  }

  upPressed = upState;
  downPressed = downState;
  enterPressed = enterState;
  backPressed = backState;
}

void gameLogic() {
  int currentTime = millis();

  if (openedMenu != NONE && currentTime - lastOpened >= 10000) {
    broadcastPacket(0x1, PLAYING);
    lastOpened = 0;
    openedMenu = NONE;
    rerender = true;
  }

  if (openedMenu == NONE && currentTime - intervalTime >= 1000) {
    timer--;

    broadcastPacket(0x3, timer);

    intervalTime = millis();
    rerenderContent = true;

    if (timer <= 5) {
      tone(BUZZER_PIN, 1000 + 100 * (5 - timer), 50);
    }

    if (timer <= 0) {
      tft.fillScreen(TFT_BLACK);

      tone(BUZZER_PIN, 2000);
      delay(300);
      noTone(BUZZER_PIN);
      delay(100);
      tone(BUZZER_PIN, 2000);
      delay(300);
      noTone(BUZZER_PIN);
      delay(100);
      tone(BUZZER_PIN, 2000);
      delay(300);
      noTone(BUZZER_PIN);

      broadcastPacket(0x1, NOT_STARTED);

      while (digitalRead(ENTER_BUTTON) == HIGH) {
        tft.setTextColor(TFT_RED);
        tft.setTextFont(2);
        tft.setTextSize(2);
        tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("GAME OVER") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2) - 20);
        tft.print("GAME OVER");
        tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("Press 'Enter' to continue", 1) / 2), (LCD_HEIGHT / 2) + (tft.fontHeight() / 2) + 2, 1);
        tft.setTextColor(TFT_WHITE);
        tft.print("Press 'Enter' to continue");
        delay(50);
      }

      enterPressed = true;
      gameStarted = false;
      openedMenu = MAIN_MENU;
      rerender = true;
    }
  }
}

void setup() {
  delay(1000); // necessary, serial seems to break without it

  Serial.begin(115200);
  Serial.flush();

  tft.init();
  tft.setRotation(3);

  pinMode(TOP_BUTTON, INPUT_PULLUP);
  pinMode(BOTTOM_BUTTON, INPUT_PULLUP);
  pinMode(ENTER_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // set i2c pins for the controller
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  delay(2000);

  initializeDevices();

  for (auto module : modules) {
    Serial.print("Module at address 0x");
    Serial.print(module.id, HEX);
    Serial.print(" is ");
    Serial.println(module.active ? "online" : "offline");
  }

  rerender = true;

  broadcastPacket(0x1, NOT_STARTED);
}

void loop() {
  int currentTime = millis();

  handleButtons();

  if (justStarted) {
    justStarted = false;

    tone(BUZZER_PIN, 1000);
    delay(50);
    tone(BUZZER_PIN, 1500);
    delay(50);
    tone(BUZZER_PIN, 2000);
    delay(50);
    noTone(BUZZER_PIN);

    broadcastPacket(0x1, PLAYING);
  }

  if (currentTime - stateTimer > 100) {
    stateTimer = millis();
    refreshStates();

    for (Module module : modules) {
      if (module.state == STRIKE) {
        strike++;
        tone(BUZZER_PIN, 800, 500);
        break;
      }
    }

    Serial.print("[lo] strike: ");
    Serial.println(strike);

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }

  if (gameStarted) gameLogic();

  render();
}