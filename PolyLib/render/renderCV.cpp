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

extern MCP4728 cvDac[10];

extern Layer layerA;

// probably obsolete
void initCVRendering() {
    cvDac[0].data.currentSample[0] = 4095; // ladder resonance
    cvDac[0].data.currentSample[1] = 4095; // distort
    cvDac[0].data.currentSample[2] = 4095; // ladder cutoff
    cvDac[0].data.currentSample[3] = 4095; // master vol right

    // cvDac[1].data.currentSample[0] = 0;    // n/a
    // cvDac[1].data.currentSample[1] = 4095; // master vol left
    // cvDac[1].data.currentSample[2] = 0;    // steiner cutoff
    // cvDac[1].data.currentSample[3] = 4095; // ladder level

    // cvDac[2].data.currentSample[0] = 4095; // steiner level
    // cvDac[2].data.currentSample[1] = 4095; // steiner resonance
    // cvDac[2].data.currentSample[2] = 4095; // steiner out to ladder in
    // cvDac[2].data.currentSample[3] = 0;

    // cvDacD.data.currentSample[0] = 4095; // steiner level
    // cvDacD.data.currentSample[1] = 4095; // steiner resonance
    // cvDacD.data.currentSample[2] = 4095; // steiner out to ladder in
    // cvDacD.data.currentSample[3] = 0;

    cvDac[0].sendCurrentBuffer();
    // cvDacB.sendCurrentBuffer();
    // cvDacC.sendCurrentBuffer();
    // cvDacD.sendCurrentBuffer();
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

    cvDac[0].data.nextSample[0] = (1.0f - layerA.ladder.resonance.currentSample[0]) * 4095.0f;   // ladder resonance
    cvDac[0].data.nextSample[1] = (1.0f - layerA.distort.distort.currentSample[0]) * 4095.0f;    // distort
    cvDac[0].data.nextSample[2] = (1.0f - layerA.ladder.cutoff.currentSample[0]) * 4095.0f;      // ladder cutoff
    cvDac[0].data.nextSample[3] = (1.0f - layerA.globalModule.right.currentSample[0]) * 4095.0f; // master vol right

    // cvDacB.data.nextSample[0] = 0;                                                            // n/a
    // cvDacB.data.nextSample[1] = (1.0f - layerA.globalModule.left.currentSample[0]) * 4095.0f; // master vol left
    // cvDacB.data.nextSample[2] = layerA.steiner.cutoff.currentSample[0] * 4095.0f;             // steiner cutoff
    // cvDacB.data.nextSample[3] = (1.0f - layerA.ladder.level.currentSample[0]) * 4095.0f;      // ladder level

    // cvDacC.data.nextSample[0] = (1.0f - layerA.steiner.level.currentSample[0]) * 4095.0f;     // steiner level
    // cvDacC.data.nextSample[1] = (1.0f - layerA.steiner.resonance.currentSample[0]) * 4095.0f; // steiner resonance
    // cvDacC.data.nextSample[2] = (1.0f - layerA.steiner.toLadder.currentSample[0]) * 4095.0f; // steiner out to ladder
    // in cvDacC.data.nextSample[3] = (layerA.lfoA.out.currentSample[0] * 0.5f + 0.5f) * 4095.0f;  // n/a

    // cvDacD.data.nextSample[0] = (1.0f - layerA.steiner.level.currentSample[0]) * 4095.0f;     // steiner level
    // cvDacD.data.nextSample[1] = (1.0f - layerA.steiner.resonance.currentSample[0]) * 4095.0f; // steiner resonance
    // cvDacD.data.nextSample[2] = (1.0f - layerA.steiner.toLadder.currentSample[0]) * 4095.0f; // steiner out to ladder
    // in cvDacD.data.nextSample[3] = (layerA.lfoA.out.currentSample[0] * 0.5f + 0.5f) * 4095.0f;  // n/a
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
