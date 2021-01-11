#pragma once

#include "datacore/datacore.hpp"
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
        __globSettingsDisplay.settings.push_back(&dispBrigthness);

        // init setting IDs
        // initID();
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
    Setting dispBrigthness = Setting("BRIGTHNESS", 10, 2, 10, false, binary);

  private:
    const std::vector<std::string> amountLayerNameList = {"A", "A+B"};
    const std::vector<std::string> midiTypeNameList = {"USB", "DIN"};
    const std::vector<std::string> offOnNameList = {"OFF", "ON"};
    const std::vector<std::string> colorThemeNameList = {"DEFAULT", "NICE"};
};

extern GlobalSettings globalSettings;
