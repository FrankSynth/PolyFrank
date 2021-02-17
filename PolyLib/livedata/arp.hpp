#pragma once

#include "clock.hpp"
#include "liveDataBase.hpp"
#include "voiceHandler.hpp"

#define ARP_UP 0
#define ARP_DN 1
#define ARP_UD 2
#define ARP_DU 3
#define ARP_URDR 4
#define ARP_DRUR 5
#define ARP_UP2 6
#define ARP_DN2 7
#define ARP_UP3 8
#define ARP_DN3 9
#define ARP_ORDR 10
#define ARP_RND 11

class Arpeggiator {
  public:
    Arpeggiator(VoiceHandler *voiceHandler) {

        this->voiceHandler = voiceHandler;

        __liveSettingsArp.category = "ARP";
        __liveSettingsArp.settings.push_back(&arpEnable);
        __liveSettingsArp.settings.push_back(&arpLatch);
        __liveSettingsArp.settings.push_back(&arpMode);
        __liveSettingsArp.settings.push_back(&arpOctave);
        __liveSettingsArp.settings.push_back(&arpPresseKeysParallel);
        __liveSettingsArp.settings.push_back(&arpRep);
        __liveSettingsArp.settings.push_back(&arpPolyrythm);
        __liveSettingsArp.settings.push_back(&arpStepsA);
        __liveSettingsArp.settings.push_back(&arpStepsB);
    }

    // functions
    void keyPressed(Key &key);
    void keyReleased(Key &key);

    void pressKey(Key key);
    void lifetime(Key &key);

    void setSustain(uint8_t sustain);
    void checkLatch();
    void ratched();
    void release();
    void serviceRoutine();

    void mode_down();
    void mode_down2();
    void mode_down3();
    void mode_ordr();
    void mode_up2();
    void mode_up3();
    void mode_rnd();
    void mode_uprdownr();
    void mode_downup();

    void mode_updown();
    void mode_downrupr();

    void orderKeys();
    void nextStep();

    void decreaseArpOct();
    void increaseArpOct();

    uint16_t direction = 1;        // arp direction for updown etc, 1 = up
    uint16_t octaveDirection = 1;  // arp octave direction for updown etc, 1 = up
    uint16_t retrigger = 0;        // clears Arp Array on next iteration
    uint16_t triggeredNewNote = 0; // Arp has a new step to send out via middleman
    uint16_t stepRepeat = 1;       // arp repeats step, for upRdownR, etc
    uint16_t restarted = 0;        // arp was reset
    uint16_t currentOctave = 0;    // current arp octave being played

    uint16_t stepArp = 0; // current arp step
    uint16_t stepSeq = 0; // current seq step

    Key lastKey; // always holds the last played key, if no keys are pressed
    Key arpKey;  // always holds the last played key, if no keys are pressed

    uint16_t randomCounter;

    std::list<Key> pressedKeys;
    std::list<Key> inputKeys;

    std::vector<Key> orderedKeys;

    uint16_t keyListPosition;

    uint8_t allKeysReleased = 0;
    uint8_t arpSustain = 0;

    VoiceHandler *voiceHandler;

    categoryStruct __liveSettingsArp;
    Setting arpEnable = Setting("ARPEGGIATOR", 0, 0, 1, false, binary, &offOnNameList);
    Setting arpMode = Setting("MODE", 0, 0, 11, false, binary, &arpModeNameList);
    Setting arpLatch = Setting("LATCH", 0, 0, 1, false, binary, &offOnNameList);
    Setting arpOctave = Setting("OCTAVE", 0, -3, 3, false, binary, &arpOctaveNameList);
    Setting arpRep = Setting("REP", 0, 0, 3, false, binary, &arpRepNameList);
    Setting arpPolyrythm = Setting("POLYRYTHM", 0, 0, 1, false, binary, &offOnNameList);

    Setting arpStepsA = Setting("STEPA", 9, 0, 22, false, binary, &arpStepNameList);
    Setting arpStepsB = Setting("STEPB", 9, 0, 22, false, binary, &arpStepNameList);

    Setting arpPresseKeysParallel = Setting("KEYS PARALLEL", 1, 1, 8, false, binary);

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
