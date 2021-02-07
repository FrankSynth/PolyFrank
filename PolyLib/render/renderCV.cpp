
#ifdef POLYRENDER

#include "renderCV.hpp"
#include "renderADSR.hpp"
#include "renderLFO.hpp"

extern MCP4728 cvDacA;
extern MCP4728 cvDacB;
extern MCP4728 cvDacC;

extern Layer layerA;

void initCVRendering() {
    //
}

void collectAllCurrentInputs() {
    for (BaseModule *m : layerA.modules) {
        for (Input *i : m->inputs) {
            i->collectCurrentSample();
        }
    }
}

void updateAllOutputSamples() {
    for (BaseModule *m : layerA.modules) {
        for (Output *o : m->outputs) {
            o->updateToNextSample();
        }
    }
}

// Render Functions

void renderMidiModule(Midi midi) {

    midi.oMod.nextSample[0] = midi.aMod.valueMapped;
    midi.oMod.nextSample[1] = midi.aMod.valueMapped;
    midi.oMod.nextSample[2] = midi.aMod.valueMapped;
    midi.oMod.nextSample[3] = midi.aMod.valueMapped;
    midi.oAftertouch.nextSample[0] = midi.aAftertouch.valueMapped;
    midi.oAftertouch.nextSample[1] = midi.aAftertouch.valueMapped;
    midi.oAftertouch.nextSample[2] = midi.aAftertouch.valueMapped;
    midi.oAftertouch.nextSample[3] = midi.aAftertouch.valueMapped;
    midi.oPitchbend.nextSample[0] = midi.aPitchbend.valueMapped;
    midi.oPitchbend.nextSample[1] = midi.aPitchbend.valueMapped;
    midi.oPitchbend.nextSample[2] = midi.aPitchbend.valueMapped;
    midi.oPitchbend.nextSample[3] = midi.aPitchbend.valueMapped;
    midi.oVeloctiy.nextSample[0] = midi.aVelocity.valueMapped;
    midi.oVeloctiy.nextSample[1] = midi.aVelocity.valueMapped;
    midi.oVeloctiy.nextSample[2] = midi.aVelocity.valueMapped;
    midi.oVeloctiy.nextSample[3] = midi.aVelocity.valueMapped;
    midi.oGate.nextSample[0] = midi.dGate.valueMapped;
    midi.oGate.nextSample[1] = midi.dGate.valueMapped;
    midi.oGate.nextSample[2] = midi.dGate.valueMapped;
    midi.oGate.nextSample[3] = midi.dGate.valueMapped;
}

void renderOSC_A(OSC_A &osc_A) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderOSC_B(OSC_B &osc_B) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderSub(Sub &sub) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderNoise(Noise &noise) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderSteiner(Steiner &steiner) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderLadder(Ladder &ladder) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderDistortion(Distortion &distort) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void renderGlobalModule(GlobalModule globalModule) {

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void writeDataToDACBuffer() {

    cvDacA.data[0] = layerA.adsrA.out.currentSample[0] * 4095;
    cvDacA.data[1] = (layerA.lfoA.out.currentSample[0] + 1) * 2047;
    cvDacA.data[2] = layerA.test.aFreq.valueMapped * 4095;
    cvDacA.data[3] = (1 - layerA.test.aDistort.valueMapped) * 4095;
}

void renderCVs() {

    collectAllCurrentInputs();

    renderMidiModule(layerA.midi);
    renderOSC_A(layerA.oscA);
    renderOSC_B(layerA.oscB);
    renderSub(layerA.sub);
    renderNoise(layerA.noise);
    renderSteiner(layerA.steiner);
    renderLadder(layerA.ladder);
    renderDistortion(layerA.distort);
    renderLFO(layerA.lfoA);
    // renderLFO(layerA.lfoB);
    renderADSR(layerA.adsrA);
    // renderADSR(layerA.adsrB);
    renderGlobalModule(layerA.globalModule);

    // TODO ? still true? need to render global stuff like Filter Out Levels here

    updateAllOutputSamples();

    writeDataToDACBuffer();

    cvDacA.setLatchPin();
    cvDacB.setLatchPin();
    cvDacC.setLatchPin();

    cvDacA.fastUpdate();

    FlagHandler::cvDacAStarted = true;
    // cvDacB.fastUpdate();
    // cvDacC.fastUpdate();
}

#endif
