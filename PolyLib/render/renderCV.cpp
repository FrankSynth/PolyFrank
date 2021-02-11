#ifdef POLYRENDER

#include "renderCV.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "renderADSR.hpp"
#include "renderDistort.hpp"
#include "renderLFO.hpp"
#include "renderLadder.hpp"
#include "renderNoise.hpp"
#include "renderOSC.hpp"
#include "renderSteiner.hpp"
#include "renderSub.hpp"

extern MCP4728 cvDacA;
extern MCP4728 cvDacB;
extern MCP4728 cvDacC;

extern Layer layerA;

// probably obsolete
// void initCVRendering() {
//     //
// }

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
        for (RenderBuffer *b : m->renderBuffer) {
            b->updateToNextSample();
        }
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

void renderGlobalModule(GlobalModule globalModule) {

    // TODO render global

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
    }
}

void writeDataToDACBuffer() {

    // TODO output assigment

    cvDacA.data[0] = layerA.ladder.resonance.currentSample[0] * 4095;
    cvDacA.data[1] = layerA.distort.distort.currentSample[0] * 4095;
    cvDacA.data[2] = layerA.ladder.cutoff.currentSample[0] * 4095;
    cvDacA.data[3] = layerA.globalModule.right.currentSample[0] * 4095;

    cvDacB.data[0] = 0;
    cvDacB.data[1] = layerA.globalModule.left.currentSample[0] * 4095;
    cvDacB.data[2] = layerA.steiner.cutoff.currentSample[0] * 4095;
    cvDacB.data[3] = layerA.ladder.level.currentSample[0] * 4095;

    cvDacC.data[0] = layerA.steiner.level.currentSample[0] * 4095;
    cvDacC.data[1] = layerA.steiner.resonance.currentSample[0] * 4095;
    cvDacC.data[2] = layerA.steiner.toLadder.currentSample[0] * 4095;
    cvDacC.data[3] = 0;
}

void setLEDs() {
    // max brightness 1023

    // TODO LED assigment

    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, fastMapLEDBrightness(layerA.adsrA.out.currentSample[0]) * 1023);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, fastMapLEDBrightness(layerA.adsrB.out.currentSample[0]) * 1023);
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, fastMapLEDBrightness(layerA.lfoA.out.currentSample[0] + 1) * 511);
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, fastMapLEDBrightness(layerA.lfoB.out.currentSample[0] + 1) * 511);
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, fastMapLEDBrightness(0));
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, fastMapLEDBrightness(0));
}

// elapsedMicros rendertimecv;
void renderCVs() {

    // rendertimecv = 0;

    collectAllCurrentInputs();

    renderMidiModule(layerA.midi);
    renderOSC_A(layerA.oscA);
    renderOSC_B(layerA.oscB);
    renderSub(layerA.sub);
    renderNoise(layerA.noise);
    renderSteiner(layerA.steiner);
    renderLadder(layerA.ladder);
    renderDistort(layerA.distort);
    renderLFO(layerA.lfoA);
    renderLFO(layerA.lfoB);
    renderADSR(layerA.adsrA);
    renderADSR(layerA.adsrB);
    renderGlobalModule(layerA.globalModule);

    updateAllOutputSamples();

    writeDataToDACBuffer();

    // uint32_t time = rendertimecv;
    // println(time);

    cvDacA.setLatchPin();
    cvDacB.setLatchPin();
    cvDacC.setLatchPin();

    // out DacB and DacC gets automatially triggered by flags when transmission is done
    cvDacA.fastUpdate();
    FlagHandler::cvDacAStarted = true;

    setLEDs();
}

#endif
