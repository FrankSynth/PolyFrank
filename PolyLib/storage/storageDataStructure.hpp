#pragma once

#include <string>
#include <vector>

typedef enum : uint8_t {
    STORE_START = 0x1,
    STORE_CATEGORY = 0x2,
    STORE_MODULE = 0x3,
    STORE_CONFIGSETTING = 0x4,
    STORE_DATADIGITAL = 0x5,
    STORE_DATAANALOG = 0x6,
    STORE_CHECKSUM = 0xF,
    STORE_LIVESETTING = 0x7,
    STORE_ARPSETTING = 0x8,
    STORE_PATCH = 0x9
} StoreTYPE;

typedef union {
    float asFloat;
    int32_t asInt;
    uint32_t asUInt;
} dataValueType;

typedef struct {
    StoreTYPE dataType;
    uint8_t id;
    dataValueType data;
} StorageBlock;

typedef struct {
    StoreTYPE dataType;
    uint8_t sourceModuleID;
    uint8_t sourceOutputID;
    uint8_t targetModuleID;
    uint8_t targetInputID;
    float amount;
} patchStorageBlock;

// Store Structure

const uint32_t BLOCKBUFFERSIZE = 4096;

const uint32_t CONFIG_BLOCKSIZE = 1024;

const uint32_t TABLE_BLOCKSIZE = 3072;

// const uint32_t PRESET_SIZE = 4096;
const uint32_t PRESET_BLOCKSIZE = 4096;

const uint32_t CONFIG_STARTADDRESS = 0x00;
const uint32_t TABLE_STARTADDRESS = CONFIG_STARTADDRESS + CONFIG_BLOCKSIZE;
const uint32_t PRESET_STARTADDRESS = CONFIG_STARTADDRESS + CONFIG_BLOCKSIZE + TABLE_BLOCKSIZE;

const uint32_t EEPROM_SIZE = 131072;
const uint32_t PRESET_NUMBERBLOCKS = (EEPROM_SIZE - CONFIG_BLOCKSIZE - TABLE_BLOCKSIZE) / PRESET_BLOCKSIZE;

extern uint8_t blockBuffer[BLOCKBUFFERSIZE];

uint8_t *readConfigBlock();
uint8_t *readPresetBlock(uint32_t presetID);
uint8_t *readPresetTableBlock();

void writePresetBlock(uint32_t presetID);
void writePresetTableBlock();
void writeConfigBlock();

void clearEEPROM();