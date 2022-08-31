#include "moduleCallbacks.hpp"
#include "layer/layer.hpp"
#include "render/renderAudio.hpp"
#include "render/renderAudioDef.h"

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

extern midi::MidiInterface<midiUSB::COMusb> midiDeviceUSB;
extern midi::MidiInterface<COMdin> midiDeviceDIN;

void lfoFreqSnap(LFO *lfo) {
    if (lfo->dFreqSnap) {
        lfo->dFreq.displayVis = true;
        lfo->aFreq.displayVis = false;
    }
    else {
        lfo->dFreq.displayVis = false;
        lfo->aFreq.displayVis = true;
    }
}

void retriggerLFOforAlign(LFO *lfo) {
    if (lfo->dAlignLFOs == 1) {
        layerCom.sendRetrigger(lfo->layerId, lfo->id, 0);
        layerCom.sendRetrigger(lfo->layerId, lfo->id, 4);
    }
}

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
    arp->voiceHandler->reset(arp->layerID);
}

void setModuleCallbacks() {
    allLayers[0]->lfoA.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[0]->lfoA));
    allLayers[0]->lfoB.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[0]->lfoB));
    allLayers[1]->lfoA.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[1]->lfoA));
    allLayers[1]->lfoB.dFreqSnap.setValueChangedCallback(std::bind(lfoFreqSnap, &allLayers[1]->lfoB));

    allLayers[0]->lfoA.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[0]->lfoA));
    allLayers[0]->lfoB.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[0]->lfoB));
    allLayers[1]->lfoA.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[1]->lfoA));
    allLayers[1]->lfoB.dAlignLFOs.setValueChangedCallback(std::bind(retriggerLFOforAlign, &allLayers[1]->lfoB));

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
}

#endif