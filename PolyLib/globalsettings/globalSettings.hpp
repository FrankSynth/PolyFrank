#pragma once

#include "datacore/datacore.hpp"
#include "layer/layer.hpp"
#include "preset/preset.hpp"
#include <vector>

extern std::vector<Layer *> allLayers;

struct categoryStruct {
    std::string category;
    std::vector<Setting *> settings;
};

struct renderStatusStruct {
    Status temperature = Status("Temp", "C", 1);
    Status usage = Status("Usage", "%", 1);
    Status timeCV = Status("CV", "uS", 1);
    Status timeAudio = Status("AUDIO", "uS", 1);
};
struct controlStatusStruct {
    Status temperature = Status("Temp", "C", 1);
    Status usage = Status("Usage", "%", 1);
};

class GlobalSettings {
  public:
    GlobalSettings() {

        // push all settings here

        __globSettingsSystem.category = "SYSTEM";
        __globSettingsSystem.settings.push_back(&extClockOutLength);
        __globSettingsSystem.settings.push_back(&presetValueHandling);

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

        statusReportString.reserve(1024); // reserve enough space for status report
        // init setting IDs
        // initID();
    }

    void saveGlobalSettings() {
        uint32_t index = 0;
        int32_t *buffer = (int32_t *)blockBuffer;

        for (Setting *i : __globSettingsSystem.settings) {
            buffer[index++] = i->value;
        }
        for (Setting *i : __globSettingsMIDI.settings) {
            buffer[index++] = i->value;
        }
        for (Setting *i : __globSettingsDisplay.settings) {
            buffer[index++] = i->value;
        }

        // Store Tuning
        for (Analog *a : allLayers[0]->tune.knobs) {
            buffer[index++] = a->valueMapped;
        }
        for (Analog *a : allLayers[1]->tune.knobs) {
            buffer[index++] = a->valueMapped;
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
            s->setValue(buffer[index++]);
        }
        for (Setting *s : __globSettingsMIDI.settings) {
            s->setValue(buffer[index++]);
        }
        for (Setting *s : __globSettingsDisplay.settings) {
            s->setValue(buffer[index++]);
        }

        // Store Tuning
        for (Analog *a : allLayers[0]->tune.knobs) {
            a->setValueWithoutMapping(buffer[index++]);
        }
        for (Analog *a : allLayers[1]->tune.knobs) {
            a->setValueWithoutMapping(buffer[index++]);
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

    std::string *statusReport() {

        statusReportString.clear();

        // control
        statusReportString += "Control Status: \n\r";
        controlStatus.temperature.appendValueAsString(statusReportString);
        controlStatus.usage.appendValueAsString(statusReportString);

        for (uint32_t layer = 0; layer < 2; layer++) {
            statusReportString += "\f";

            for (uint32_t chip = 0; chip < 2; chip++) {

                statusReportString += "Render Status " + std::to_string(layer) + std::to_string(chip) + "\n\r";

                renderStatus[layer][chip].temperature.appendValueAsString(statusReportString);
                renderStatus[layer][chip].usage.appendValueAsString(statusReportString);
                renderStatus[layer][chip].timeCV.appendValueAsString(statusReportString);
                renderStatus[layer][chip].timeAudio.appendValueAsString(statusReportString);
                statusReportString += "\n\r";
            }
        }

        return &statusReportString;
    }

    // beim Hinzufuegen von neuen Katergorien mussa auch im Save und Load die Kategorie eingepflegt werden
    categoryStruct __globSettingsSystem;
    categoryStruct __globSettingsMIDI;
    categoryStruct __globSettingsDisplay;

    std::string statusReportString;

    // all Settings, don't forget to push to __globSettings vector of this class
    Setting multiLayer = Setting("LAYER", 0, 0, 1, false, binary, &amountLayerNameList);

    Setting extClockOutLength = Setting("EXT CLK. ms.", 2, 1, 50, false, binary);
    Setting presetValueHandling = Setting("PRESET VALUE", 0, 0, 1, false, binary, &presetValueHandlingNameList);

    Setting midiSource = Setting("MIDI", 0, 0, 1, false, binary, &midiTypeNameList);
    Setting midiSend = Setting("SEND", 0, 0, 1, false, binary, &offOnNameList);
    Setting midiLayerAChannel = Setting("Layer A Ch.", 0, 0, 10, false, binary);
    Setting midiLayerBChannel = Setting("Layer B Ch.", 0, 0, 10, false, binary);

    Setting dispColor = Setting("COLOR", 0, 0, 1, false, binary, &colorThemeNameList);
    Setting dispBrightness = Setting("BRIGHTNESS", 10, 2, 10, false, binary);
    Setting dispLED = Setting("LED", 5, 1, 20, false, binary);

    Setting dispTemperature = Setting("TEMPERATURE", 0, 0, 1, false, binary, &amountLayerNameList);

    Error error;

    controlStatusStruct controlStatus;
    renderStatusStruct renderStatus[2][2];

    uint8_t shift = 0;

  private:
    const std::vector<const char *> presetValueHandlingNameList = {"OVERWRITE", "GRAB"};

    const std::vector<const char *> amountLayerNameList = {"OFF", "ON"};
    const std::vector<const char *> midiTypeNameList = {"USB", "DIN"};
    const std::vector<const char *> offOnNameList = {"OFF", "ON"};
    const std::vector<const char *> colorThemeNameList = {"DEFAULT", "INVERT"};

    const std::vector<const char *> polyMergeNameList = {"A | B", "A + B"};
    const std::vector<const char *> polySplitNameList = {"1|8", "2|4", "4|2", "8|1", "AUTO"};
};

extern GlobalSettings globalSettings;
