#ifdef POLYCONTROL
#include "liveData.hpp"

LiveData liveData;

void LiveData::controlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    // TODO zu Midi Modul weiterleiten

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

void LiveData::update() {

    arpA.serviceRoutine();
    arpB.serviceRoutine();

    clockHandling();
}

void LiveData::clockHandling() {
    // TODO LFO

    if (clock.ticked) {
        for (uint8_t i = 0; i < NUMBERLAYER; i++) {
            if (!(clock.counter % clockTicksPerStep[arps[i]->arpStepsA.value]) ||
                (!(clock.counter % clockTicksPerStep[arps[i]->arpStepsB.value]) && arps[i]->arpPolyrythm.value)) {
                arps[i]->nextStep();

                // println("nextStep");
            }
        }
        clock.ticked = 0;
    }
}

#endif
