#ifdef POLYCONTROL

#include "preset.hpp"

std::vector<presetStruct> presets;

uint8_t blockBuffer[PRESET_BLOCKSIZE];

// AUfbau der Daten:   1Block -> Allgemein Settings, 2Block Liste aller Presets, freie Blöcke ..,  presets

void updatePresetList() {
    EEPROM_SPI_ReadBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);

    presets.clear();

    for (uint32_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {
        presetStruct newEntry;

        if (((presetStruct *)blockBuffer)[i].usageState == PRESET_USED) {
            newEntry = ((presetStruct *)blockBuffer)[i];
            newEntry.ID = i;
            presets.push_back(newEntry);
        }
        else {
            newEntry.usageState = PRESET_FREE;
            newEntry.ID = i;
            presets.push_back(newEntry);
        }
    }
}

void removePreset(uint16_t blockID) {
    // write empty entry to table
    presetStruct newEntry;
    newEntry.usageState = PRESET_FREE;
    newEntry.name[0] = '\0';

    uint32_t address = TABLE_STARTADDRESS + blockID * sizeof(presetStruct);

    EEPROM_SPI_WriteBuffer((uint8_t *)&newEntry, address, sizeof(presetStruct));

    updatePresetList();
}

uint32_t freePresetID() {
    for (presetStruct p : presets) {
        if (p.usageState == PRESET_FREE)
            return p.ID;
    }
    return 0;
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
    newEntry.usageState = PRESET_USED;
    newEntry.ID = blockID;

    for (uint32_t i = 0; i < PRESET_NAMELENGTH; i++) {
        if (i < name.size()) {

            newEntry.name[i] = name.data()[i];
        }
        else {
            newEntry.name[i] = '\0';
        }
    }

    address = TABLE_STARTADDRESS + newEntry.ID * sizeof(presetStruct);

    EEPROM_SPI_WriteBuffer((uint8_t *)&newEntry, address, sizeof(presetStruct));

    updatePresetList();
}

void writeConfigBlock() {

    EEPROM_SPI_WriteBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);
}

uint8_t *readPreset(uint16_t blockID) {
    uint32_t address = blockID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;

    EEPROM_SPI_ReadBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    return blockBuffer;
}

uint8_t *readConfig() {

    EEPROM_SPI_ReadBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);

    return blockBuffer;
}

void writeLiveDataBlock() {

    EEPROM_SPI_WriteBuffer(blockBuffer, LIVEDATA_STARTADDRESS, LIVEDATA_BLOCKSIZE);
}

uint8_t *readLiveData() {

    EEPROM_SPI_ReadBuffer(blockBuffer, LIVEDATA_STARTADDRESS, LIVEDATA_BLOCKSIZE);

    return blockBuffer;
}
#endif