#include "i2c_manager.h"

I2CReceiver::I2CReceiver(ModuleType moduleType) : moduleType(moduleType), currentState(STATE_UNKNOWN) {}

void I2CReceiver::begin() {
  deviceAddress = requestAvailableAddress();

  Wire.begin(deviceAddress);
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  Wire.onReceive(receiveCallback);
  Wire.onRequest(requestCallback);
}

uint8_t getAddress() {
  return deviceAddress;
}

void I2CReceiver::receiveCallback(int numBytes) {
  selectedCommand = Wire.read();
  receivedData = 0;

  if (numBytes == 3) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();
    receivedData = (highByte << 8) | lowByte;
  }

  // these are kept separate from requestCallback because they arent excepted to return anything

  if (selectedCommand == 0x1) {
    if (currentState == SOLVED && receivedData != NOT_STARTED) return;

    currentState = static_cast<ModuleState>(receivedData);
    receivedData = 0x0;
    selectedCommand = 0xF;

    return;
  }

  if (selectedCommand == 0x3) {
    timer = receivedData;
    receivedData = 0x0;
    selectedCommand = 0xF;

    char buffer[6];
    sprintf(buffer, "%02d:%02d", timer / 60, timer % 60);
    
    Serial.print("New time: ");
    Serial.println(buffer);

    return;
  }
}

void I2CReceiver::requestCallback() {
  switch (selectedCommand) {
    case 0x2:
      Wire.write(currentState);
      if (currentState == STRIKE)
        currentState = PLAYING;
      break;
    case 0x4:
      Wire.write(moduleType);
      break;
    default:
      Wire.write(0xF);
  }

  selectedCommand = 0xF;
}

bool I2CReceiver::checkAddressAvailability(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() != 0; // returns true if address is free
}

uint8_t I2CReceiver::requestAvailableAddress() {
  for (uint8_t address = START_ADDRESS; address <= END_ADDRESS; address++) {
    if (checkAddressAvailability(address))
      return address;
  }

  return 0x0;
}