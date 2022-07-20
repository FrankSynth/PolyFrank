#ifdef POLYCONTROL
#include "liveData.hpp"
#include "com/comCommands.h"
#include "midiInterface/midi_Defs.h"

#define MIDIRESOLUTION7 127
#define MIDIRESOLUTION14 16383

LiveData liveData;
extern COMinterChip layerCom;

const std::vector<const char *> offOnNameList = {"OFF", "ON"};
const std::vector<const char *> clockSourceList = {"EXTERN", "MIDI", "INTERN"};
const std::vector<const char *> externalClockMultList = {"1/32", "1/16", "1/8", "1/4", "1/2", "1/1"};

uint16_t extClockMultiply[] = {3, 6, 12, 24, 48, 96};

void LiveData::controlChange(uint8_t channel, uint8_t cc, int16_t value) {

    if (channel == globalSettings.midiLayerAChannel.value ||
        globalSettings.midiLayerAChannel.value == 0) { // Check Midi Channel Layer A
        distributeCC(cc, value, 0);
    }

    if (channel == globalSettings.midiLayerBChannel.value ||
        globalSettings.midiLayerBChannel.value == 0) { // Check Midi Channel Layer A
        distributeCC(cc, value, 1);
    }

    // TODO NRPN implementation
}
void LiveData::distributeCC(uint8_t cc, int16_t value, uint8_t layer) {

    switch (cc) {
        case midi::ModulationWheel: allLayers[layer]->midi.aMod.setValue(value); break;
        case midi::Sustain:
            arps[layer].setSustain(value);         // set arp Sustain
            if (arps[layer].arpEnable.value) {     // check arp is on?
                voiceHandler.setSustain(0, layer); // disable voiceHandler Sustain
            }
            else {
                voiceHandler.setSustain(value, layer); // enable VoiceHandler Sustain
            }
            break;
        case midi::AfterTouchChannel: allLayers[layer]->midi.aAftertouch.setValue(value); break;
        case midi::PitchBend: allLayers[layer]->midi.aPitchbend.setValue(value); break;
    }
}

// functions
void LiveData::keyPressed(uint8_t channel, uint8_t note, uint8_t velocity) {

    Key key;
    key.note = note;
    key.velocity = velocity;

    // check Midi channel

    if (channel == globalSettings.midiLayerAChannel.value ||
        globalSettings.midiLayerAChannel.value == 0) { // check midi Channel

        // TODO split note als setting??  -> verstehe nicht genau was du meinst, key split teil obere und untere
        // keyboard hälfte auf die layer auf
        if ((livemodeKeysplit.value && key.note >= 64) || !livemodeKeysplit.value) { // Key split ? + upper half
            key.layerID = 0;

            if (!arps[0].arpEnable.value) { // arp aus?
                voiceHandler.playNote(key); // directMode
            }
            arps[0].keyPressed(key);
        }
    }

    if (channel == globalSettings.midiLayerBChannel.value || globalSettings.midiLayerBChannel.value == 0) {

        if ((livemodeKeysplit.value && key.note < 64) || !livemodeKeysplit.value) {
            key.layerID = 1;
            if (!arps[1].arpEnable.value) {
                voiceHandler.playNote(key);
            }
            arps[1].keyPressed(key);
        }
    }
}
void LiveData::keyReleased(uint8_t channel, uint8_t note) {

    Key key;
    key.note = note;

    if (channel == globalSettings.midiLayerAChannel.value ||
        globalSettings.midiLayerAChannel.value == 0) { // check midi Channel
        key.layerID = 0;

        if (!arps[0].arpEnable.value) { // arp aus?
            voiceHandler.freeNote(key); // directMode
        }
        arps[0].keyReleased(key);
    }

    if (channel == globalSettings.midiLayerBChannel.value || globalSettings.midiLayerBChannel.value == 0) {
        key.layerID = 1;

        if (!arps[1].arpEnable.value) {
            voiceHandler.freeNote(key);
        }
        arps[1].keyReleased(key);
    }
}

void LiveData::receivedStart() {

    if (livemodeClockSource.value == 1) { // clock source == midi

        clock.reset();
        for (byte i = 0; i < 2; i++) {
            arps[i].restart();
        }
    }
}
void LiveData::receivedContinue() {

    if (livemodeClockSource.value == 1) { // clock source == midi
        // clock.reset();
        for (byte i = 0; i < 2; i++) {
            arps[i].continueRestart();
        }
    }
}
void LiveData::receivedStop() {
    if (livemodeClockSource.value == 1) { // clock source == midi

        for (byte i = 0; i < 2; i++) {
            arps[i].reset();
        }
    }
}

