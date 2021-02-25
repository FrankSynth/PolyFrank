#ifdef POLYRENDER

#include "renderCV.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "renderADSR.hpp"
#include "renderDistort.hpp"
#include "renderGlobal.hpp"
#include "renderLFO.hpp"
#include "renderLadder.hpp"
#include "renderNoise.hpp"
#include "renderOSC.hpp"
#include "renderSteiner.hpp"
#include "renderSub.hpp"

extern MCP4728 cvDacA;
extern MCP4728 cvDacB;
extern MCP4728 cvDacC;
extern MCP4728 cvDacAx;
extern MCP4728 cvDacBx;
extern MCP4728 cvDacCx;

extern Layer layerA;

// probably obsolete
void initCVRendering() {
    cvDacA.data.currentSample[0] = 4095; // ladder resonance
    cvDacA.data.currentSample[1] = 4095; // distort
    cvDacA.data.currentSample[2] = 4095; // ladder cutoff
    cvDacA.data.currentSample[3] = 4095; // master vol right

    cvDacB.data.currentSample[0] = 0;    // n/a
    cvDacB.data.currentSample[1] = 4095; // master vol left
    cvDacB.data.currentSample[2] = 0;    // steiner cutoff
    cvDacB.data.currentSample[3] = 4095; // ladder level

    cvDacC.data.currentSample[0] = 4095; // steiner level
    cvDacC.data.currentSample[1] = 4095; // steiner resonance
    cvDacC.data.currentSample[2] = 4095; // steiner out to ladder in
    cvDacC.data.currentSample[3] = 0;

    cvDacA.sendCurrentBuffer();
    cvDacB.sendCurrentBuffer();
    cvDacC.sendCurrentBuffer();

    cvDacAx.data.currentSample[0] = 4095; // ladder resonance
    cvDacAx.data.currentSample[1] = 4095; // distort
    cvDacAx.data.currentSample[2] = 4095; // ladder cutoff
    cvDacAx.data.currentSample[3] = 4095; // master vol right

    cvDacBx.data.currentSample[0] = 0;    // n/a
    cvDacBx.data.currentSample[1] = 4095; // master vol left
    cvDacBx.data.currentSample[2] = 0;    // steiner cutoff
    cvDacBx.data.currentSample[3] = 4095; // ladder level

    cvDacCx.data.currentSample[0] = 4095; // steiner level
    cvDacCx.data.currentSample[1] = 4095; // steiner resonance
    cvDacCx.data.currentSample[2] = 4095; // steiner out to ladder in
    cvDacCx.data.currentSample[3] = 0;

    cvDacAx.sendCurrentBuffer();
    cvDacBx.sendCurrentBuffer();
    cvDacCx.sendCurrentBuffer();
}

inline void collectAllCurrentInputs() {
    for (BaseModule *m : layerA.modules) {
        for (Input *i : m->inputs) {
            i->collectCurrentSample();
        }
    }
}

