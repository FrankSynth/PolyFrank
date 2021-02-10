#pragma once

#include "arp.hpp"
#include "clock.hpp"
#include "liveDataBase.hpp"
#include "poly.hpp"
#include "voiceHandler.hpp"


class LiveData {
  public:
    LiveData() {
        __liveSettingsLivemode.category = "MODE";
        __liveSettingsLivemode.settings.push_back(&voiceHandler.livemodeVoiceMode);
        __liveSettingsLivemode.settings.push_back(&voiceHandler.livemodeMergeLayer);
        __liveSettingsLivemode.settings.push_back(&livemodeKeysplit);
    }
    ~LiveData() {}

    void controlChange(uint8_t channel, uint8_t cc, uint8_t value);

    // functions
    void keyPressed(uint8_t channel, uint8_t note, uint8_t velocity);
    void keyReleased(uint8_t channel, uint8_t note);

    void clockTick();
    void clockHandling();

    void serviceRoutine();
    VoiceHandler voiceHandler;

    Arpeggiator arpA = Arpeggiator(&voiceHandler);
    Arpeggiator arpB = Arpeggiator(&voiceHandler);

    uint16_t bpm = 0;

    std::vector<Arpeggiator *> arps = {&arpA, &arpB};

    categoryStruct __liveSettingsLivemode;

    Setting livemodeKeysplit = Setting("KEYSPLIT", 0, 0, 1, false, binary, &offOnNameList);

    const std::vector<std::string> offOnNameList = {"OFF", "ON"};
};

extern LiveData liveData;
