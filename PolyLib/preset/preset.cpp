#ifdef POLYCONTROL

#include "preset.hpp"

const static uint8_t BLOCKUSED = 0x0F;

std::vector<presetStruct> presets;

uint8_t blockBuffer[PRESET_BLOCKSIZE];

// AUfbau der Daten:   1Block -> Allgemein Settings, 2Block Liste aller Presets, freie Blöcke ..,  presets

void updatePresetList() {
    EEPROM_SPI_ReadBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);

    presets.clear();

    for (uint32_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {
        if (((presetStruct *)blockBuffer)[i].isFree == BLOCKUSED) {
            presetStruct newEntry = ((presetStruct *)blockBuffer)[i];
            presets.push_back(newEntry);
        }
    }
}

void removePreset(uint16_t blockID) {
    // write empty entry to table
    presetStruct newEntry;
    newEntry.isFree = 0;
    newEntry.name[PRESET_NAMELENGTH] = '\0';

    uint32_t address = TABLE_STARTADDRESS + blockID * sizeof(presetStruct);

    EEPROM_SPI_WriteBuffer((uint8_t *)&newEntry, address, sizeof(presetStruct));
}

std::vector<presetStruct> *getPresetList() {
    return &presets;
}

void initPreset() {
    EEPROM_SPI_INIT(&hspi6);
}
void writePresetBlock(uint16_t blockID, std::string name) {

    uint32_t address = PRESET_BLOCKSIZE + blockID * PRESET_BLOCKSIZE;

    EEPROM_SPI_WriteBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    // write new entry to table
    presetStruct newEntry;
    newEntry.isFree = BLOCKUSED;
    newEntry.ID = blockID;

    for (uint32_t i = 0; i < PRESET_NAMELENGTH - 1; i++) {
        if (i < name.size()) {

            newEntry.name[i] = name.data()[i];
        }
        else {
            newEntry.name[i] = ' ';
        }
    }
    newEntry.name[PRESET_NAMELENGTH] = '\0';

    address = TABLE_STARTADDRESS + newEntry.ID * sizeof(presetStruct);

    EEPROM_SPI_WriteBuffer((uint8_t *)&newEntry, address, sizeof(presetStruct));

    updatePresetList();
}

void writeConfigBlock(uint16_t blockID) {
    uint32_t address = CONFIG_STARTADDRESS;

    EEPROM_SPI_WriteBuffer(blockBuffer, address, CONFIG_BLOCKSIZE);
}

uint8_t *readPreset(uint16_t blockID) {
    uint32_t address = blockID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;

    EEPROM_SPI_ReadBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    return blockBuffer;
}

uint8_t *readConfig() {
    uint32_t address = CONFIG_STARTADDRESS;

    EEPROM_SPI_ReadBuffer(blockBuffer, address, CONFIG_BLOCKSIZE);

    return blockBuffer;
}
#endif