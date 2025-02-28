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

enum Menus {
  NONE,
  MAIN_MENU,
  SETTINGS,
  SETTINGS_TIMER,
  SETTINGS_BATTERIES,
  SETTINGS_PORTS,
  SETTINGS_FLAGS,
  INFORMATION
};

TFT_eSPI tft = TFT_eSPI();
BusController controller;

bool rerender = true;
bool rerenderContent = true;
Menus openedMenu = MAIN_MENU;
int selectedItem = 0;

int batteries = 0;
int flags = 0;
int ports = 0;

// timing variables
int lastOpened = 0;
int intervalTime = 0;
int stateTimer = 0;

bool upPressed = false;
bool downPressed = false;
bool enterPressed = false;
bool backPressed = false;

bool justStarted = false;
uint16_t timer = 120; // just enough time for 18 hours gameplay
uint16_t defaultTimer = 300; // 5 minutes
int leftPadding = 2;

void renderTopBar() {
  tft.fillRect(0, 0, LCD_WIDTH, 20, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(leftPadding, 2, 2);

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
    case SETTINGS_BATTERIES:
      tft.println("Number of batteries");
      break;
    case SETTINGS_FLAGS:
      tft.println("Number of flags");
      break;
    case SETTINGS_PORTS:
      tft.println("Number of ports");
      break;
    case INFORMATION:
      tft.println("Information");
      break;
  }
}

void defaultBottomBar() {
  int x = tft.textWidth("F1 - UP");
  int y = tft.textWidth("F2 - DOWN");
  int z = tft.textWidth("F3 - ENTER");

  tft.print("F1 - UP");
  tft.setCursor(leftPadding * 3 + x, LCD_HEIGHT - 14);
  tft.print("F2 - DOWN");
  tft.setCursor(leftPadding * 5 + x + y, LCD_HEIGHT - 14);
  tft.print("F3 - ENTER");
  tft.setCursor(leftPadding * 7 + x + y + z, LCD_HEIGHT - 14);
  tft.setTextColor(openedMenu == MAIN_MENU ? controller.state == PAUSED ? TFT_WHITE : TFT_DARKGREY : TFT_WHITE);
  tft.print("F4 - BACK");
}

