#pragma once

#include "datacore/datacore.hpp"
#include "preset/preset.hpp"
#include <vector>

struct categoryStruct {
    std::string category;
    std::vector<Setting *> settings;
};

class GlobalSettings {
  public:
    GlobalSettings() {

        // push all settings here

        __globSettingsSystem.category = "SYSTEM";
        __globSettingsSystem.settings.push_back(&amountLayers);

        __globSettingsMIDI.category = "MIDI";
        __globSettingsMIDI.settings.push_back(&midiSource);
        __globSettingsMIDI.settings.push_back(&midiSend);
        __globSettingsMIDI.settings.push_back(&midiLayer1Channel);
        __globSettingsMIDI.settings.push_back(&midiLayer2Channel);

        __globSettingsDisplay.category = "DISPLAY";
        __globSettingsDisplay.settings.push_back(&dispColor);
        __globSettingsDisplay.settings.push_back(&dispBrightness);

        // init setting IDs
        // initID();
    }

    void saveGlobalSettings() {
        uint32_t index = 0;
        int32_t *buffer = (int32_t *)blockBuffer;

        for (Setting *i : __globSettingsSystem.settings) {
            buffer[index] = i->value;
            index++;
        }
        for (Setting *i : __globSettingsMIDI.settings) {
            buffer[index] = i->value;
            index++;
        }
        for (Setting *i : __globSettingsDisplay.settings) {
            buffer[index] = i->value;
            index++;
        }

        writeConfigBlock();

        if ((uint32_t)((uint8_t *)&buffer[index] - (uint8_t *)blockBuffer) > (CONFIG_BLOCKSIZE)) {
            PolyError_Handler("ERROR | FATAL | GlobalSettings -> saveGlobalSettings -> BufferOverflow!");
        }
    }

    void loadGlobalSettings() {
        uint32_t index = 0;
        int32_t *buffer = (int32_t *)blockBuffer;

        readConfig();

        for (Setting *i : __globSettingsSystem.settings) {
            i->setValue(buffer[index]);
            index++;
        }
        for (Setting *i : __globSettingsMIDI.settings) {
            i->setValue(buffer[index]);
            index++;
        }
        for (Setting *i : __globSettingsDisplay.settings) {
            i->setValue(buffer[index]);
            index++;
        }
    }

    // init
    void initID() {
        // ID MIDIID;
        // ID DispID;
        // ID modID;

        // for (Setting *setting : __globSettingsMIDI) {
        //     setting->id = modID.getNewId();
        // }
        // for (Setting *setting : __globSettingsDisplay) {
        //     setting->id = modID.getNewId();
        // }
        // for (Setting *setting : __globSettingsSystem) {
        //     setting->id = modID.getNewId();
        // }
    }

    // return all settings
    // std::vector<Setting *> getSettings() { return __globSettings; }

    // beim Hinzufuegen von neuen Katergorien mussa auch im Save und Load die Kategorie eingepflegt werden
    categoryStruct __globSettingsSystem;
    categoryStruct __globSettingsMIDI;
    categoryStruct __globSettingsDisplay;

    // all Settings, don't forget to push to __globSettings vector of this class
    Setting amountLayers = Setting("LAYER", 0, 0, 1, false, binary, &amountLayerNameList);

    Setting midiSource = Setting("MIDI", 0, 0, 1, false, binary, &midiTypeNameList);
    Setting midiSend = Setting("SEND", 0, 0, 1, false, binary, &offOnNameList);
    Setting midiLayer1Channel = Setting("Layer A Ch.", 0, 0, 10, false, binary);
    Setting midiLayer2Channel = Setting("Layer B Ch.", 0, 0, 10, false, binary);

    Setting dispColor = Setting("COLOR", 0, 0, 1, false, binary, &colorThemeNameList);
    Setting dispBrightness = Setting("BRIGHTNESS", 10, 2, 10, false, binary);

    Error error;

  private:
    const std::vector<std::string> amountLayerNameList = {"A", "A+B"};
    const std::vector<std::string> midiTypeNameList = {"USB", "DIN"};
    const std::vector<std::string> offOnNameList = {"OFF", "ON"};
    const std::vector<std::string> colorThemeNameList = {"DEFAULT", "NICE"};
};

extern GlobalSettings globalSettings;
