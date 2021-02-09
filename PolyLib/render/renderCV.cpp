#ifdef POLYRENDER

#include "renderCV.hpp"
#include "renderADSR.hpp"
#include "renderLFO.hpp"
#include "renderNoise.hpp"
#include "renderOSC.hpp"
#include "renderSub.hpp"

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
    for (RenderBuffer *b : layerA.oscA.renderBuffer) {
        b->updateToNextSample();
    }
    for (RenderBuffer *b : layerA.oscB.renderBuffer) {
        b->updateToNextSample();
    }
    for (RenderBuffer *b : layerA.noise.renderBuffer) {
        b->updateToNextSample();
    }
    for (RenderBuffer *b : layerA.sub.renderBuffer) {
        b->updateToNextSample();
    }
}

// Render Functions

void renderMidiModule(Midi midi) {
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        midi.oMod.nextSample[voice] = midi.aMod.valueMapped;
        midi.oAftertouch.nextSample[voice] = midi.aAftertouch.valueMapped;
        midi.oPitchbend.nextSample[voice] = midi.aPitchbend.valueMapped;
        midi.oNote.nextSample[voice] = (float)midi.rawNote[voice] / 127.0f;
        midi.oVeloctiy.nextSample[voice] = (float)midi.rawVelocity[voice] / 127.0f;
        midi.oGate.nextSample[voice] = midi.rawGate[voice];
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
    cvDacA.data[2] = layerA.noise.out.currentSample[0] * 4095;
    cvDacA.data[3] = 0;
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
    renderLFO(layerA.lfoB);
    renderADSR(layerA.adsrA);
    renderADSR(layerA.adsrB);
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
