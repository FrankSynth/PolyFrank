#pragma once

#include <string>
#include <vector>

typedef enum { PRESET_FREE, PRESET_USED } USAGE_STATE;

extern const uint8_t PRESETBLOCKUSED;

const uint32_t CONFIG_BLOCKSIZE = 1024;

const uint32_t TABLE_BLOCKSIZE = 3072;

const uint32_t LIVEDATA_BLOCKSIZE = 128;
const uint32_t PRESET_SIZE = 1280;
const uint32_t PRESET_BLOCKSIZE = PRESET_SIZE + PRESET_SIZE + LIVEDATA_BLOCKSIZE;

const uint32_t PRESET_NAMELENGTH = 32;

const uint32_t CONFIG_STARTADDRESS = 0x00;
const uint32_t TABLE_STARTADDRESS = CONFIG_STARTADDRESS + CONFIG_BLOCKSIZE;
const uint32_t PRESET_STARTADDRESS = CONFIG_STARTADDRESS + CONFIG_BLOCKSIZE + TABLE_BLOCKSIZE;

const uint32_t EEPROM_SIZE = 131072;

const uint32_t PRESET_NUMBERBLOCKS = (EEPROM_SIZE - CONFIG_BLOCKSIZE - TABLE_BLOCKSIZE) / PRESET_BLOCKSIZE;

typedef struct presetStruct {
    char name[PRESET_NAMELENGTH];
    uint8_t categoryID;
    uint8_t storageID;

    uint32_t saveCounterID;
    USAGE_STATE usageState;

    bool operator<(const presetStruct &str) const { return (saveCounterID < str.saveCounterID); }
    bool operator>(const presetStruct &str) const { return (saveCounterID > str.saveCounterID); }
} presetStruct;

extern std::vector<presetStruct> presets;
extern std::vector<presetStruct *> presetsSorted;
extern presetStruct *freePreset;
extern uint8_t blockBuffer[PRESET_BLOCKSIZE];

std::vector<presetStruct> *getPresetList();

void updatePresetList();
void writePresetBlock(presetStruct *preset, std::string name);
void writeConfigBlock();

uint8_t *readConfig();
uint8_t *readPreset(presetStruct *preset);

void removePreset(presetStruct *preset);

void clearEEPROM();