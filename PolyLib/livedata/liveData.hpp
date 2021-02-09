#pragma once

#include "globalsettings/globalSettings.hpp"
#include "poly.hpp"
#include <algorithm>
#include <list>
#include <string>
#include <vector>

#define NUMBERLAYER 2
#define NUMBERVOICES 8
#define NUMBERCLOCKSTATES 23

#define MAXCLOCKTICKS 2304

typedef enum { LAYERA, LAYERB, LAYERAB } playModeEnum;
typedef enum { FREE, PLAY, SELECT, SUSTAIN } voiceStatusEnum;

struct Key {
    uint8_t note;
    uint8_t velocity;
    uint8_t layerID;
    uint8_t released = 0;
    uint32_t born = 0; // in micros?
    uint32_t lifespan = 0;
};

// struct pro voice für den aktuellen status: gespielter ton, c
typedef struct {
    voiceStatusEnum status = FREE;
    uint8_t note = 0;
    uint8_t velocity = 0;
    uint8_t voiceID = 0;
    uint8_t layerID = 0;
    uint32_t playID = 0;
} voiceStateStruct;

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

// this class handles all data occuring during live play

class Clock {
  public:
    void tick() { // TODO timing logic
        static elapsedMillis averagingStartTimer;
        static elapsedMicros tickTimer;
        static uint32_t accumMicrosPerTick = 0;
        static uint32_t averageCounter = 0;
        static uint8_t doNotCalcBpm = 0;

        counter++;
        counter %= MAXCLOCKTICKS;
        ticked = 1;

        averageCounter++;
        accumMicrosPerTick += tickTimer;
        tickTimer = 0;

        if (averagingStartTimer > 1000) {

            // avoid updating bpm when last update is too long ago
            if (averagingStartTimer > 2000 || doNotCalcBpm) {
                averagingStartTimer = 0;
                accumMicrosPerTick = 0;
                averageCounter = 0;
                doNotCalcBpm = 0;
                return;
            }
            bpm = 60000000. / ((((float)accumMicrosPerTick * 24) / (float)(averageCounter)));
            accumMicrosPerTick = 0;
            averageCounter = 0;
            averagingStartTimer = 0;
        }
    }

    uint8_t ticked = 0;

    uint32_t counter = 0;

    float bpm = 120;
};

class Arpeggiator {
  public:
    Arpeggiator(VoiceHandler *voiceHandler) {

        this->voiceHandler = voiceHandler;

        __liveSettingsArp.category = "ARP";
        __liveSettingsArp.settings.push_back(&arpEnable);
        __liveSettingsArp.settings.push_back(&arpLatch);
        __liveSettingsArp.settings.push_back(&arpMode);
        __liveSettingsArp.settings.push_back(&arpSteps);
        __liveSettingsArp.settings.push_back(&arpOctave);
        __liveSettingsArp.settings.push_back(&arpRep);
        __liveSettingsArp.settings.push_back(&arpPressed);
    }

    // functions
    void keyPressed(Key &key);
    void keyReleased(Key &key);

    void pressKey(Key &key);
    void lifetime(Key &key);

    void releaseAndRatchet();

    void orderKeys();
    void nextStep();

    std::list<Key> pressedKeys;
    std::list<Key> inputKeys;

    std::vector<Key> orderedKeys;

    uint16_t keyListPosition;

    uint8_t allKeysReleased = 0;
    uint8_t sustain = 0;

    VoiceHandler *voiceHandler;

    categoryStruct __liveSettingsArp;
    Setting arpMode = Setting("MODE", 0, 0, 11, false, binary, &arpModeNameList);
    Setting arpOctave = Setting("OCTAVE", 0, -3, 3, false, binary, &arpOctaveNameList);
    Setting arpSteps = Setting("STEP", 9, 0, 22, false, binary, &arpStepNameList);
    Setting arpRep = Setting("REP", 0, 0, 3, false, binary, &arpRepNameList);
    Setting arpLatch = Setting("LATCH", 0, 0, 1, false, binary, &offOnNameList);
    Setting arpEnable = Setting("ARPEGGIATOR", 0, 0, 1, false, binary, &offOnNameList);
    Setting arpPressed = Setting("KEYS ", 1, 1, 8, false, binary);

    const std::vector<std::string> offOnNameList = {"OFF", "ON"};

    const std::vector<std::string> arpRepNameList = {"OFF", "1", "2", "3"};
    const std::vector<std::string> arpOctaveNameList = {"-3", "-2", "-1", "0", "1", "2", "3"};

    const std::vector<std::string> arpStepNameList = {
        "1/64T", "1/32T", "1/32", "1/16T", "1/16", "1/8T", "1/16.", "1/8", "1/4T", "1/8.", "1/4", "1/2T",
        "1/4.",  "1/2",   "1/1T", "1/2.",  "1/1",  "2/1T", "1/1.",  "2/1", "4/1T", "2/1.", "4/1"};

    const std::vector<std::string> arpModeNameList = {"UP",          "DOWN",        "UP/DOWN", "DOWN/UP",
                                                      "UP R/DOWN R", "DOWN R/UP R", "UP 2",    "DOWN 2",
                                                      "UP 3",        "DOWN 3",      "ORDER",   "RANNDOM"};
};

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

    void update();
    void clockHandling();
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
