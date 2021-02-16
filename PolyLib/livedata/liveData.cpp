#ifdef POLYCONTROL
#include "liveData.hpp"

LiveData liveData;
extern COMinterChip layerCom[2];

void LiveData::controlChange(uint8_t channel, uint8_t cc, uint8_t value) {

    if (channel == globalSettings.midiLayerAChannel.value) {
        switch (cc) {
            case 64: voiceHandler.setSustain(value, 0); break;
        }
    }

    if (channel == globalSettings.midiLayerBChannel.value) {
        switch (cc) {
            case 64: voiceHandler.setSustain(value, 1); break;
        }
    }

    // TODO zu Midi Modul weiterleiten,
    // mod(cc1), pitchbend (cc2) - pitchbend hat eine eigene function, weil native 14 bit

    // TODO NRPN implementation

    // Sustain liegt im voiceHandler
    // Sustain muss auch in den Arp
}

// functions
void LiveData::keyPressed(uint8_t channel, uint8_t note, uint8_t velocity) {

    Key key;
    key.note = note;
    key.velocity = velocity;

    // check Midi channel

    if (channel == globalSettings.midiLayerAChannel.value) {                         // check midi Channel
        if ((livemodeKeysplit.value && key.note >= 44) || !livemodeKeysplit.value) { // Key split ? + upper half
            key.layerID = 0;

            if (!arpA.arpEnable.value) {    // arp aus?
                voiceHandler.playNote(key); // directMode
            }
            arpA.keyPressed(key);
        }
    }

    if (channel == globalSettings.midiLayerBChannel.value) {
        if ((livemodeKeysplit.value && key.note < 44) || !livemodeKeysplit.value) {
            key.layerID = 1;
            if (!arpB.arpEnable.value) {
                voiceHandler.playNote(key);
            }
            arpB.keyPressed(key);
        }
    }
}
void LiveData::keyReleased(uint8_t channel, uint8_t note) {

    Key key;
    key.note = note;

    if (channel == globalSettings.midiLayerAChannel.value) { // check midi Channel
        key.layerID = 0;

        if (!arpA.arpEnable.value) {    // arp aus?
            voiceHandler.freeNote(key); // directMode
        }
        arpA.keyReleased(key);
    }

    if (channel == globalSettings.midiLayerBChannel.value) {
        key.layerID = 1;

        if (!arpB.arpEnable.value) {
            voiceHandler.freeNote(key);
        }
        arpB.keyReleased(key);
    }
}

void LiveData::clockTick() {
    clock.tick();
}

void LiveData::serviceRoutine() {

    arpA.serviceRoutine();
    arpB.serviceRoutine();

    clockHandling();
}

void LiveData::clockHandling() {
    // TODO LFO

    if (clock.ticked) {
        for (uint8_t i = 0; i < NUMBERLAYER; i++) {

            // ARP Steps
            if (!(clock.counter % clockTicksPerStep[arps[i]->arpStepsA.value]) ||
                (!(clock.counter % clockTicksPerStep[arps[i]->arpStepsB.value]) && arps[i]->arpPolyrythm.value)) {
                arps[i]->nextStep();
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
            // ADSR Sync
            // if (allLayers[i]->adsrA.dClockSync.value && !(clock.counter % allLayers[i]->adsrA.dClock.value)) {
            //     layerCom[i].sendRetrigger(allLayers[i]->adsrA.id, 8); // all voices = 8
            // }
            // if (allLayers[i]->adsrB.dClockSync.value && !(clock.counter % allLayers[i]->adsrB.dClock.value)) {
            //     layerCom[i].sendRetrigger(allLayers[i]->adsrB.id, 8); // all voices = 8
            // }
        }
        clock.ticked = 0;
    }
}

#endif
