#pragma once

#include "arp.hpp"
#include "clock.hpp"
#include "liveDataBase.hpp"
#include "poly.hpp"
#include "voiceHandler.hpp"

extern Clock clock;
extern ClockSource internalClock;

class LiveData {
  public:
    LiveData() {
        __liveSettingsLivemode.category = "MODE";
        __liveSettingsLivemode.settings.push_back(&voiceHandler.livemodeVoiceMode);
        __liveSettingsLivemode.settings.push_back(&voiceHandler.livemodeMergeLayer);
        __liveSettingsLivemode.settings.push_back(&livemodeKeysplit);
        __liveSettingsLivemode.settings.push_back(&livemodeClockSource);
        __liveSettingsLivemode.settings.push_back(&internalClock.clockBPM);
    }
    ~LiveData() {}

    void controlChange(uint8_t channel, uint8_t cc, int16_t value);
    void distributeCC(uint8_t cc, int16_t value, uint8_t layer);

    // functions
    void keyPressed(uint8_t channel, uint8_t note, uint8_t velocity);
    void keyReleased(uint8_t channel, uint8_t note);

    void midiClockTick();

    void internalClockTick();
    void externalClockTick();
    void clockHandling();

    void serviceRoutine();
    VoiceHandler voiceHandler;

    Arpeggiator arpA = Arpeggiator(&voiceHandler);
    Arpeggiator arpB = Arpeggiator(&voiceHandler);

    uint16_t bpm = 0;

    std::vector<Arpeggiator *> arps = {&arpA, &arpB};

    categoryStruct __liveSettingsLivemode;

    Setting livemodeKeysplit = Setting("KEYSPLIT", 0, 0, 1, false, binary, &offOnNameList);
    Setting livemodeClockSource = Setting("Clock Source", 0, 0, 2, false, binary, &clockSourceList);

    const std::vector<std::string> offOnNameList = {"OFF", "ON"};
    const std::vector<std::string> clockSourceList = {"EXTERN", "MIDI", "INTERN"};
};

extern LiveData liveData;
