

#ifdef POLYCONTROL

#include "storage/loadStoredData.hpp"
#include "debughelper/debughelper.hpp"
#include "globalsettings/globalSettings.hpp"
#include "hardware/device.hpp"
#include "layer/layer.hpp"
#include "livedata/liveData.hpp"

extern M95M01 eeprom;

extern std::vector<Layer *> allLayers;
extern LiveData liveData;

volatile LAYER_SELECT layerFilter = LAYER_AB;

volatile bool pendingPresetLoad = false;

void readConfig() {
    readConfigBlock();
    if (checkBuffer())
        decodeBuffer();
};
bool readPresetTable() {
    readPresetTableBlock();
    return checkBuffer();
};

void readPreset(uint8_t presetID, LAYER_SELECT layer) {
    readPresetBlock(presetID);

    layerFilter = layer;
    pendingPresetLoad = true;
};

void presetServiceRoutine() {
    if (pendingPresetLoad == true) {
        pendingPresetLoad = false;

        if (checkBuffer()) {
            switch (layerFilter) {
                case LAYER_A: allLayers[0]->clearPatches(); break;
                case LAYER_B: allLayers[1]->clearPatches(); break;
                case LAYER_AB:
                    allLayers[0]->clearPatches();
                    allLayers[1]->clearPatches();
                    break;

                default: break;
            }
            decodeBuffer();
        }
    }
}

///////PRESET/////////

std::vector<presetStruct> presets;
std::vector<presetStruct *> presetsSorted;
presetStruct *freePreset;

uint32_t saveID = 0;

bool comparePtrPresetStruct(presetStruct *a, presetStruct *b) {
    return (*a > *b);
}

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

    if (!readPresetTable()) {
        println("ERROR | PresetTable Corrupted! -> Save Preset for rebuilding Table-");
        freePreset = &presets[0]; // start new PresetTable
        return;
    }

    presetStruct *buffer = (presetStruct *)&(((StorageBlock *)blockBuffer)[1]);

    for (uint32_t i = 0; i < PRESET_NUMBERBLOCKS; i++) {

        if (buffer[i].usageState == PRESET_USED) {
            presets[i] = buffer[i];
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

void writePresetTable() {
    presetStruct *buffer = (presetStruct *)(&blockBuffer[sizeof(StorageBlock)]);

    StorageBlock block;

    uint32_t blockIndex = 0;
    for (presetStruct p : presets) {
        buffer[blockIndex++] = p;
    }

    uint32_t numBytes = blockIndex * sizeof(presetStruct) + sizeof(StorageBlock);

    // Store Start at block Beginning
    block.dataType = STORE_START;
    block.id = 0x00;
    block.data.asUInt = numBytes;
    ((StorageBlock *)blockBuffer)[0] = block;

    uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *)&blockBuffer, block.data.asUInt);

    // Store Checksum
    block.dataType = STORE_CHECKSUM;
    block.id = 0x00;
    block.data.asUInt = checksum;
    ((StorageBlock *)&(blockBuffer[numBytes]))[0] = block;

    if (numBytes + sizeof(StorageBlock) > TABLE_BLOCKSIZE) {
        println("ERROR | FATAL | Table block exceed max table size");
        return;
    }

    writePresetTableBlock();
    updatePresetList();
}

void removePresetfromTable(presetStruct *preset) {
    // write empty entry to table
    preset->usageState = PRESET_FREE;

    writePresetTable();
}

std::vector<presetStruct> *getPresetList() {
    return &presets;
}

void writeConfig(uint32_t blockIndex) {

    if (blockIndex * sizeof(StorageBlock) > CONFIG_BLOCKSIZE) {
        println("ERROR | FATAL | Config block exceed max config size");
        return;
    }
    packBuffer(blockIndex * sizeof(StorageBlock));
    writeConfigBlock();
}

void packBuffer(uint32_t numBytes) {

    StorageBlock *buffer = (StorageBlock *)blockBuffer;
    StorageBlock block = buffer[0];

    // Store Start at block Beginning
    block.dataType = STORE_START;
    block.id = 0x00;
    block.data.asUInt = numBytes;
    buffer[0] = block;

    uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *)&blockBuffer, numBytes);
    // Store Checksum
    block.dataType = STORE_CHECKSUM;
    block.id = 0x00;
    block.data.asUInt = checksum;
    ((StorageBlock *)&(blockBuffer[numBytes]))[0] = block;
}

void writePreset(uint32_t numBytes, presetStruct *preset, std::string name) {

    if (preset == nullptr) {
        updatePresetList();
        return;
    }

    if (numBytes > PRESET_BLOCKSIZE) {
        println("ERROR | FATAL | presetblock exceed max preset size");
        return;
    }

    packBuffer(numBytes);
    writePresetBlock(preset->storageID);

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
    writePresetTable();
}

//////////DECODE/////////

void loadSettingToGlobalSetting(uint8_t categoryID, uint8_t dataID, uint32_t data) {
    for (categoryStruct *c : globalSettings.__categorys) {
        if (c->storeID == categoryID) {
            for (Setting *s : c->settings) {
                if (s->storeID == dataID) {
                    if (s->storeable) {
                        s->setValue(data);
                        return;
                    }
                }
            }
        }
    }
    println("INFO | DecodeBuffer | read->Depreceated Setting -> Skip");
}

void loadPatchDatablockToLayer(uint8_t layerID, uint8_t sourceID, uint8_t targetID, float amount) {
    if ((layerID > 1) | ((layerID != layerFilter) & (layerFilter != LAYER_AB)))
        return;
    // println("P | Source ID: ", sourceID, "  Target ID: ", targetID, "  Amount: ", amount);
    allLayers[layerID]->addPatchInOutById(sourceID, targetID, amount);
}

