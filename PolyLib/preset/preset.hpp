#pragma once

#include "hardware/M95M01.hpp"
#include "spi.h"
#include <string>
#include <vector>

extern const uint8_t PRESETBLOCKUSED;

const uint32_t CONFIG_BLOCKSIZE = 1024;
const uint32_t TABLE_BLOCKSIZE = 2048;
const uint32_t PRESET_BLOCKSIZE = 3072;

const uint32_t PRESET_NAMELENGTH = 32;

const uint32_t CONFIG_STARTADDRESS = 0x00;
const uint32_t TABLE_STARTADDRESS = 0x00 + CONFIG_BLOCKSIZE;
const uint32_t PRESET_STARTADDRESS = 0x00 + CONFIG_BLOCKSIZE + TABLE_BLOCKSIZE;

const uint32_t EEPROM_SIZE = 131072;

const uint32_t PRESET_NUMBERBLOCKS = (EEPROM_SIZE - CONFIG_BLOCKSIZE - TABLE_BLOCKSIZE) / PRESET_BLOCKSIZE;

typedef struct {
    char name[PRESET_NAMELENGTH];
    uint8_t ID;
    uint8_t usageState;

} presetStruct;

extern std::vector<presetStruct> presets;
extern uint8_t blockBuffer[PRESET_BLOCKSIZE];

std::vector<presetStruct> *getPresetList();

void updatePresetList();

void writePresetBlock(uint16_t blockID, std::string name);
void writeConfigBlock();

uint8_t *readConfig();
uint8_t *readPreset(uint16_t blockID);

void initPreset();
void removePreset(uint16_t blockID);