#pragma once

#include "stdint.h"

void printHelp();
void printStatus();
void printDeviceManager();
void printPreset(uint8_t index);

void sendDataACK();
void sendDataNACK();

bool receiveDataBlock(uint8_t *buffer, uint32_t length);

void COMmunicateISR();
void supervisor();

void shutdown();