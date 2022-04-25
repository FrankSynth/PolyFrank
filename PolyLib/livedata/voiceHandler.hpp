#pragma once

#include "liveDataBase.hpp"

class VoiceHandler {
  public:
    VoiceHandler() {
        // init Voices
        for (size_t i = 0; i < 2; i++) {
            for (uint8_t x = 0; x < NUMBERVOICES; x++) {
                voices[i][x].voiceID = x;
                voices[i][x].layerID = i;
            }
        }
    }
    void playNote(Key &key);
    void freeNote(Key &key);
    void sendGateOff(voiceStateStruct *v);
    void sustainOff(uint8_t layer);
    void reset(uint8_t layer);

    void sustainOn(uint8_t layer);

    void findVoices(uint8_t note, voiceStateStruct *voices);

    void getNextVoices(uint8_t numberVoices, uint8_t layer);

    void getNextVoicesAB(uint8_t numberVoices);

    void searchNextVoice(voiceStateStruct *voices);
    void searchNextVoiceAB();

    void setSustain(uint8_t value, uint8_t layer);

    std::vector<voiceStateStruct *> foundVoices;

    uint32_t playIDCounter = 0;

    uint32_t sustain[2] = {0, 0};

    Setting livemodeVoiceMode = Setting("MODE", 0, 0, 3, false, binary, &polySplitNameList);
    Setting livemodeMergeLayer = Setting("MERGE", 0, 0, 1, false, binary, &polyMergeNameList);

    const std::vector<const char *> polySplitNameList = {"1|8", "2|4", "4|2", "8|1"};
    const std::vector<const char *> polyMergeNameList = {"A | B", "A + B"};

    voiceStateStruct voices[2][NUMBERVOICES];
};