inline void updateAllOutputSamples() {

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

inline void renderMidiModule(Midi midi) {
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oMod.nextSample[voice] = midi.aMod.valueMapped;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oAftertouch.nextSample[voice] = midi.aAftertouch.valueMapped;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oPitchbend.nextSample[voice] = midi.aPitchbend.valueMapped;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oNote.nextSample[voice] = (float)midi.rawNote[voice] / 127.0f;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oVeloctiy.nextSample[voice] = (float)midi.rawVelocity[voice] / 127.0f;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        midi.oGate.nextSample[voice] = midi.rawGate[voice];
}

inline void writeDataToDACBuffer() {

    // TODO output assigment

    cvDacA.data.nextSample[0] = (1.0f - layerA.ladder.resonance.currentSample[0]) * 4095.0f;   // ladder resonance
    cvDacA.data.nextSample[1] = (1.0f - layerA.distort.distort.currentSample[0]) * 4095.0f;    // distort
    cvDacA.data.nextSample[2] = (1.0f - layerA.ladder.cutoff.currentSample[0]) * 4095.0f;      // ladder cutoff
    cvDacA.data.nextSample[3] = (1.0f - layerA.globalModule.right.currentSample[0]) * 4095.0f; // master vol right

    cvDacB.data.nextSample[0] = 0;                                                            // n/a
    cvDacB.data.nextSample[1] = (1.0f - layerA.globalModule.left.currentSample[0]) * 4095.0f; // master vol left
    cvDacB.data.nextSample[2] = layerA.steiner.cutoff.currentSample[0] * 4095.0f;             // steiner cutoff
    cvDacB.data.nextSample[3] = (1.0f - layerA.ladder.level.currentSample[0]) * 4095.0f;      // ladder level

    cvDacC.data.nextSample[0] = (1.0f - layerA.steiner.level.currentSample[0]) * 4095.0f;     // steiner level
    cvDacC.data.nextSample[1] = (1.0f - layerA.steiner.resonance.currentSample[0]) * 4095.0f; // steiner resonance
    cvDacC.data.nextSample[2] = (1.0f - layerA.steiner.toLadder.currentSample[0]) * 4095.0f; // steiner out to ladder in
    cvDacC.data.nextSample[3] = (layerA.lfoA.out.currentSample[0] * 0.5f + 0.5f) * 4095.0f;  // n/a

    cvDacAx.data.nextSample[0] = (1.0f - layerA.ladder.resonance.currentSample[0]) * 4095.0f;   // ladder resonance
    cvDacAx.data.nextSample[1] = (1.0f - layerA.distort.distort.currentSample[0]) * 4095.0f;    // distort
    cvDacAx.data.nextSample[2] = (1.0f - layerA.ladder.cutoff.currentSample[0]) * 4095.0f;      // ladder cutoff
    cvDacAx.data.nextSample[3] = (1.0f - layerA.globalModule.right.currentSample[0]) * 4095.0f; // master vol right

    cvDacBx.data.nextSample[0] = 0;                                                            // n/a
    cvDacBx.data.nextSample[1] = (1.0f - layerA.globalModule.left.currentSample[0]) * 4095.0f; // master vol left
    cvDacBx.data.nextSample[2] = layerA.steiner.cutoff.currentSample[0] * 4095.0f;             // steiner cutoff
    cvDacBx.data.nextSample[3] = (1.0f - layerA.ladder.level.currentSample[0]) * 4095.0f;      // ladder level

    cvDacCx.data.nextSample[0] = (1.0f - layerA.steiner.level.currentSample[0]) * 4095.0f;     // steiner level
    cvDacCx.data.nextSample[1] = (1.0f - layerA.steiner.resonance.currentSample[0]) * 4095.0f; // steiner resonance
    cvDacCx.data.nextSample[2] =
        (1.0f - layerA.steiner.toLadder.currentSample[0]) * 4095.0f;                         // steiner out to ladder in
    cvDacCx.data.nextSample[3] = (layerA.lfoA.out.currentSample[0] * 0.5f + 0.5f) * 4095.0f; // n/a
}

inline void setSwitches() {

    const int32_t &steinerMode = layerA.steiner.dMode.valueMapped;
    const int32_t &ladderMode = layerA.ladder.dSlope.valueMapped;

    switch (steinerMode) {
        case 0:
            HAL_GPIO_WritePin(switch_1_A_GPIO_Port, switch_1_A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(switch_1_B_GPIO_Port, switch_1_B_Pin, GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(switch_1_A_GPIO_Port, switch_1_A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(switch_1_B_GPIO_Port, switch_1_B_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(switch_1_A_GPIO_Port, switch_1_A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(switch_1_B_GPIO_Port, switch_1_B_Pin, GPIO_PIN_SET);
            break;
        case 3:
            HAL_GPIO_WritePin(switch_1_A_GPIO_Port, switch_1_A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(switch_1_B_GPIO_Port, switch_1_B_Pin, GPIO_PIN_RESET);
            break;

        default: PolyError_Handler("renderCV | setSwitches | wrong steinerMode"); break;
    }

    switch (ladderMode) {
        case 0:
            HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_RESET);
            break;
        case 3:
            HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_RESET);
            break;

        default: PolyError_Handler("renderCV | setSwitches | wrong ladderMode"); break;
    }
}

LogCurve ledLog(16, 0.1);

inline void setLEDs() {
    // max brightness 1023

    // TODO LED assigment
    // TODO LED Brightness setting?

    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, ledLog.mapValue(layerA.adsrA.out.currentSample[0]) * 1023.0f);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, ledLog.mapValue(layerA.adsrB.out.currentSample[0]) * 1023.0f);
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3,
                         ledLog.mapValue(layerA.lfoA.out.currentSample[0] * 0.5f + 0.5f) * 1023.0f);
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4,
                         ledLog.mapValue(layerA.lfoB.out.currentSample[0] * 0.5f + 0.5f) * 1023.0f);
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, ledLog.mapValue(layerA.adsrA.out.currentSample[0]) * 1023.0f);
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, ledLog.mapValue(layerA.adsrB.out.currentSample[0]) * 1023.0f);
}

// elapsedMicros rendertimecv;
void renderCVs() {
    // cvDacA.setLatchPin();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PINo_SET);

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

    setSwitches();
    setLEDs();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

    // cvDacA.resetLatchPin();
}

#endif
