#include "moduleCallbacks.hpp"
#include "layer/layer.hpp"
#include "render/renderAudio.hpp"

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

extern std::vector<Layer *> allLayers;
extern LiveData liveData;

void lfoFreqSnap(uint32_t layerID, LFO &lfo) {
    if (lfo.dFreqSnap) {
        lfo.dFreq.displayVis = true;
        lfo.aFreq.displayVis = false;
    }
    else {
        lfo.dFreq.displayVis = false;
        lfo.aFreq.displayVis = true;
    }
}

void layer0lfoA() {
    lfoFreqSnap(0, allLayers[0]->lfoA);
}
void layer0lfoB() {
    lfoFreqSnap(0, allLayers[0]->lfoB);
}
void layer1lfoA() {
    lfoFreqSnap(1, allLayers[1]->lfoA);
}
void layer1lfoB() {
    lfoFreqSnap(1, allLayers[1]->lfoB);
}

void layer0WaveShaperX1() {
    allLayers[0]->waveshaper.aPoint1X.valueMapped =
        std::min((float)allLayers[0]->waveshaper.aPoint1X, allLayers[0]->waveshaper.aPoint2X - 0.001f);
}
void layer0WaveShaperX2() {
    allLayers[0]->waveshaper.aPoint2X.valueMapped =
        std::min((float)allLayers[0]->waveshaper.aPoint2X, allLayers[0]->waveshaper.aPoint3X - 0.001f);
    allLayers[0]->waveshaper.aPoint2X.valueMapped =
        std::max((float)allLayers[0]->waveshaper.aPoint2X, allLayers[0]->waveshaper.aPoint1X + 0.001f);
}
void layer0WaveShaperX3() {
    allLayers[0]->waveshaper.aPoint3X.valueMapped =
        std::max((float)allLayers[0]->waveshaper.aPoint2X, allLayers[0]->waveshaper.aPoint3X - 0.001f);
}

void layer1WaveShaperX1() {
    allLayers[1]->waveshaper.aPoint1X.valueMapped =
        std::min((float)allLayers[1]->waveshaper.aPoint1X, allLayers[1]->waveshaper.aPoint2X - 0.001f);
}
void layer1WaveShaperX2() {
    allLayers[1]->waveshaper.aPoint2X.valueMapped =
        std::min((float)allLayers[1]->waveshaper.aPoint2X, allLayers[1]->waveshaper.aPoint3X - 0.001f);
    allLayers[1]->waveshaper.aPoint2X.valueMapped =
        std::max((float)allLayers[1]->waveshaper.aPoint2X, allLayers[1]->waveshaper.aPoint1X + 0.001f);
}
void layer1WaveShaperX3() {
    allLayers[1]->waveshaper.aPoint3X.valueMapped =
        std::max((float)allLayers[1]->waveshaper.aPoint2X, allLayers[1]->waveshaper.aPoint3X - 0.001f);
}

void setModuleCallbacks() {
    allLayers[0]->lfoA.dFreqSnap.setValueChangedCallback(layer0lfoA);
    allLayers[0]->lfoB.dFreqSnap.setValueChangedCallback(layer0lfoB);
    allLayers[1]->lfoA.dFreqSnap.setValueChangedCallback(layer1lfoA);
    allLayers[1]->lfoB.dFreqSnap.setValueChangedCallback(layer1lfoB);

    allLayers[0]->waveshaper.aPoint1X.setValueChangedCallback(layer0WaveShaperX1);
    allLayers[0]->waveshaper.aPoint2X.setValueChangedCallback(layer0WaveShaperX2);
    allLayers[0]->waveshaper.aPoint3X.setValueChangedCallback(layer0WaveShaperX3);
    allLayers[1]->waveshaper.aPoint1X.setValueChangedCallback(layer1WaveShaperX1);
    allLayers[1]->waveshaper.aPoint2X.setValueChangedCallback(layer1WaveShaperX2);
    allLayers[1]->waveshaper.aPoint3X.setValueChangedCallback(layer1WaveShaperX3);
}

#endif