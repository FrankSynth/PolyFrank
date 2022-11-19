#include "moduleCallbacks.hpp"
#include "layer/layer.hpp"
#include "render/renderAudio.hpp"
#include "render/renderAudioDef.h"
#include "wavetables/wavetables.hpp"

extern void setGUIColor(int32_t *colorSelection);

#ifdef POLYRENDER

#include "wavetables/wavetables.hpp"

extern Layer layerA;

void oscAdSample0Callback() {
    switchOscAWavetable(0, wavetables[layerA.oscA.dSample0.valueMapped]);
}
void oscAdSample1Callback() {
    switchOscAWavetable(1, wavetables[layerA.oscA.dSample1.valueMapped]);
}
void oscAdSample2Callback() {
    switchOscAWavetable(2, wavetables[layerA.oscA.dSample2.valueMapped]);
}
void oscAdSample3Callback() {
    switchOscAWavetable(3, wavetables[layerA.oscA.dSample3.valueMapped]);
}

void oscBdSample0Callback() {
    switchOscBWavetable(0, wavetables[layerA.oscB.dSample0.valueMapped]);
}
void oscBdSample1Callback() {
    switchOscBWavetable(1, wavetables[layerA.oscB.dSample1.valueMapped]);
}
void oscBdSample2Callback() {
    switchOscBWavetable(2, wavetables[layerA.oscB.dSample2.valueMapped]);
}
void oscBdSample3Callback() {
    switchOscBWavetable(3, wavetables[layerA.oscB.dSample3.valueMapped]);
}

void setModuleCallbacks() {
    layerA.oscA.dSample0.setValueChangedCallback(oscAdSample0Callback);
    layerA.oscA.dSample1.setValueChangedCallback(oscAdSample1Callback);
    layerA.oscA.dSample2.setValueChangedCallback(oscAdSample2Callback);
    layerA.oscA.dSample3.setValueChangedCallback(oscAdSample3Callback);

    layerA.oscB.dSample0.setValueChangedCallback(oscBdSample0Callback);
    layerA.oscB.dSample1.setValueChangedCallback(oscBdSample1Callback);
    layerA.oscB.dSample2.setValueChangedCallback(oscBdSample2Callback);
    layerA.oscB.dSample3.setValueChangedCallback(oscBdSample3Callback);
}

#endif

#ifdef POLYCONTROL

#include "livedata/liveData.hpp"
#include "midiInterface/MIDIInterface.h"

extern std::vector<Layer *> allLayers;
extern GlobalSettings globalSettings;
extern LiveData liveData;
extern COMinterChip layerCom;
extern Clock clock;

extern midi::MidiInterface<midiUSB::COMusb> midiDeviceUSB;
extern midi::MidiInterface<COMdin> midiDeviceDIN;

void lfoFreqSnap(LFO *lfo) {
    if (lfo->dFreqSnap) {
        // lfo->dFreq.displayVis = true;
        // lfo->aFreq.displayVis = false;
    }
    else {
        // lfo->dFreq.displayVis = false;
        // lfo->aFreq.displayVis = true;
    }
}

void lfoSetSnappedFreqFromKnob(LFO *lfo) {
    lfo->dFreq.valueMapped = std::round(lfo->aFreq.valueMapped * (float)lfo->dFreq.max);
    // layerCom.sendSetting(lfo->dFreq.layerId, lfo->dFreq.moduleId, lfo->dFreq.id, lfo->dFreq.valueMapped);
}

void lfoSetSnappedFreqFromList(LFO *lfo) {
    lfo->aFreq.valueMapped = (float)lfo->dFreq / lfo->dFreq.max;
    lfo->aFreq.value = lfo->aFreq.reverseMapping(lfo->aFreq.valueMapped);

    layerCom.sendSetting(lfo->aFreq.layerId, lfo->aFreq.moduleId, lfo->aFreq.id, lfo->aFreq.valueMapped);
}

// void retriggerLFOforAlign(LFO *lfo) {
//     if (lfo->dAlignLFOs == 1) {
//         layerCom.sendRetrigger(lfo->layerId, lfo->id, 0);
//         layerCom.sendRetrigger(lfo->layerId, lfo->id, 4);
//     }
// }

void clearComBufferForMidi() {
    midiDeviceDIN.mTransport.clear();
    midiDeviceUSB.mTransport.clear();
}

