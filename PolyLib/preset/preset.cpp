#ifdef POLYCONTROL

#include "preset.hpp"

// AUfbau der Daten:   1Block -> Allgemein Settings, 2Block Liste aller Presets, freie Blöcke ..,  presets

void updatePresetList() {
    EEPROM_SPI_ReadBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);

    presets.clear();

    for (uint32_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {
        if (((preset *)blockBuffer)[i].isFree == 0) {
            preset newEntry = ((preset *)blockBuffer)[i];
            presets.push_back(newEntry);
        }
    }
}

std::vector<preset> *getPresetList() {
    return &presets;
}

void writePresetBlock(uint16_t blockID, char name[32]) {
    uint32_t address = blockID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;

    EEPROM_WritePage(blockBuffer, address, PRESET_BLOCKSIZE);

    // write new entry to table
    preset newEntry;
    newEntry.isFree = 0;
    newEntry.ID = blockID;

    for (uint32_t i = 0; i < PRESET_NAMELENGTH; i++) {
        newEntry.name[i] = name[i];
    }

    EEPROM_SPI_WriteBuffer(blockBuffer, TABLE_STARTADDRESS + newEntry.ID * sizeof(preset), sizeof(preset));

    updatePresetList();
}

void writeConfigBlock(uint16_t blockID) {
    uint32_t address = CONFIG_STARTADDRESS;

    EEPROM_WritePage(blockBuffer, address, CONFIG_BLOCKSIZE);
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