void renderBottomBar() {
  tft.fillRect(0, LCD_HEIGHT - 20, LCD_WIDTH, LCD_HEIGHT, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(leftPadding, LCD_HEIGHT - 14, 1);

  if (openedMenu == MAIN_MENU) {
    switch (selectedItem) {
      case 1:
        tft.print("Change game behaviour");
        break;
      case 2:
        tft.print("Display connected modules");
        break;
      default: {
        defaultBottomBar();
        break;
      }
    }

    return;
  }

  if (
    openedMenu == SETTINGS_TIMER ||
    openedMenu == SETTINGS_BATTERIES ||
    openedMenu == SETTINGS_FLAGS ||
    openedMenu == SETTINGS_PORTS
  ) {
    tft.print("F1 +");
    tft.setCursor(leftPadding * 3 + tft.textWidth("F1 +"), LCD_HEIGHT - 14);
    tft.print("F2 -");
    tft.setCursor(leftPadding * 5 + tft.textWidth("F1 +") + tft.textWidth("F2 -"), LCD_HEIGHT - 14);
    tft.print("F3 - SAVE");

    return;
  }

  defaultBottomBar();
}

void renderContent() {
  tft.setCursor(leftPadding, 22, 2);

  if (openedMenu == MAIN_MENU) {
    tft.setTextColor(TFT_WHITE, selectedItem == 0 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print(controller.state == PAUSED ? "End game" : "Start game");

    tft.setCursor(leftPadding, 22 + tft.fontHeight() + 4, 2);
    tft.setTextColor(controller.state == PAUSED ? TFT_DARKGREY : TFT_WHITE, selectedItem == 1 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Settings");

    tft.setCursor(leftPadding, 22 + tft.fontHeight() * 2 + 8, 2);
    tft.setTextColor(controller.state == PAUSED ? TFT_DARKGREY : TFT_WHITE, selectedItem == 2 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Information");
  }

  if (openedMenu == SETTINGS) {
    tft.setTextColor(TFT_WHITE, selectedItem == 0 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Timer");

    tft.setCursor(leftPadding, 22 + tft.fontHeight() + 4, 2);
    tft.setTextColor(TFT_WHITE, selectedItem == 1 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Batteries");

    tft.setCursor(leftPadding, 22 + tft.fontHeight() * 2 + 8, 2);
    tft.setTextColor(TFT_WHITE, selectedItem == 2 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Flags");

    tft.setCursor(leftPadding, 22 + tft.fontHeight() * 3 + 12, 2);
    tft.setTextColor(TFT_WHITE, selectedItem == 3 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Ports");

    tft.setCursor(leftPadding, 22 + tft.fontHeight() * 4 + 16, 2);
    tft.setTextColor(TFT_WHITE, selectedItem == 4 ? TFT_DARKGREEN : TFT_BLACK);
    tft.print("Randomize");
  }

  if (openedMenu == SETTINGS_TIMER) {
    char buffer[6];
    sprintf(buffer, "%02d:%02d", defaultTimer / 60, defaultTimer % 60);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(6);
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
    tft.print(buffer);
  }

  if (openedMenu == SETTINGS_BATTERIES) {
    char buffer[3];
    sprintf(buffer, "%02d", batteries);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(6);
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
    tft.print(buffer);
  }

  if (openedMenu == SETTINGS_FLAGS) {
    char buffer[3];
    sprintf(buffer, "%02d", flags);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(6);
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
    tft.print(buffer);
  }

  if (openedMenu == SETTINGS_PORTS) {
    char buffer[2];
    sprintf(buffer, "%d", ports);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(6);
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth(buffer) / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
    tft.print(buffer);
  }

  if (openedMenu == INFORMATION) {
    if (controller.modules == 0) {
      tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("NO MODULES FOUND") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
      tft.setTextColor(TFT_WHITE);
      tft.print("NO MODULES FOUND");
      return;
    }

    // int padding = 1;

    // for (Module module : controller.modules) {
    //   tft.setCursor(leftPadding, 22 + tft.fontHeight() * padding + 2 * ++padding, 2);
    // }
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

void gameOver() {
  tft.fillScreen(TFT_BLACK);

  controller.updateState(NOT_STARTED);

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

  while (digitalRead(ENTER_BUTTON) == HIGH) {
    tft.setTextColor(TFT_RED);
    tft.setTextFont(2);
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("GAME OVER") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2) - 20);
    tft.print("GAME OVER");
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("Press 'Enter' to continue", 1) / 2), (LCD_HEIGHT / 2) + (tft.fontHeight() / 2) + 2, 1);
    tft.setTextColor(TFT_WHITE);
    tft.print("Press 'Enter' to continue");
    delay(50);
  }

  controller.solved = 0;
  controller.strikes = 0;
  timer = 0;
  enterPressed = true;
  openedMenu = MAIN_MENU;
  rerender = true;
}

void gameWon() {
  tft.fillScreen(TFT_BLACK);

  tone(BUZZER_PIN, 2000);
  delay(300);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, 1500);
  delay(300);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, 1000);
  delay(300);
  noTone(BUZZER_PIN);

  while (digitalRead(ENTER_BUTTON) == HIGH) {
    tft.setTextColor(TFT_GREEN);
    tft.setTextFont(2);
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("GAME WON") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2) - 20);
    tft.print("GAME WON");
    tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("Press 'Enter' to continue", 1) / 2), (LCD_HEIGHT / 2) + (tft.fontHeight() / 2) + 2, 1);
    tft.setTextColor(TFT_WHITE);
    tft.print("Press 'Enter' to continue");
    delay(50);
  }

  controller.updateState(NOT_STARTED);

  controller.solved = 0;
  controller.strikes = 0;
  timer = 0;
  enterPressed = true;
  openedMenu = MAIN_MENU;
  rerender = true;
}

void startGame() {
  controller.updateState(PLAYING);
  timer = defaultTimer;
  justStarted = true;

  openedMenu = NONE;
  rerender = true;

  randomSeed(analogRead(A0));

  controller.configuration.randomize(flags, ports, batteries);
  // Serial.print("Flags set: 0b");
  // Serial.println(controller.configuration.flags, BIN);
  // Serial.print("Ports set: 0b");
  // Serial.println(controller.configuration.ports, BIN);
  // Serial.print("Serial number: ");
  // Serial.println(controller.configuration.serial);

  controller.sendConfiguration();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(8);
  tft.setCursor((LCD_WIDTH / 2) - (tft.textWidth("321") / 2), (LCD_HEIGHT / 2) - (tft.fontHeight() / 2));
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
}

