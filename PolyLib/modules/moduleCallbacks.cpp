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

void setModuleCallbacks() {}

#endif