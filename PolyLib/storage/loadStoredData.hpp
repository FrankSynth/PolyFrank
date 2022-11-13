
#pragma once

#include "stdint.h"
#include "string"
#include "vector"

#include "storage/storageDataStructure.hpp"

typedef enum { PRESET_FREE, PRESET_USED } USAGE_STATE;

typedef enum { LAYER_A, LAYER_B, LAYER_AB } LAYER_SELECT;

const uint32_t PRESET_NAMELENGTH = 32;

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

inline void storeToBlock(StorageBlock *buffer, StorageBlock &block, uint32_t &blockIndex) {
    if (block.id != 0xFF) {
        buffer[blockIndex++] = block;
    }
}

void updatePresetList();
void removePresetfromTable(presetStruct *preset);

void writePreset(uint32_t numBytes, presetStruct *preset, std::string name);
void writeConfig(uint32_t blockIndex);

void readConfig();
bool readPresetTable();
void readPreset(uint8_t presetID, LAYER_SELECT layer);

void loadPatchDatablockToLayer(uint8_t layerID, uint8_t sourceID, uint8_t targetID, float amount);

void loadSettingToGlobalSetting(uint8_t categoryID, uint8_t dataID, uint32_t data);
void loadAnalogDatablockToModule(uint8_t layerID, uint8_t moduleID, uint8_t dataID, int32_t data);
void loadDigitalDatablockToModule(uint8_t layerID, uint8_t moduleID, uint8_t dataID, int32_t data);
void loadSettingToLive(uint8_t dataID, int32_t data);
void loadSettingToArp(uint8_t layerID, uint8_t dataID, int32_t data);

void decodeBuffer();
bool checkBuffer();
void packBuffer(uint32_t numBytes);

void presetServiceRoutine();
