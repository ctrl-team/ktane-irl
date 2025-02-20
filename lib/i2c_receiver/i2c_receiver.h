#ifndef I2C_RECEIVER
#define I2C_RECEIVER

#include <Wire.h>

// I2C BUS Pins
#define SDA_PIN 0
#define SCL_PIN 1

#define START_ADDRESS 0x08 // we have to avoid reserved addresses
#define END_ADDRESS 0x16

enum ModuleState {
  PLAYING = 0x1,
  STRIKE = 0x2,
  SOLVED = 0x3,
  PAUSED = 0x4,
  NOT_STARTED = 0x5,
  STATE_UNKNOWN = 0xF
};

enum ModuleType {
  DEBUG_MODULE = 0x01,
  BUTTON_MODULE = 0x2,
  MODULE_UNKNOWN = 0xF
};

struct Module {
  uint8_t id;
  ModuleType type;
  ModuleState state;
  bool active;
};

class I2CReceiver {
  public:
    I2CReceiver(ModuleType moduleType);
  
    void begin();
    uint8_t getAddress();
    void receiveCallback(int numBytes);
    void requestCallback();
    bool checkAddressAvailability(uint8_t address);
    uint8_t requestAvailableAddress();

    ModuleType moduleType;
    ModuleState currentState;
    uint16_t timer;

  private:
    uint8_t deviceAddress;
    uint8_t selectedCommand;
    uint8_t receivedData;
};

#endif