void handleButtons() {
  bool upState = digitalRead(TOP_BUTTON) == LOW;
  bool downState = digitalRead(BOTTOM_BUTTON) == LOW;
  bool enterState = digitalRead(ENTER_BUTTON) == LOW;
  bool backState = digitalRead(BACK_BUTTON) == LOW;

  if (upState && !upPressed) {
    tone(BUZZER_PIN, 1000, 50);

    if (openedMenu == MAIN_MENU)
      selectedItem = (selectedItem == 0) ? 2 : selectedItem - 1;

    if (openedMenu == SETTINGS)
      selectedItem = (selectedItem == 0) ? 4 : selectedItem - 1;

    if (openedMenu == SETTINGS_TIMER)
      defaultTimer < sizeof(uint16_t) && defaultTimer++;

    if (openedMenu == SETTINGS_BATTERIES)
      batteries <= 10 && batteries++;

    if (openedMenu == SETTINGS_FLAGS)
      flags <= 11 && flags++;

    if (openedMenu == SETTINGS_PORTS)
      ports <= 6 && ports++;

    rerenderContent = true;
  }

  if (downState && !downPressed) {
    tone(BUZZER_PIN, 1000, 50);

    if (openedMenu == MAIN_MENU)
      selectedItem = ++selectedItem % 3;
    
    if (openedMenu == SETTINGS)
      selectedItem = ++selectedItem % 5;

    if (openedMenu == SETTINGS_TIMER)
      defaultTimer > 0 && defaultTimer--;

    if (openedMenu == SETTINGS_BATTERIES)
      batteries > 0 && batteries--;

    if (openedMenu == SETTINGS_FLAGS)
      flags > 0 && flags--;

    if (openedMenu == SETTINGS_PORTS)
      ports > 0 && ports--;

    rerenderContent = true;
  }

  if (enterState && !enterPressed) {
    if (openedMenu != MAIN_MENU || (openedMenu == MAIN_MENU && (controller.state != PLAYING && selectedItem != 0))) {
      tone(BUZZER_PIN, 1000, 50);
    }

    if (openedMenu == MAIN_MENU) {
      switch (selectedItem) {
        case 0:
          if (controller.state == PAUSED)
            gameOver();
          else
            startGame();
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

      return;
    }

    if (openedMenu == SETTINGS) {
      switch (selectedItem) {
        case 0:
          openedMenu = SETTINGS_TIMER;
          rerender = true;
          break;
        case 1:
          openedMenu = SETTINGS_BATTERIES;
          rerender = true;
          break;
        case 2:
          openedMenu = SETTINGS_FLAGS;
          rerender = true;
          break;
        case 3:
          openedMenu = SETTINGS_PORTS;
          rerender = true;
          break;
        case 4:
          randomSeed(analogRead(A1));
          batteries = random(0, 10);
          flags = random(0, 11);
          ports = random(0, 6);
          break;
      }

      return;
    }
    
    if (
      openedMenu == SETTINGS_TIMER ||
      openedMenu == SETTINGS_BATTERIES ||
      openedMenu == SETTINGS_FLAGS ||
      openedMenu == SETTINGS_PORTS
    ) {
      openedMenu = SETTINGS;
      rerender = true;
      return;
    }

    if (openedMenu == NONE) {
      controller.updateState(PAUSED);
      openedMenu = MAIN_MENU;
      rerender = true;
      lastOpened = millis();
      return;
    }
  }

  if (backState && !backPressed) {
    tone(BUZZER_PIN, 1000, 50);

    if (openedMenu == MAIN_MENU && controller.state == PAUSED) {
      controller.updateState(PLAYING);

      openedMenu = NONE;
      rerender = true;
    }

    if (openedMenu == SETTINGS || openedMenu == INFORMATION) {
      openedMenu = MAIN_MENU;
      rerender = true;
    }

    if (openedMenu == SETTINGS_TIMER) {
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
    controller.updateState(PLAYING);

    lastOpened = 0;
    openedMenu = NONE;
    rerender = true;
  }

  if (openedMenu == NONE && currentTime - intervalTime >= 1000) {
    timer--;

    controller.updateTimer(timer);

    intervalTime = millis();
    rerenderContent = true;

    if (timer <= 5)
      tone(BUZZER_PIN, 1000 + 100 * (5 - timer), 50);

    if (timer <= 0) gameOver();
  }

  if (controller.strikes > 2)
    gameOver();

  if (controller.solved >= controller.moduleCount)
    gameWon();
}

void setup() {
  delay(1000); // necessary, serial seems to break without it

  Serial.begin(115200);
  Serial.flush();

  tft.init();
  tft.setRotation(3);

  controller.begin();

  pinMode(TOP_BUTTON, INPUT_PULLUP);
  pinMode(BOTTOM_BUTTON, INPUT_PULLUP);
  pinMode(ENTER_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  rerender = true;
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
  }

  if (currentTime - stateTimer > 100) {
    stateTimer = millis();
    controller.refreshStates();

    if (controller.justStriked) {
      tone(BUZZER_PIN, 800, 500);

      Serial.print("[lo] strike: ");
      Serial.println(controller.strikes);
    }

    if (controller.justSolved) {
      tone(BUZZER_PIN, 1000);
      delay(50);
      tone(BUZZER_PIN, 1500);
      delay(50);
      tone(BUZZER_PIN, 2000);
      delay(50);
      noTone(BUZZER_PIN);

      Serial.print("[lo] solved: ");
      Serial.println(controller.solved);
    }
  }

  if (controller.state == PLAYING) gameLogic();

  render();
}