void layer0WaveShaperX1(Waveshaper *waveshaper) {
    waveshaper->aPoint1X.valueMapped = std::min((float)waveshaper->aPoint1X, waveshaper->aPoint2X - WAVESHAPERDISTANCE);
    waveshaper->aPoint1X.valueMapped = std::max((float)waveshaper->aPoint1X, WAVESHAPERDISTANCE);
    waveshaper->aPoint1X.value = waveshaper->aPoint1X.reverseMapping(waveshaper->aPoint1X.valueMapped);
}
void layer0WaveShaperX2(Waveshaper *waveshaper) {
    waveshaper->aPoint2X.valueMapped = std::min((float)waveshaper->aPoint2X, waveshaper->aPoint3X - WAVESHAPERDISTANCE);
    waveshaper->aPoint2X.valueMapped = std::max((float)waveshaper->aPoint2X, waveshaper->aPoint1X + WAVESHAPERDISTANCE);
    waveshaper->aPoint2X.value = waveshaper->aPoint2X.reverseMapping(waveshaper->aPoint2X.valueMapped);
}
void layer0WaveShaperX3(Waveshaper *waveshaper) {
    waveshaper->aPoint3X.valueMapped = std::min((float)waveshaper->aPoint3X, 1.0f - WAVESHAPERDISTANCE);
    waveshaper->aPoint3X.valueMapped = std::max((float)waveshaper->aPoint3X, waveshaper->aPoint2X + WAVESHAPERDISTANCE);
    waveshaper->aPoint3X.value = waveshaper->aPoint3X.reverseMapping(waveshaper->aPoint3X.valueMapped);
}

void layer0PhaseShaperX2(Phaseshaper *phaseshaper) {
    phaseshaper->aPoint2X.valueMapped = std::min((float)phaseshaper->aPoint2X, (float)phaseshaper->aPoint3X);
    phaseshaper->aPoint2X.value = phaseshaper->aPoint2X.reverseMapping(phaseshaper->aPoint2X.valueMapped);
}
void layer0PhaseShaperX3(Phaseshaper *phaseshaper) {
    phaseshaper->aPoint3X.valueMapped = std::max((float)phaseshaper->aPoint2X, (float)phaseshaper->aPoint3X);
    phaseshaper->aPoint3X.value = phaseshaper->aPoint3X.reverseMapping(phaseshaper->aPoint3X.valueMapped);
}

void resetVoiceHandler(Arpeggiator *arp) {
    if (arp->wasArpEnabled != arp->arpEnable.value)
        arp->voiceHandler->reset(arp->layerID);

    arp->wasArpEnabled = arp->arpEnable.value;
}

void tuneLadderCutoff(float *min, float *max) {
    allLayers[0]->ladder.aCutoff.setNewRange(*min, *max);
    allLayers[1]->ladder.aCutoff.setNewRange(*min, *max);
}
void tuneLadderRes(float *min, float *max) {
    allLayers[0]->ladder.aResonance.setNewRange(*min, *max);
    allLayers[1]->ladder.aResonance.setNewRange(*min, *max);
}
void tuneSteinerCutoff(float *min, float *max) {
    allLayers[0]->steiner.aCutoff.setNewRange(*min, *max);
    allLayers[1]->steiner.aCutoff.setNewRange(*min, *max);
}
void tuneSteinerRes(float *min, float *max) {
    allLayers[0]->steiner.aResonance.setNewRange(*min, *max);
    allLayers[1]->steiner.aResonance.setNewRange(*min, *max);
}

void updateLEDCurrent() {
    FlagHandler::ledDriverUpdateCurrent = true;
}

void wavetableSetCallbackOSC(uint32_t layerID, uint32_t oscillator, int32_t *value) {

    int32_t foundIndex[4] = {0, 0, 0, 0};

    for (uint32_t i = 0; i < 4; i++) {
        for (uint32_t t = 0; t < wavetables.size(); t++) {
            if (wavetables[t] == waveTableSets[*value][i]) {
                foundIndex[i] = (int32_t)t;
            }
        }
    }

    if (oscillator == 0) { // OSC A
        allLayers[layerID]->oscA.dSample0.setValueWithoutMapping(foundIndex[0]);
        allLayers[layerID]->oscA.dSample1.setValueWithoutMapping(foundIndex[1]);
        allLayers[layerID]->oscA.dSample2.setValueWithoutMapping(foundIndex[2]);
        allLayers[layerID]->oscA.dSample3.setValueWithoutMapping(foundIndex[3]);
    }

    else if (oscillator == 1) { // OSC B
        allLayers[layerID]->oscB.dSample0.setValueWithoutMapping(foundIndex[0]);
        allLayers[layerID]->oscB.dSample1.setValueWithoutMapping(foundIndex[1]);
        allLayers[layerID]->oscB.dSample2.setValueWithoutMapping(foundIndex[2]);
        allLayers[layerID]->oscB.dSample3.setValueWithoutMapping(foundIndex[3]);
    }
}

