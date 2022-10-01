#ifdef POLYCONTROL

#include "preset.hpp"
#include "datacore/datalocation.hpp"
#include "hardware/device.hpp"

extern M95M01 eeprom;

std::vector<presetStruct> presets;
std::vector<presetStruct *> presetsSorted;
presetStruct *freePreset;

uint32_t saveID = 0;

RAM1 uint8_t blockBuffer[PRESET_BLOCKSIZE];

RAM3_DMA volatile uint8_t presetDMABlockBuffer[PRESET_BLOCKSIZE];

bool comparePtrPresetStruct(presetStruct *a, presetStruct *b) {
    return (*a > *b);
}

// AUfbau der Daten:   1Block -> Allgemein Settings, 2Block Liste aller Presets, freie Blöcke ..,  presets

void updatePresetList() {
    static uint8_t init = 1;

    if (init) {

        presets.reserve(PRESET_NUMBERBLOCKS);

        for (uint32_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {
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

    eeprom.SPI_ReadBuffer(blockBuffer, TABLE_STARTADDRESS, TABLE_BLOCKSIZE);

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
            break;
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

    eeprom.SPI_WriteBuffer((uint8_t *)preset, address, sizeof(presetStruct));

    updatePresetList();
}

std::vector<presetStruct> *getPresetList() {
    return &presets;
}

void writePresetBlock(presetStruct *preset, std::string name) {
    println("INFO || write preset: ", name, " ID: ", preset->storageID);

    if (preset == nullptr) {
        updatePresetList();
        return;
    }

    uint32_t address = PRESET_STARTADDRESS + preset->storageID * PRESET_BLOCKSIZE;

    eeprom.SPI_WriteBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    // write new entry to table
    if (preset->usageState == PRESET_FREE) {
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
    }
    address = TABLE_STARTADDRESS + preset->storageID * sizeof(presetStruct);

    eeprom.SPI_WriteBuffer((uint8_t *)preset, address, sizeof(presetStruct));

    updatePresetList();
}

void writeConfigBlock() {
    eeprom.SPI_WriteBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);
}

uint8_t *readPreset(presetStruct *preset) {

    uint32_t address = preset->storageID * PRESET_BLOCKSIZE + PRESET_STARTADDRESS;

    eeprom.SPI_ReadBuffer(blockBuffer, address, PRESET_BLOCKSIZE);

    return blockBuffer;
}

uint8_t *readConfig() {

    eeprom.SPI_ReadBuffer(blockBuffer, CONFIG_STARTADDRESS, CONFIG_BLOCKSIZE);

    return blockBuffer;
}

void clearEEPROM() { // CLEAR EEPROM WHEN CORRUPTED

    println("INFO | CLEAR EEPROM .... START");

    uint8_t buffer[2048];
    uint32_t data = 0x00;

    fastMemset((uint32_t *)buffer, &data, 2048 / 4);

    for (uint32_t i = 0; i < EEPROM_SIZE / 2048; i++) {
        eeprom.SPI_WriteBuffer(buffer, 2048 * i, 2048);
    }
    println("   ... DONE");
}
#endif