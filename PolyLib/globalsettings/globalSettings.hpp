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
        __globSettingsSystem.settings.push_back(&extClockOutLength);

        __globSettingsMIDI.category = "MIDI";
        __globSettingsMIDI.settings.push_back(&midiSource);
        __globSettingsMIDI.settings.push_back(&midiSend);
        __globSettingsMIDI.settings.push_back(&midiLayerAChannel);
        __globSettingsMIDI.settings.push_back(&midiLayerBChannel);

        __globSettingsDisplay.category = "DISPLAY";
        __globSettingsDisplay.settings.push_back(&dispColor);
        __globSettingsDisplay.settings.push_back(&dispBrightness);
        __globSettingsDisplay.settings.push_back(&dispLED);
        __globSettingsDisplay.settings.push_back(&dispTemperature);

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
        println("INFO || Config Datasize: ", (uint32_t)((uint8_t *)&buffer[index] - (uint8_t *)buffer));
    }

    void loadGlobalSettings() {
        uint32_t index = 0;
        int32_t *buffer = (int32_t *)blockBuffer;

        readConfig();

        for (Setting *s : __globSettingsSystem.settings) {
            s->setValue(buffer[index]);
            index++;
        }
        for (Setting *s : __globSettingsMIDI.settings) {
            s->setValue(buffer[index]);
            index++;
        }
        for (Setting *s : __globSettingsDisplay.settings) {
            s->setValue(buffer[index]);
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

    void setShift(uint8_t shift) { this->shift = shift; }

    // beim Hinzufuegen von neuen Katergorien mussa auch im Save und Load die Kategorie eingepflegt werden
    categoryStruct __globSettingsSystem;
    categoryStruct __globSettingsMIDI;
    categoryStruct __globSettingsDisplay;

    // all Settings, don't forget to push to __globSettings vector of this class
    Setting multiLayer = Setting("LAYER", 0, 0, 1, false, binary, &amountLayerNameList);

    Setting extClockOutLength = Setting("EXT CLK. ms.", 2, 1, 50, false, binary);

    Setting midiSource = Setting("MIDI", 0, 0, 1, false, binary, &midiTypeNameList);
    Setting midiSend = Setting("SEND", 0, 0, 1, false, binary, &offOnNameList);
    Setting midiLayerAChannel = Setting("Layer A Ch.", 0, 0, 10, false, binary);
    Setting midiLayerBChannel = Setting("Layer B Ch.", 0, 0, 10, false, binary);

    Setting dispColor = Setting("COLOR", 0, 0, 1, false, binary, &colorThemeNameList);
    Setting dispBrightness = Setting("BRIGHTNESS", 10, 2, 10, false, binary);
    Setting dispLED = Setting("LED", 10, 1, 10, false, binary);

    Setting dispTemperature = Setting("TEMPERATURE", 0, 0, 1, false, binary, &amountLayerNameList);

    Error error;

    uint32_t temperature = 0;

    uint8_t shift = 0;

  private:
    const std::vector<const char *> amountLayerNameList = {"OFF", "ON"};
    const std::vector<const char *> midiTypeNameList = {"USB", "DIN"};
    const std::vector<const char *> offOnNameList = {"OFF", "ON"};
    const std::vector<const char *> colorThemeNameList = {"DEFAULT", "INVERT"};

    const std::vector<const char *> polyMergeNameList = {"A | B", "A + B"};
    const std::vector<const char *> polySplitNameList = {"1|8", "2|4", "4|2", "8|1", "AUTO"};
};

extern GlobalSettings globalSettings;
