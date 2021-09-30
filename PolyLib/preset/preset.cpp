#ifdef POLYCONTROL

#include "preset.hpp"

std::vector<presetStruct> presets;
std::vector<presetStruct *> presetsSorted;
presetStruct *freePreset;

uint32_t saveID = 0;

uint8_t blockBuffer[PRESET_BLOCKSIZE];

bool comparePtrPresetStruct(presetStruct *a, presetStruct *b) {
    return (*a > *b);
}

// AUfbau der Daten:   1Block -> Allgemein Settings, 2Block Liste aller Presets, freie Blöcke ..,  presets

void updatePresetList() {
    static uint8_t init = 1;

    if (init) {
        println("build preset buffer");
        for (size_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {
            presetStruct newEntry;
            newEntry.usageState = PRESET_FREE;
            newEntry.storageID = i;
            newEntry.saveCounterID = 0;

            presets.push_back(newEntry);
        }
        init = 0;
    }

    presetsSorted.clear();

    // println("preset Size: ", presets.size());

    EEPROM_SPI_ReadBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);

    for (uint32_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {

        if (((presetStruct *)blockBuffer)[i].usageState == PRESET_USED) {
            presets[i] = ((presetStruct *)blockBuffer)[i];
            presetsSorted.push_back(&presets[i]);
        }
        else {
            presets[i].usageState = PRESET_FREE;
        }
    }

    freePreset = nullptr;
    for (size_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {
        if (presets[i].usageState == PRESET_FREE) {
            freePreset = &presets[i];
        }
    }
    if (freePreset == nullptr)
        PolyError_Handler("No Free Save slots Available");

    if (presetsSorted.size()) {
        std::sort(presetsSorted.begin(), presetsSorted.end(), comparePtrPresetStruct);
        saveID = presetsSorted.front()->saveCounterID + 1;
    }
}

void removePreset(presetStruct *preset) {
    // write empty entry to table
    preset->usageState = PRESET_FREE;

    uint32_t address = TABLE_STARTADDRESS + preset->storageID * sizeof(presetStruct);

    EEPROM_SPI_WriteBuffer((uint8_t *)preset, address, sizeof(presetStruct));

    updatePresetList();
}

std::vector<presetStruct> *getPresetList() {
    return &presets;
}

void initPreset() {
    EEPROM_SPI_INIT(&hspi6);
}

void writePresetBlock(presetStruct *preset, std::string name) {

    if (preset == nullptr){
        updatePresetList();
        return;
    }

    uint32_t address = PRESET_BLOCKSIZE + preset->storageID * PRESET_BLOCKSIZE;

    EEPROM_SPI_WriteBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    // write new entry to table
    preset->usageState = PRESET_USED;
    preset->saveCounterID = saveID;

    for (uint32_t i = 0; i < PRESET_NAMELENGTH; i++) {
        if (i < name.size()) {

            preset->name[i] = name.data()[i];
        }
        else {
            preset->name[i] = '\0';
        }
    }

    address = TABLE_STARTADDRESS + preset->storageID * sizeof(presetStruct);

    EEPROM_SPI_WriteBuffer((uint8_t *)preset, address, sizeof(presetStruct));

    updatePresetList();
}

void writeConfigBlock() {
    EEPROM_SPI_WriteBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);
}

uint8_t *readPreset(presetStruct *preset) {
    uint32_t address = preset->storageID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;

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