void LiveData::receivedReset() {
    for (byte i = 0; i < 2; i++) {
        arps[i].reset();
        voiceHandler.reset(i);
    }
}

void LiveData::receivedMidiSongPosition(unsigned int spp) {
    if (livemodeClockSource.value == 1) { // clock source == midi
                                          // clock.receivedNewSPP = 1;
        clock.reset();
        clock.counter = (spp * 6) % MAXCLOCKTICKS;
    }
}

void LiveData::midiClockTick() {
    if (livemodeClockSource.value == 1) { // clock source == midi
        arps[0].midiUpdateDelayTimer = 0;
        arps[1].midiUpdateDelayTimer = 0;
        clock.tick();
    }
}

void LiveData::internalClockTick() {
    if (livemodeClockSource.value == 2) { // clock source == internal tick
        clock.tick();
    }
}

void LiveData::externalClockTick() {
    if (livemodeClockSource.value == 0) { // clock source == external Sync
        for (uint32_t i = 0; i < extClockMultiply[livemodeExternalClockMultiply.value]; i++) {
            clock.tick();
        }
    }
}

void LiveData::serviceRoutine() {

    internalClock.serviceRoutine();
    arps[0].serviceRoutine();
    arps[1].serviceRoutine();

    clockHandling();
}

void LiveData::clockHandling() {
    if (clock.ticked == 0)
        return;

    for (uint8_t i = 0; i < 2; i++) {
        if (allLayers[i]->layerState.value == 1) { // check layer state

            // ARP Steps
            if (!(clock.counter % clockTicksPerStep[arps[i].arpStepsA.value]) ||
                (!(clock.counter % clockTicksPerStep[arps[i].arpStepsB.value]) && arps[i].arpPolyrythm.value)) {
                arps[i].nextStep();
            }

            // LFO Sync

            for (LFO *lfo : allLayers[i]->lfos) {

                if (lfo->dClockTrigger == 0)
                    continue;

                uint32_t clockTicks = clockTicksPerStep[lfo->dClockStep];

                if (clock.counter % clockTicks == 0) {
                    layerCom.sendRetrigger(i, lfo->id, VOICEALL); // all voices = 8
                }
            }

            for (ADSR *adsr : allLayers[i]->adsrs) {
                if (adsr->dClockTrigger == 0)
                    continue;

                uint32_t clockTicks = clockTicksPerStep[adsr->dClockStep];
                if (clock.counter % clockTicks == 0) {
                    layerCom.sendRetrigger(i, adsr->id, VOICEALL);
                }
            }
        }
    }

    calcAllLFOSnapFreq();

    clock.ticked = 0;
}

void LiveData::calcLFOSnapFreq(LFO &lfo) {
    if (lfo.dFreqSnap == 0)
        return;

    float freq = clock.bpm * multTime[lfo.dFreq] / 60.0f;
    lfo.aFreq.setValueWithoutMapping(freq);
}

void LiveData::calcAllLFOSnapFreq() {
    for (uint8_t i = 0; i < 2; i++) {
        for (LFO *lfo : allLayers[i]->lfos)
            calcLFOSnapFreq(*lfo);
    }
}

void LiveData::saveLiveDataSettings() {
    uint32_t index = 0;
    int32_t *buffer = (int32_t *)blockBuffer;

    for (Setting *s : __liveSettingsLivemode.settings) {
        buffer[index] = s->value;
        index++;
    }
    for (Setting *s : arps[0].__liveSettingsArp.settings) {
        buffer[index] = s->value;
        index++;
    }
    for (Setting *s : arps[1].__liveSettingsArp.settings) {
        buffer[index] = s->value;
        index++;
    }

    writeLiveDataBlock();

    if ((uint32_t)((uint8_t *)&buffer[index] - (uint8_t *)blockBuffer) > (LIVEDATA_BLOCKSIZE)) {
        PolyError_Handler("ERROR | FATAL | LiveDataSettings -> saveLiveDataSettings -> BufferOverflow!");
    }
}

void LiveData::loadLiveDataSettings() {
    uint32_t index = 0;
    int32_t *buffer = (int32_t *)blockBuffer;

    readLiveData();

    for (Setting *s : __liveSettingsLivemode.settings) {
        s->setValue(buffer[index]);
        index++;
    }
    for (Setting *s : arps[0].__liveSettingsArp.settings) {
        s->setValue(buffer[index]);
        index++;
    }
    for (Setting *s : arps[1].__liveSettingsArp.settings) {
        s->setValue(buffer[index]);
        index++;
    }
}

#endif
