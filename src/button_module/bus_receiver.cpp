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
  if (numBytes <= 0) return; // sanity check

  requestedCommand = Wire.read();
  numBytes--;

  if (requestedCommand == 0x7) {
    Wire.readBytes(configuration.serial, numBytes);

    Serial.print("Serial number received: ");
    Serial.println(configuration.serial);
    
    return;
  }

  // receiving a 16-bit integer
  if (numBytes == 2) {
    uint8_t buffer[2];
    Wire.readBytes(buffer, 2);
    uint16_t value = (buffer[0] << 8) | buffer[1];

    switch (requestedCommand) {
      case 0x5:
        configuration.flags = value;
        Serial.print("Flags set: 0b");
        Serial.println(configuration.flags, BIN);
        break;

      case 0x6:
        configuration.ports = value;
        Serial.print("Ports set: 0b");
        Serial.println(configuration.ports, BIN);
        break;

      case 0x1:
        if (state == SOLVED && value != NOT_STARTED) return;
        state = static_cast<ModuleState>(value);
        break;

      case 0x3:
        timer = value;
        break;

      default:
        Serial.print("Unknown command: 0x");
        Serial.println(requestedCommand, HEX);
        break;
    }

    return;
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