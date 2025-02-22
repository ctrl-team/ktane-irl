#include "bus_receiver.h"

BusReceiver* BusReceiver::instancePointer = nullptr;

BusReceiver* BusReceiver::getInstance() {
  return instancePointer;
}

void BusReceiver::begin(ModuleType type) {
  instancePointer = this;

  this->type = type;

  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);

  timer = 0;
  state = NOT_STARTED;

  moduleAddress = requestAvailableAddress();

  Wire.begin(moduleAddress);
  Wire.onReceive(staticReceiveCallback);
  Wire.onRequest(staticRequestCallback);
}

void BusReceiver::receiveCallback(int numBytes) {
  requestedCommand = Wire.read();

  receivedData = 0;

  if (numBytes == 3) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();
    receivedData = (highByte << 8) | lowByte;
  }

  if (requestedCommand == 0x1) {
    if (state == SOLVED && receivedData != NOT_STARTED) return;
    state = static_cast<ModuleState>(receivedData);
    receivedData = 0x0;
    requestedCommand = 0xF;
  }

  if (requestedCommand == 0x3) {
    timer = receivedData;
    receivedData = 0x0;
    requestedCommand = 0xF;
  }
}

void BusReceiver::requestCallback() {
  switch (requestedCommand) {
    case 0x2:
      Wire.write(state);
      if (state == STRIKE)
        state = PLAYING;
      break;
    case 0x4:
      Wire.write(type);
      break;
    default:
      Wire.write(0xF);
  }

  requestedCommand = 0xF;
}

void BusReceiver::staticReceiveCallback(int numBytes) {
  BusReceiver* instance = getInstance();
  if (instance) {
    instance->receiveCallback(numBytes);
  }
}

void BusReceiver::staticRequestCallback() {
  BusReceiver* instance = getInstance();
  if (instance) {
    instance->requestCallback();
  }
}

bool BusReceiver::checkAddressAvailability(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() != 0;
}

uint8_t BusReceiver::requestAvailableAddress() {
  for (uint8_t address = START_ADDRESS; address <= END_ADDRESS; address++) {
    if (checkAddressAvailability(address))
      return address;
  }

  return 0x0;
}