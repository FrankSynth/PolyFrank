#ifdef POLYCONTROL
#include "liveData.hpp"
#include "midiInterface/midi_Defs.h"

#define MIDIRESOLUTION7 127
#define MIDIRESOLUTION14 16383

LiveData liveData;
extern COMinterChip layerCom[2];

uint16_t extClockMultiply[] = {3, 6, 12, 24, 48, 96};

void LiveData::controlChange(uint8_t channel, uint8_t cc, int16_t value) {

    if (channel == globalSettings.midiLayerAChannel.value) { // Check Midi Channel Layer A
        distributeCC(cc, value, 0);
    }

    if (channel == globalSettings.midiLayerBChannel.value) { // Check Midi Channel Layer A
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

    if (channel == globalSettings.midiLayerAChannel.value) { // check midi Channel

        // TODO split note als setting??
        if ((livemodeKeysplit.value && key.note >= 64) || !livemodeKeysplit.value) { // Key split ? + upper half
            key.layerID = 0;

            if (!arps[0].arpEnable.value) { // arp aus?
                voiceHandler.playNote(key); // directMode
            }
            arps[0].keyPressed(key);
        }
    }

    if (channel == globalSettings.midiLayerBChannel.value) {

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

    if (channel == globalSettings.midiLayerAChannel.value) { // check midi Channel
        key.layerID = 0;

        if (!arps[0].arpEnable.value) { // arp aus?
            voiceHandler.freeNote(key); // directMode
        }
        arps[0].keyReleased(key);
    }

    if (channel == globalSettings.midiLayerBChannel.value) {
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
        for (byte i = 0; i < 2; i++) {
            arps[i].midiUpdateDelayTimer = 0;
        }
    }
}
void LiveData::receivedStop() {
    if (livemodeClockSource.value == 1) { // clock source == midi

        for (byte i = 0; i < 2; i++) {
            arps[i].midiUpdateDelayTimer = 0;
        }
    }
}

void LiveData::receivedReset() {
    // reset();
    // TODO reset and default everything
}

void LiveData::midiClockTick() {
    if (livemodeClockSource.value == 1) { // clock source == midi
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
        for (uint16_t i = 0; i < extClockMultiply[livemodeExternalClockMultiply.value]; i++) {
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
    if (clock.ticked) {
        arps[0].midiUpdateDelayTimer = 0;
        arps[1].midiUpdateDelayTimer = 0;

        for (uint8_t i = 0; i < 2; i++) {
            if (allLayers[i]->LayerState.value == 1) { // check layer state

                // ARP Steps
                if (!(clock.counter % clockTicksPerStep[arps[i].arpStepsA.value]) ||
                    (!(clock.counter % clockTicksPerStep[arps[i].arpStepsB.value]) && arps[i].arpPolyrythm.value)) {
                    arps[i].nextStep();
                }

                // LFO Sync

                uint32_t clockTicksPerStepLFOA = clockTicksPerStep[allLayers[i]->lfoA.dClockStep.valueMapped];

                if (allLayers[i]->lfoA.dClockSync.valueMapped && !(clock.counter % clockTicksPerStepLFOA)) {
                    layerCom[i].sendRetrigger(allLayers[i]->lfoA.id, 8); // all voices = 8

                    if (clock.counter % allLayers[i]->lfoA.dFreqSnap.valueMapped) {
                        float freq = (clock.bpm * 24 / 60) / (float)clockTicksPerStepLFOA / 60.;
                        allLayers[i]->lfoA.aFreq.setValueWithoutMapping(freq);
                    }
                }

                uint32_t clockTicksPerStepLFOB = clockTicksPerStep[allLayers[i]->lfoB.dClockStep.valueMapped];
                if (allLayers[i]->lfoB.dClockSync.valueMapped && !(clock.counter % clockTicksPerStepLFOB)) {
                    layerCom[i].sendRetrigger(allLayers[i]->lfoB.id, 8); // all voices = 8

                    if (clock.counter % allLayers[i]->lfoB.dFreqSnap.valueMapped) {
                        float freq = (clock.bpm * 24 / 60) / (float)clockTicksPerStepLFOB;
                        allLayers[i]->lfoB.aFreq.setValueWithoutMapping(freq);
                    }
                }
            }
        }
        clock.ticked = 0;
    }
}

#endif
