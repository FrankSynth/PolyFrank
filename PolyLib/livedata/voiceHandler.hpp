#pragma once

#include "liveDataBase.hpp"

class VoiceHandler {
  public:
    VoiceHandler() {
        // init Voices

        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            voicesA[i].voiceID = i;
            voicesA[i].layerID = 0;
        }

        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            voicesB[i].voiceID = i;
            voicesB[i].layerID = 1;
        }
    }
    void playNote(Key &key);
    void freeNote(Key &key);
    void sendGateOff(voiceStateStruct *v);
    void sustainOff(uint8_t layer);

    void sustainOn(uint8_t layer);

    void findVoices(uint8_t note, voiceStateStruct *voices);

    void getNextVoicesA(uint8_t numberVoices);

    void getNextVoicesB(uint8_t numberVoices);
    void getNextVoicesAB(uint8_t numberVoices);

    void searchNextVoice(voiceStateStruct *voices);
    void searchNextVoiceAB();

    void setSustain(uint8_t value, uint8_t layer);

    std::vector<voiceStateStruct *> foundVoices;
    uint8_t layerID = 0;

    uint32_t playIDCounter = 0;

    uint32_t sustainA = 0;
    uint32_t sustainB = 0;

    Setting livemodeVoiceMode = Setting("MODE", 0, 0, 4, false, binary, &polySplitNameList);
    Setting livemodeMergeLayer = Setting("MERGE", 0, 0, 1, false, binary, &polyMergeNameList);

    const std::vector<std::string> polySplitNameList = {"1|8", "2|4", "4|2", "8|1", "AUTO"};
    const std::vector<std::string> polyMergeNameList = {"A | B", "A + B"};

    voiceStateStruct voicesA[NUMBERVOICES];
    voiceStateStruct voicesB[NUMBERVOICES];
};