void loadAnalogDatablockToModule(uint8_t layerID, uint8_t moduleID, uint8_t dataID, int32_t data) {
    if ((layerID > 1) | ((layerID != layerFilter) & (layerFilter != LAYER_AB)))
        return;
    for (BaseModule *m : allLayers[layerID]->getModules()) {
        if (m->storeID == moduleID) {
            for (Analog *a : m->getAnalog()) {
                if (a->storeID == dataID) {

                    // println("A | Module ID: ", moduleID, "  DataID: ", dataID, "  Data: ", data);
                    a->presetLock = true;
                    a->setValue(data);
                    return;
                }
            }
        }
    }
    println("INFO | DecodeBuffer | read->Depreceated Analog -> Skip");
}
void loadDigitalDatablockToModule(uint8_t layerID, uint8_t moduleID, uint8_t dataID, int32_t data) {
    if ((layerID > 1) | ((layerID != layerFilter) & (layerFilter != LAYER_AB)))
        return;
    for (BaseModule *m : allLayers[layerID]->getModules()) {
        if (m->storeID == moduleID) {
            for (Digital *d : m->getDigital()) {

                if (d->storeID == dataID) {
                    // println("D | Module ID: ", moduleID, "  DataID: ", dataID, "  Data: ", data);
                    d->setValueWithoutMapping(data);
                    return;
                }
            }
        }
    }
    println("INFO | DecodeBuffer | read->Depreceated Digital -> Skip");
}
void loadSettingToLive(uint8_t dataID, int32_t data) {
    for (Setting *s : liveData.__liveSettingsLivemode) {
        if (s->storeID == dataID) {
            if (s->storeable) {
                s->setValue(data);
                return;
            }
        }
    }

    println("INFO | DecodeBuffer | read->Depreceated Live Setting -> Skip");
}

void loadSettingToArp(uint8_t layerID, uint8_t dataID, int32_t data) {
    if ((layerID > 1) | ((layerID != layerFilter) & (layerFilter != LAYER_AB)))
        return;
    for (Setting *s : liveData.arps[layerID].__liveSettingsArp) {
        if (s->storeID == dataID) {
            if (s->storeable) {
                s->setValue(data);
                return;
            }
        }
    }

    println("INFO | DecodeBuffer | read->Depreceated Arp Setting -> Skip");
}

bool checkBuffer() {
    StorageBlock block = ((StorageBlock *)blockBuffer)[0];

    uint32_t numBytes = block.data.asUInt;
    uint32_t checksum = 0;

    if (block.dataType == STORE_START) {
        if (numBytes < BLOCKBUFFERSIZE) { // size valid?
            checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *)&blockBuffer, numBytes);
        }
        else {
            println("ERROR | DecodeBuffer | read->Size Failed!");

            return false; // invalid Size
        }
    }
    else {
        println("ERROR | DecodeBuffer | read->StartMarker Failed!");
        return false; // wrong start marker
    }
    block = ((StorageBlock *)&blockBuffer[numBytes])[0];

    if (block.dataType == STORE_CHECKSUM) {
        if (checksum != block.data.asUInt) {
            println("ERROR | DecodeBuffer | read->Checksum Failed!");
            return false;
        }
    }
    else {
        println("ERROR | DecodeBuffer | read->ChecksumMarker Failed!");
        return false; // wrong start marker
    }

    return true;
}

void decodeBuffer() {

    uint8_t *buffer = blockBuffer;
    StorageBlock sBlock;
    patchStorageBlock pBlock;

    sBlock = *(StorageBlock *)blockBuffer;

    uint32_t numBytes = sBlock.data.asUInt;

    bool decodeActive = true;

    uint8_t layerID = 0xFF;
    uint8_t parentID = 0xFF;

    uint32_t byteIndex = sizeof(StorageBlock);

    while (decodeActive & (byteIndex < numBytes)) {

        if ((StoreTYPE)buffer[byteIndex] ==
            STORE_PATCH) { // We expect an layer containing block befor the patchblock decoding start

            pBlock = *(patchStorageBlock *)&(buffer[byteIndex]);

            loadPatchDatablockToLayer(layerID, pBlock.sourceID, pBlock.targetID, pBlock.amount);
            byteIndex += sizeof(patchStorageBlock);
        }
        else {
            sBlock = *(StorageBlock *)&(buffer[byteIndex]);
            byteIndex += sizeof(StorageBlock);

            switch (sBlock.dataType) {
                case STORE_CATEGORY: parentID = sBlock.id; break;
                case STORE_CONFIGSETTING: loadSettingToGlobalSetting(parentID, sBlock.id, sBlock.data.asInt); break;
                case STORE_MODULE:
                    parentID = sBlock.id & 0x7F;
                    layerID = sBlock.id >> 7;
                    break;
                case STORE_DATAANALOG:
                    loadAnalogDatablockToModule(layerID, parentID, sBlock.id, sBlock.data.asInt);
                    break;
                case STORE_DATADIGITAL:
                    loadDigitalDatablockToModule(layerID, parentID, sBlock.id, sBlock.data.asInt);
                    break;
                case STORE_LIVESETTING: loadSettingToLive(sBlock.id, sBlock.data.asInt); break;
                case STORE_ARPSETTING:
                    layerID = sBlock.id >> 7;
                    loadSettingToArp(layerID, sBlock.id & 0x7F, sBlock.data.asInt);
                    break;

                case STORE_CHECKSUM: println("Blockend reached -> exit decoding"); return;

                default: println("INFO | DecodeBuffer | read->Depreceated Block -> Skip"); break;
            }
        }
    }
}

#endif