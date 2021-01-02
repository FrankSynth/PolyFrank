#include "renderCV.hpp"

extern MCP4728 cvDacA;
extern MCP4728 cvDacB;
extern MCP4728 cvDacC;

extern Layer layerA;

void renderCVs() {
    // microTimer = micros();

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

    cvDacA.setLatchPin();
    cvDacB.setLatchPin();
    cvDacC.setLatchPin();

    cvDacA.fastUpdate();

    FlagHandler::cvDacAStarted = true;
    // cvDacB.fastUpdate();
    // cvDacC.fastUpdate();
}