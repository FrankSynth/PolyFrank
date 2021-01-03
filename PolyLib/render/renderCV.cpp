#include "renderCV.hpp"

extern MCP4728 cvDacA;
extern MCP4728 cvDacB;
extern MCP4728 cvDacC;

extern Layer layerA;

void updateAllOutputSamples() {
    for (BaseModule *m : layerA.getModules()) {
        for (Output *o : m->getOutputs()) {
            o->updateToNextSample();
        }
    }
}

// TODO remove this later
void tempRendering() {
    static float dacStep = 0;

    dacStep += layerA.test.aCutoff.valueMapped / (1000.0f / (float)CVTIMERINTERVALUS); // 100hz sinus

    cvDacB.data[3] = (fast_sin_f32(dacStep) + 1) * 2047; // 100hz sinus
    // cvDacB.data[3] = 4095; // const

    if (dacStep > 1) {
        dacStep -= 1;
    }

    cvDacA.data[0] = layerA.test.aCutoff.valueMapped * 4095;
    cvDacA.data[1] = (1 - layerA.test.aResonance.valueMapped) * 4095;
    cvDacA.data[2] = layerA.test.aFreq.valueMapped * 4095;
    cvDacA.data[3] = (1 - layerA.test.aDistort.valueMapped) * 4095;
}

void renderCVs() {
    // render everything that is local to modules
    for (BaseModule *m : layerA.getModules()) {
        m->render();
    }

    // TODO need to render global stuff like Filter Out Levels here

    updateAllOutputSamples();

    // TODO copy all output.currentSamples to their DAC data storage here

    // TODO remove this later
    tempRendering();

    cvDacA.setLatchPin();
    cvDacB.setLatchPin();
    cvDacC.setLatchPin();

    cvDacA.fastUpdate();

    FlagHandler::cvDacAStarted = true;
    // cvDacB.fastUpdate();
    // cvDacC.fastUpdate();
}
