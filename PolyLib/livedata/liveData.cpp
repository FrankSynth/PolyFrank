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
const std::vector<const char *> extStepNameList = {"1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1"};

const uint32_t clockTicksPerExternalSyncOut[23] = {3, 6, 12, 24, 48, 96, 192, 384};

uint16_t extClockMultiply[] = {3, 6, 12, 24, 48, 96};

extern void nextLayer();
extern bool layerMergeMode;

void switchLiveMode(int32_t *setting) {
    if (*setting == 0) {
        println("INFO | PlayMode SINGLE");

        layerSendMode = SINGLELAYER;
        layerMergeMode = false;
        allLayers[0]->resendLayerConfig();
        allLayers[1]->resendLayerConfig();
        liveData.voiceHandler.livemodeVoiceModeB.disable = false; // disable
        liveData.voiceHandler.livemodeVoiceModeA.max = 3;
        liveData.voiceHandler.livemodeVoiceModeA.valueNameList = &liveData.voiceHandler.polySplitNameList;
    }
    else if (*setting == 1) {

        println("INFO | PlayMode DUAL");

        layerSendMode = DUALLAYER;
        layerMergeMode = true;
        allLayers[0]->resendLayerConfig();
        liveData.voiceHandler.livemodeVoiceModeB.disable = true; // enable
        liveData.voiceHandler.livemodeVoiceModeA.max = 4;
        liveData.voiceHandler.livemodeVoiceModeA.valueNameList = &liveData.voiceHandler.polySplitNameListDual;

        nextLayer(); // jump layer to A
    }
}

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
    if (voiceHandler.livemodeMergeLayer.value != 1) { // we are in 16 voice mode-> skip

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
    if (voiceHandler.livemodeMergeLayer.value != 1) { // we are in 16 voice mode-> skip
        if (channel == globalSettings.midiLayerBChannel.value || globalSettings.midiLayerBChannel.value == 0) {
            key.layerID = 1;

            if (!arps[1].arpEnable.value) {
                voiceHandler.freeNote(key);
            }
            arps[1].keyReleased(key);
        }
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
    if (livemodeClockSource.value != 1)
        return;

    extClockTimeout = 0;
    arps[0].midiUpdateDelayTimer = 0;
    arps[1].midiUpdateDelayTimer = 0;
    clock.tick();
}

void LiveData::internalClockTick() {
    if (livemodeClockSource.value == 2) { // clock source == internal tick
        clock.tick();
    }
}

void LiveData::externalClockTick() {
    if (liveData.livemodeClockSource.value != 0)
        return;

    extClockTimeout = 0;
    extSync = true;
    calcBPMfromExternalSource();
}
void LiveData::calcBPMfromExternalSource() {
    clock.bpm = (double)60000000.0 / (double)meassuredBPMTime;
    meassuredBPMTime = 0;
};

void LiveData::serviceRoutine() {

    internalClock.serviceRoutine();

    if (liveData.livemodeClockSource.value == 0) { // external clock mode
        externalSyncHandling();
    }

    else {
        clockHandling();
    }

    arps[0].serviceRoutine();
    arps[1].serviceRoutine();

    // reset  External Clock Signal Out
    if (extClockLengthTimer > (uint32_t)globalSettings.extClockOutLength.value) {
        HAL_GPIO_WritePin(IO_Sync_OUT_GPIO_Port, IO_Sync_OUT_Pin, GPIO_PIN_SET);
    }

    static float bpmCache = 0;
    if (clock.bpm != bpmCache) {
        bpmCache = clock.bpm;
        layerCom.sendBPM(clock.bpm);
    }
}

void switchClockSourceCallback(int32_t *setting) {

    switch (*setting) {
        case 0: {
            liveData.arps[0].arpStepsA.displayVis = false;
            liveData.arps[0].arpStepsB.displayVis = false;
            liveData.arps[1].arpStepsA.displayVis = false;
            liveData.arps[1].arpStepsB.displayVis = false;

            liveData.arps[0].arpStepsAExt.displayVis = true;
            liveData.arps[0].arpStepsBExt.displayVis = true;
            liveData.arps[1].arpStepsAExt.displayVis = true;
            liveData.arps[1].arpStepsBExt.displayVis = true;

            // allLayers[0]->lfoA.dClockStep.displayVis = false;
            // allLayers[0]->lfoB.dClockStep.displayVis = false;
            // allLayers[0]->lfoA.dEXTDiv.displayVis = true;
            // allLayers[0]->lfoB.dEXTDiv.displayVis = true;

            allLayers[0]->envA.dClockStep.displayVis = false;
            allLayers[0]->envF.dClockStep.displayVis = false;
            allLayers[0]->envA.dEXTDiv.displayVis = true;
            allLayers[0]->envF.dEXTDiv.displayVis = true;

            // allLayers[1]->lfoA.dClockStep.displayVis = false;
            // allLayers[1]->lfoB.dClockStep.displayVis = false;
            // allLayers[1]->lfoA.dEXTDiv.displayVis = true;
            // allLayers[1]->lfoB.dEXTDiv.displayVis = true;

            allLayers[1]->envA.dClockStep.displayVis = false;
            allLayers[1]->envF.dClockStep.displayVis = false;
            allLayers[1]->envA.dEXTDiv.displayVis = true;
            allLayers[1]->envF.dEXTDiv.displayVis = true;

            liveData.livemodeExternalClockMultiplyOut.disable = true;
            break;
        }
        default: {
            liveData.arps[0].arpStepsA.displayVis = true;
            liveData.arps[0].arpStepsB.displayVis = true;
            liveData.arps[1].arpStepsA.displayVis = true;
            liveData.arps[1].arpStepsB.displayVis = true;

            liveData.arps[0].arpStepsAExt.displayVis = false;
            liveData.arps[0].arpStepsBExt.displayVis = false;
            liveData.arps[1].arpStepsAExt.displayVis = false;
            liveData.arps[1].arpStepsBExt.displayVis = false;

            // allLayers[0]->lfoA.dClockStep.displayVis = true;
            // allLayers[0]->lfoB.dClockStep.displayVis = true;
            // allLayers[0]->lfoA.dEXTDiv.displayVis = false;
            // allLayers[0]->lfoB.dEXTDiv.displayVis = false;

            // allLayers[0]->envA.dClockStep.displayVis = true;
            // allLayers[0]->envF.dClockStep.displayVis = true;
            // allLayers[0]->envA.dEXTDiv.displayVis = false;
            // allLayers[0]->envF.dEXTDiv.displayVis = false;

            // allLayers[1]->lfoA.dClockStep.displayVis = true;
            // allLayers[1]->lfoB.dClockStep.displayVis = true;
            // allLayers[1]->lfoA.dEXTDiv.displayVis = false;
            // allLayers[1]->lfoB.dEXTDiv.displayVis = false;

            // allLayers[1]->envA.dClockStep.displayVis = true;
            // allLayers[1]->envF.dClockStep.displayVis = true;
            // allLayers[1]->envA.dEXTDiv.displayVis = false;
            // allLayers[1]->envF.dEXTDiv.displayVis = false;

            liveData.livemodeExternalClockMultiplyOut.disable = false;
            break;
        }
    }
}

void LiveData::externalSyncHandling() {

    if (extSync == false) // no ext signal received
        return;

    extSync = false;

    extSyncCounter++;

    // Mirror Input sync to output
    HAL_GPIO_WritePin(IO_Sync_OUT_GPIO_Port, IO_Sync_OUT_Pin, GPIO_PIN_RESET);
    extClockLengthTimer = 0;

    // Internal Clocking
    for (uint8_t i = 0; i < 2; i++) {
        if (allLayers[i]->layerState.value == 1) { // check layer state

            // ARP Steps
            if (!(extSyncCounter % extSyncPerStep[arps[i].arpStepsAExt.value]) ||
                (!(extSyncCounter % extSyncPerStep[arps[i].arpStepsBExt.value]) && arps[i].arpPolyrythm.value)) {
                arps[i].nextStep();
            }

            // LFO Sync

            for (LFO *lfo : allLayers[i]->lfos) {

                if (lfo->dClockTrigger == 0)
                    continue;

                uint32_t clockTicks = extSyncPerStep[lfo->dEXTDiv];

                if (extSyncCounter % clockTicks == 0) {
                    layerCom.sendRetrigger(i, lfo->id, VOICEALL); // all voices = 8
                }
            }

            for (ADSR *adsr : allLayers[i]->adsrs) {
                if (adsr->dClockTrigger == 0)
                    continue;

                uint32_t clockTicks = extSyncPerStep[adsr->dEXTDiv];
                if (extSyncCounter % clockTicks == 0) {
                    layerCom.sendRetrigger(i, adsr->id, VOICEALL);
                }
            }
        }
    }
}

void LiveData::clockHandling() {

    if (clock.ticked == 0) // no tick received
        return;

    // External Clock Signal Out
    if (!(clock.counter % clockTicksPerExternalSyncOut[livemodeExternalClockMultiplyOut.value])) {
        extClockLengthTimer = 0;
        HAL_GPIO_WritePin(IO_Sync_OUT_GPIO_Port, IO_Sync_OUT_Pin, GPIO_PIN_RESET);
    }

    // Internal Clocking
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

    clock.ticked = 0;
}

uint32_t LiveData::writeLive(uint32_t blockStartIndex) {

    StorageBlock *buffer = (StorageBlock *)&(blockBuffer[blockStartIndex]);
    StorageBlock block;

    uint32_t blockIndex = 0;

    block.dataType = STORE_LIVESETTING;
    for (Setting *i : __liveSettingsLivemode) {
        if (i->storeable) {
            block.id = i->storeID;
            block.data.asInt = i->value;
            storeToBlock(buffer, block, blockIndex);
        }
    }
    // Store Tuning
    block.dataType = STORE_ARPSETTING;
    for (uint8_t layer = 0; layer < 2; layer++) {
        for (Setting *i : arps[layer].__liveSettingsArp) {
            if (i->storeable) {
                block.id = i->storeID | layer << 7;
                block.data.asInt = i->value;
                storeToBlock(buffer, block, blockIndex);
            }
        }
    }
    return blockStartIndex + blockIndex * sizeof(StorageBlock);
}

// void LiveData::getLiveConfiguration(int32_t *buffer) {
//     uint32_t index = 0;

//     for (Setting *s : __liveSettingsLivemode.settings) {
//         if (s->storeable == 1) {
//             buffer[index] = s->value;
//             index++;
//         }
//     }
//     for (Setting *s : arps[0].__liveSettingsArp.settings) {
//         buffer[index] = s->value;
//         index++;
//     }
//     for (Setting *s : arps[1].__liveSettingsArp.settings) {
//         buffer[index] = s->value;
//         index++;
//     }
// }

// void LiveData::setLiveConfiguration(int32_t *buffer, LayerSelect layer) {
//     uint32_t index = 0;

//     for (Setting *s : __liveSettingsLivemode.settings) {
//         if (s->storeable == 1) {
//             s->setValue(buffer[index]);
//             index++;
//         }
//     }

//     for (Setting *s : arps[0].__liveSettingsArp.settings) {
//         if (layer == LAYER_AB || layer == LAYER_A) {
//             s->setValue(buffer[index]);
//         }
//         index++;
//     }
//     for (Setting *s : arps[1].__liveSettingsArp.settings) {
//         if (layer == LAYER_AB || layer == LAYER_B) {
//             s->setValue(buffer[index]);
//         }
//         index++;
//     }
// }

void LiveData::resetLiveConfig() {

    for (Setting *s : __liveSettingsLivemode) {
        s->resetValue();
    }
    for (Setting *s : arps[0].__liveSettingsArp) {
        s->resetValue();
    }
    for (Setting *s : arps[1].__liveSettingsArp) {
        s->resetValue();
    }
}

#endif
