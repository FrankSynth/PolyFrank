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
        __liveSettingsLivemode.settings.push_back(&livemodeExternalClockMultiply);
    }
    ~LiveData() {}

    void controlChange(uint8_t channel, uint8_t cc, int16_t value);
    void distributeCC(uint8_t cc, int16_t value, uint8_t layer);

    // functions
    void keyPressed(uint8_t channel, uint8_t note, uint8_t velocity);
    void keyReleased(uint8_t channel, uint8_t note);

    void midiClockTick();
    void receivedStart();
    void receivedContinue();
    void receivedStop();
    void receivedReset();
    void receivedMidiSongPosition(unsigned int spp);
    void internalClockTick();
    void externalClockTick();

    void clockHandling();

    void serviceRoutine();

    void reset();

    VoiceHandler voiceHandler;

    Arpeggiator arps[2] = {Arpeggiator(&voiceHandler), Arpeggiator(&voiceHandler)};

    uint16_t bpm = 0;

    categoryStruct __liveSettingsLivemode;

    Setting livemodeKeysplit = Setting("KEYSPLIT", 0, 0, 1, false, binary, &offOnNameList);
    Setting livemodeClockSource = Setting("Clock Source", 1, 0, 2, false, binary, &clockSourceList);
    Setting livemodeExternalClockMultiply = Setting("EXT. Clock", 3, 0, 5, false, binary, &externalClockMultList);

    const std::vector<std::string> offOnNameList = {"OFF", "ON"};
    const std::vector<std::string> clockSourceList = {"EXTERN", "MIDI", "INTERN"};
    const std::vector<std::string> externalClockMultList = {"1/32", "1/16", "1/8", "1/4", "1/2", "1/1"};
};

extern LiveData liveData;