void setModuleCallbacks() {
    allLayers[0]->lfoA.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[0]->lfoA));
    allLayers[0]->lfoB.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[0]->lfoB));
    allLayers[1]->lfoA.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[1]->lfoA));
    allLayers[1]->lfoB.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[1]->lfoB));

    allLayers[0]->lfoA.aFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromKnob, &allLayers[0]->lfoA));
    allLayers[0]->lfoB.aFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromKnob, &allLayers[0]->lfoB));
    allLayers[1]->lfoA.aFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromKnob, &allLayers[1]->lfoA));
    allLayers[1]->lfoB.aFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromKnob, &allLayers[1]->lfoB));

    allLayers[0]->lfoA.dFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromList, &allLayers[0]->lfoA));
    allLayers[0]->lfoB.dFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromList, &allLayers[0]->lfoB));
    allLayers[1]->lfoA.dFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromList, &allLayers[1]->lfoA));
    allLayers[1]->lfoB.dFreq.setValueChangedCallback(std::bind(lfoSetSnappedFreqFromList, &allLayers[1]->lfoB));

    // allLayers[0]->lfoA.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[0]->lfoA));
    // allLayers[0]->lfoB.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[0]->lfoB));
    // allLayers[1]->lfoA.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[1]->lfoA));
    // allLayers[1]->lfoB.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[1]->lfoB));

    allLayers[0]->waveshaperA.aPoint1X.setValueChangedCallback(
        std::bind(layer0WaveShaperX1, &allLayers[0]->waveshaperA));
    allLayers[0]->waveshaperA.aPoint2X.setValueChangedCallback(
        std::bind(layer0WaveShaperX2, &allLayers[0]->waveshaperA));
    allLayers[0]->waveshaperA.aPoint3X.setValueChangedCallback(
        std::bind(layer0WaveShaperX3, &allLayers[0]->waveshaperA));
    allLayers[1]->waveshaperA.aPoint1X.setValueChangedCallback(
        std::bind(layer0WaveShaperX1, &allLayers[1]->waveshaperA));
    allLayers[1]->waveshaperA.aPoint2X.setValueChangedCallback(
        std::bind(layer0WaveShaperX2, &allLayers[1]->waveshaperA));
    allLayers[1]->waveshaperA.aPoint3X.setValueChangedCallback(
        std::bind(layer0WaveShaperX3, &allLayers[1]->waveshaperA));

    allLayers[0]->phaseshaperA.aPoint2X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX2, &allLayers[0]->phaseshaperA));
    allLayers[0]->phaseshaperA.aPoint3X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX3, &allLayers[0]->phaseshaperA));
    allLayers[1]->phaseshaperA.aPoint2X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX2, &allLayers[1]->phaseshaperA));
    allLayers[1]->phaseshaperA.aPoint3X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX3, &allLayers[1]->phaseshaperA));

    allLayers[0]->waveshaperB.aPoint1X.setValueChangedCallback(
        std::bind(layer0WaveShaperX1, &allLayers[0]->waveshaperB));
    allLayers[0]->waveshaperB.aPoint2X.setValueChangedCallback(
        std::bind(layer0WaveShaperX2, &allLayers[0]->waveshaperB));
    allLayers[0]->waveshaperB.aPoint3X.setValueChangedCallback(
        std::bind(layer0WaveShaperX3, &allLayers[0]->waveshaperB));
    allLayers[1]->waveshaperB.aPoint1X.setValueChangedCallback(
        std::bind(layer0WaveShaperX1, &allLayers[1]->waveshaperB));
    allLayers[1]->waveshaperB.aPoint2X.setValueChangedCallback(
        std::bind(layer0WaveShaperX2, &allLayers[1]->waveshaperB));
    allLayers[1]->waveshaperB.aPoint3X.setValueChangedCallback(
        std::bind(layer0WaveShaperX3, &allLayers[1]->waveshaperB));

    allLayers[0]->phaseshaperB.aPoint2X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX2, &allLayers[0]->phaseshaperB));
    allLayers[0]->phaseshaperB.aPoint3X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX3, &allLayers[0]->phaseshaperB));
    allLayers[1]->phaseshaperB.aPoint2X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX2, &allLayers[1]->phaseshaperB));
    allLayers[1]->phaseshaperB.aPoint3X.setValueChangedCallback(
        std::bind(layer0PhaseShaperX3, &allLayers[1]->phaseshaperB));

    liveData.arps[0].arpEnable.setValueChangedCallback(std::bind(resetVoiceHandler, &liveData.arps[0]));
    liveData.arps[1].arpEnable.setValueChangedCallback(std::bind(resetVoiceHandler, &liveData.arps[1]));

    globalSettings.midiSource.setValueChangedCallback(clearComBufferForMidi);

    liveData.voiceHandler.livemodeMergeLayer.setValueChangedCallback(
        std::bind(switchLiveMode, &(liveData.voiceHandler.livemodeMergeLayer.value)));

    globalSettings.dispColor.setValueChangedCallback(std::bind(setGUIColor, &globalSettings.dispColor.value));

    liveData.livemodeClockSource.setValueChangedCallback(
        std::bind(switchClockSourceCallback, &liveData.livemodeClockSource.value));

    // Tuning
    allLayers[0]->tune.tuneCutoffScaleLadder.setValueChangedCallback(std::bind(
        tuneLadderCutoff, &allLayers[0]->ladder.aCutoff.min, &allLayers[0]->tune.tuneCutoffScaleLadder.valueMapped));
    allLayers[0]->tune.tuneResonanceScaleLadder.setValueChangedCallback(std::bind(
        tuneLadderRes, &allLayers[0]->ladder.aResonance.min, &allLayers[0]->tune.tuneResonanceScaleLadder.valueMapped));

    allLayers[0]->tune.tuneCutoffScaleSteiner.setValueChangedCallback(std::bind(
        tuneSteinerCutoff, &allLayers[0]->steiner.aCutoff.min, &allLayers[0]->tune.tuneCutoffScaleSteiner.valueMapped));
    allLayers[0]->tune.tuneResonanceScaleSteiner.setValueChangedCallback(
        std::bind(tuneSteinerRes, &allLayers[0]->steiner.aResonance.min,
                  &allLayers[0]->tune.tuneResonanceScaleSteiner.valueMapped));

    allLayers[1]->tune.tuneCutoffScaleLadder.setValueChangedCallback(std::bind(
        tuneLadderCutoff, &allLayers[1]->ladder.aCutoff.min, &allLayers[1]->tune.tuneCutoffScaleLadder.valueMapped));
    allLayers[1]->tune.tuneResonanceScaleLadder.setValueChangedCallback(std::bind(
        tuneLadderRes, &allLayers[1]->ladder.aResonance.min, &allLayers[1]->tune.tuneResonanceScaleLadder.valueMapped));

    allLayers[1]->tune.tuneCutoffScaleSteiner.setValueChangedCallback(std::bind(
        tuneSteinerCutoff, &allLayers[1]->steiner.aCutoff.min, &allLayers[1]->tune.tuneCutoffScaleSteiner.valueMapped));
    allLayers[1]->tune.tuneResonanceScaleSteiner.setValueChangedCallback(
        std::bind(tuneSteinerRes, &allLayers[1]->steiner.aResonance.min,
                  &allLayers[1]->tune.tuneResonanceScaleSteiner.valueMapped));

    globalSettings.dispLED.setValueChangedCallback(std::bind(updateLEDCurrent));

    allLayers[0]->oscA.dWavetableSet.setValueChangedCallback(
        std::bind(wavetableSetCallbackOSC, 0, 0, &(allLayers[0]->oscA.dWavetableSet.valueMapped)));
    allLayers[0]->oscB.dWavetableSet.setValueChangedCallback(
        std::bind(wavetableSetCallbackOSC, 0, 1, &(allLayers[0]->oscB.dWavetableSet.valueMapped)));

    allLayers[1]->oscA.dWavetableSet.setValueChangedCallback(
        std::bind(wavetableSetCallbackOSC, 1, 0, &(allLayers[1]->oscA.dWavetableSet.valueMapped)));
    allLayers[1]->oscB.dWavetableSet.setValueChangedCallback(
        std::bind(wavetableSetCallbackOSC, 1, 1, &(allLayers[1]->oscB.dWavetableSet.valueMapped)));
}

#endif