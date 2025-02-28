#include "bus_controller.h"

ModuleType BusController::whoAreYou(uint8_t targetAddress) {
  if (sendPacket(targetAddress, 0x04))
    return static_cast<ModuleType>(receiveByte(targetAddress));
  return MODULE_UNKNOWN;
}

ModuleState BusController::getState(uint8_t targetAddress) {
  if (sendPacket(targetAddress, 0x02))
    return static_cast<ModuleState>(receiveByte(targetAddress));
  return STATE_UNKNOWN;
}

void BusController::refreshStates() {
  justStriked = false;
  justSolved = false;

  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    if (!modules[module].active) continue;

    ModuleState moduleState = getState(modules[module].id);
    if (moduleState == STATE_UNKNOWN) continue;

    if (moduleState == STRIKE) {
      justStriked = true;
      strikes++;
    }

    if (modules[module].state != SOLVED && moduleState == SOLVED) {
      justSolved = true;
      solved++;
    }

    modules[module].state = moduleState;

    Serial.print("[rs] Address 0x");
    Serial.print(modules[module].id, HEX);
    Serial.print(" state is 0x");
    Serial.println(modules[module].state, HEX);
  }
}

void BusController::initializeDevices() {
  moduleCount = 0;

  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    Serial.print("[id] Scanning address 0x");
    Serial.println(module, HEX);

    if (!checkAddressAvailability(module)) {
      Serial.print("[id] Skipping address 0x");
      Serial.println(module, HEX);
      continue;
    }

    modules[module].id = module;
    modules[module].type = MODULE_UNKNOWN;
    modules[module].active = false;

    ModuleType moduleType = whoAreYou(module);
    if (moduleType == MODULE_UNKNOWN) continue;

    modules[module].type = moduleType;
    modules[module].active = true;

    ModuleState moduleState = getState(module);
    if (moduleState == STATE_UNKNOWN) continue;

    modules[module].state = moduleState;

    moduleCount++;

    Serial.print("[id] Address 0x");
    Serial.print(module, HEX);
    Serial.print(" identified itself as type 0x");
    Serial.println(moduleType, HEX);
  }
}

void BusController::updateState(ModuleState state) {
  this->state = state;
  broadcastPacket(0x1, state);
}

void BusController::updateTimer(uint16_t timer) {
  broadcastPacket(0x3, timer);
}

void BusController::begin() {
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  state = NOT_STARTED;
  strikes = 0;
  solved = 0;

  initializeDevices();

  for (auto module : modules) {
    Serial.print("Module at address 0x");
    Serial.print(module.id, HEX);
    Serial.print(" is ");
    Serial.println(module.active ? "online" : "offline");
  }

  broadcastPacket(0x1, state);
}

bool BusController::checkAddressAvailability(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

uint8_t BusController::receiveByte(uint8_t targetAddress) {
  Wire.requestFrom(targetAddress, 1);

  uint32_t startTime = millis();
  while (!Wire.available()) {
    // 80ms timeout
    if (millis() - startTime > 80) {
      Serial.print("Timeout waiting for response from 0x");
      Serial.println(targetAddress, HEX);
      return 0xF;
    }
    delay(1); // allow CPU time for other tasks
  }

  return Wire.read();
}

bool BusController::sendPacket(uint8_t targetAddress, uint8_t command) {
  Wire.beginTransmission(targetAddress);
  Wire.write(command);

  // non-zero means error
  if (Wire.endTransmission() != 0) {
    Serial.print("Failed to send 0x");
    Serial.print(command, HEX);
    Serial.print(" request to address 0x");
    Serial.println(targetAddress, HEX);
    return false;
  }

  return true;
}

bool BusController::sendPacket(uint8_t targetAddress, uint8_t command, uint16_t data) {
  Wire.beginTransmission(targetAddress);
  Wire.write(command);
  Wire.write((uint8_t)(data >> 8)); // high byte
  Wire.write((uint8_t)(data & 0xFF)); // low byte

  // non-zero means error
  if (Wire.endTransmission() != 0) {
    Serial.print("Failed to send 0x");
    Serial.print(command, HEX);
    Serial.print(" with data 0x");
    Serial.print(data, HEX);
    Serial.print(" to address 0x");
    Serial.println(targetAddress, HEX);
    return false;
  }

  return true;
}

void BusController::broadcastPacket(uint8_t command, uint16_t data) {
  for (int module = START_ADDRESS; module < END_ADDRESS; module++) {
    if (!checkAddressAvailability(module)) continue;
    sendPacket(module, command, data);
  }
}