#pragma once

#include "arp.hpp"
#include "clock.hpp"
#include "liveDataBase.hpp"
#include "poly.hpp"
#include "voiceHandler.hpp"

extern Clock clock;
extern ClockSource internalClock;

extern const std::vector<const char *> offOnNameList;
extern const std::vector<const char *> clockSourceList;
extern const std::vector<const char *> externalClockMultList;
extern const std::vector<const char *> extStepNameList;

void switchLiveMode(int32_t *setting);
void switchClockSourceCallback(int32_t *setting);

class LiveData {
  public:
    LiveData() {

        livemodeKeysplit.storeID = 0x00;
        livemodeClockSource.storeID = 0x01;
        internalClock.clockBPM.storeID = 0x02;
        livemodeExternalClockMultiplyOut.storeID = 0x03;

        voiceHandler.livemodeVoiceModeA.storeID = 0x10;
        voiceHandler.livemodeVoiceModeB.storeID = 0x11;

        voiceHandler.livemodeMergeLayer.storeable = false;

        __liveSettingsLivemode.push_back(&voiceHandler.livemodeVoiceModeA);
        __liveSettingsLivemode.push_back(&voiceHandler.livemodeVoiceModeB);
        __liveSettingsLivemode.push_back(&voiceHandler.livemodeMergeLayer);
        __liveSettingsLivemode.push_back(&livemodeKeysplit);
        __liveSettingsLivemode.push_back(&livemodeClockSource);
        __liveSettingsLivemode.push_back(&internalClock.clockBPM);
        __liveSettingsLivemode.push_back(&livemodeExternalClockMultiplyOut);
    }
    ~LiveData() {}

    void init() {}
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

    void calcBPMfromExternalSource();

    void clockHandling();
    void externalSyncHandling();

    void serviceRoutine();

    void calcLFOSnapFreq(LFO &lfo);
    void calcAllLFOSnapFreq();

    // void getLiveConfiguration(int32_t *buffer);
    // void setLiveConfiguration(int32_t *buffer, LayerSelect layer);

    void resetLiveConfig();
    void reset();

    uint32_t writeLive(uint32_t blockStartIndex);

    VoiceHandler voiceHandler;

    Arpeggiator arps[2] = {Arpeggiator(&voiceHandler, 0), Arpeggiator(&voiceHandler, 1)};

    std::vector<Setting *> __liveSettingsLivemode;

    Setting livemodeKeysplit = Setting("KEYSPLIT", 0, 0, 1, false, binary, &offOnNameList, false);
    Setting livemodeClockSource = Setting("CLK SOURCE", 1, 0, 2, false, binary, &clockSourceList, false);

    Setting livemodeExternalClockMultiplyOut = Setting("EXT. CLK OUT", 3, 0, 7, false, binary, &extStepNameList, false);

    elapsedMillis extClockLengthTimer;
    elapsedSeconds extClockTimeout = 301;
    elapsedMicros meassuredBPMTime = 0;
    int32_t extSyncCounter = 0;

    bool extSync = false;
};

extern LiveData liveData;
