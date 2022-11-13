#pragma once

#include "datacore/datacore.hpp"
#include "layer/layer.hpp"
#include "storage/loadStoredData.hpp"

#include <vector>

extern std::vector<Layer *> allLayers;
extern CRC_HandleTypeDef hcrc;

struct categoryStruct {
    std::string category;
    std::vector<Setting *> settings;
    uint8_t storeID;
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

        extClockOutLength.storeID = 0;
        presetValueHandling.storeID = 1;
        functionButtons.storeID = 2;

        midiSource.storeID = 10;
        midiSend.storeID = 11;
        midiLayerAChannel.storeID = 12;
        midiLayerBChannel.storeID = 13;
        midiPitchBendRange.storeID = 14;

        dispColor.storeID = 20;
        dispBrightness.storeID = 21;
        dispLED.storeID = 22;

        /////////
        __categorys.push_back(&__globSettingsSystem);
        __categorys.push_back(&__globSettingsMIDI);
        __categorys.push_back(&__globSettingsDisplay);

        __globSettingsSystem.category = "SYSTEM";
        __globSettingsSystem.storeID = 0;
        __globSettingsSystem.settings.push_back(&extClockOutLength);
        __globSettingsSystem.settings.push_back(&presetValueHandling);
        __globSettingsSystem.settings.push_back(&functionButtons);

        __globSettingsMIDI.category = "MIDI";
        __globSettingsMIDI.storeID = 1;
        __globSettingsMIDI.settings.push_back(&midiSource);
        __globSettingsMIDI.settings.push_back(&midiSend);
        __globSettingsMIDI.settings.push_back(&midiLayerAChannel);
        __globSettingsMIDI.settings.push_back(&midiLayerBChannel);
        __globSettingsMIDI.settings.push_back(&midiPitchBendRange);

        __globSettingsDisplay.category = "DISPLAY";
        __globSettingsDisplay.storeID = 2;
        __globSettingsDisplay.settings.push_back(&dispColor);
        __globSettingsDisplay.settings.push_back(&dispBrightness);
        __globSettingsDisplay.settings.push_back(&dispLED);

        statusReportString.reserve(1024); // reserve enough space for status report
    }

    void init() {
        dispBrightness.setValue(10);
        dispLED.setValue(10);
        dispColor.setValue(0);
    };
    void writeGlobalSettings() {
        StorageBlock *buffer = (StorageBlock *)blockBuffer;
        StorageBlock block;

        uint32_t blockIndex = 1;

        for (categoryStruct *c : __categorys) {

            block.dataType = STORE_CATEGORY;
            block.id = c->storeID;
            block.data.asUInt = 0x00;
            storeToBlock(buffer, block, blockIndex);

            block.dataType = STORE_CONFIGSETTING;
            for (Setting *d : c->settings) {
                if (d->storeable) {
                    block.id = d->storeID;
                    block.data.asInt = d->value;
                    storeToBlock(buffer, block, blockIndex);
                }
            }
        }

        // Store Tuning
        for (uint8_t layer = 0; layer < 2; layer++) {
            block.dataType = STORE_MODULE;
            block.id = allLayers[layer]->tune.storeID | layer << 7;
            block.data.asUInt = 0x00;
            storeToBlock(buffer, block, blockIndex);

            block.dataType = STORE_DATAANALOG; // Do not check storable!
            for (Analog *a : allLayers[layer]->tune.knobs) {
                if (a->storeable) {
                    block.id = a->storeID;
                    block.data.asInt = a->value;
                    storeToBlock(buffer, block, blockIndex);
                }
            }
        }

        writeConfig(blockIndex);
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
    std::vector<categoryStruct *> __categorys;
    categoryStruct __globSettingsSystem;
    categoryStruct __globSettingsMIDI;
    categoryStruct __globSettingsDisplay;

    std::string statusReportString;

    // all Settings, don't forget to push to __globSettings vector of this class
    Setting multiLayer = Setting("LAYER", 0, 0, 1, false, binary, &amountLayerNameList);

    Setting extClockOutLength = Setting("EXT CLK. ms.", 2, 1, 50, false, binary);
    Setting presetValueHandling = Setting("PRESET VALUE", 0, 0, 1, false, binary, &presetValueHandlingNameList);
    Setting functionButtons = Setting("F BUTTON", 0, 0, 1, false, binary, &functionButtonsHandlingNameList);

    Setting midiSource = Setting("MIDI", 0, 0, 1, false, binary, &midiTypeNameList);
    Setting midiSend = Setting("SEND", 0, 0, 1, false, binary, &offOnNameList);
    Setting midiLayerAChannel = Setting("Layer A Ch.", 0, 0, 10, false, binary);
    Setting midiLayerBChannel = Setting("Layer B Ch.", 0, 0, 10, false, binary);
    Setting midiPitchBendRange = Setting("PB Range", 2, 1, 12, false, binary, &pbRangeNameList);

    Setting dispColor = Setting("COLOR", 0, 0, 1, false, binary, &colorThemeNameList);
    Setting dispBrightness = Setting("BRIGHTNESS", 10, 2, 10, false, binary);
    Setting dispLED = Setting("LED", 5, 1, 20, false, binary);

    Error error;

    controlStatusStruct controlStatus;
    renderStatusStruct renderStatus[2][2];

    uint8_t shift = 0;

  private:
    const std::vector<const char *> functionButtonsHandlingNameList = {"SAVE SLOTS", "FUNCTION2"};

    const std::vector<const char *> presetValueHandlingNameList = {"OVERWRITE", "GRAB"};

    const std::vector<const char *> amountLayerNameList = {"OFF", "ON"};
    const std::vector<const char *> midiTypeNameList = {"USB", "DIN"};
    const std::vector<const char *> offOnNameList = {"OFF", "ON"};
    const std::vector<const char *> colorThemeNameList = {"DEFAULT", "INVERT"};

    const std::vector<const char *> polyMergeNameList = {"A | B", "A + B"};
    const std::vector<const char *> polySplitNameList = {"1|8", "2|4", "4|2", "8|1", "AUTO"};
    const std::vector<const char *> pbRangeNameList = {"1 Semitone",  "2 Semitones",  "3 Semitones",  "4 Semitones",
                                                       "5 Semitones", "6 Semitones",  "7 Semitones",  "8 Semitones",
                                                       "9 Semitones", "10 Semitones", "11 Semitones", "12 Semitones"};
};

extern GlobalSettings globalSettings;
