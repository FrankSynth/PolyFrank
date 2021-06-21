#ifdef POLYRENDER

#include "renderAudioDef.h"

#include "datacore/dataHelperFunctions.hpp"
#include "renderADSR.hpp"
#include "renderCV.hpp"
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

/*
List of assignement

    00 cvDac[0].data.currentSample[0] -- VOICE 1 | LADDER CUTOFF
    01 cvDac[0].data.currentSample[1] -- VOICE 0 | LADDER CUTOFF
    02 cvDac[0].data.currentSample[2] -- VOICE 0 | STEINER CUTOFF
    03 cvDac[0].data.currentSample[3] -- VOICE 1 | STEINER CUTOFF

    04 cvDac[1].data.currentSample[0] -- VOICE 1 | LADDER RES
    05 cvDac[1].data.currentSample[1] -- VOICE 0 | OUT LEVEL R
    06 cvDac[1].data.currentSample[2] -- VOICE 0 | OUT LEVEL L
    07 cvDac[1].data.currentSample[3] -- VOICE 1 | DISTORT LEVEL

    08 cvDac[2].data.currentSample[0] -- VOICE 1 | OUT LEVEL R
    09 cvDac[2].data.currentSample[1] -- VOICE 0 | LADDER RES
    10 cvDac[2].data.currentSample[2] -- VOICE 0 | DISTORT LEVEL
    11 cvDac[2].data.currentSample[3] -- VOICE 1 | OUT LEVEL L

    12 cvDac[3].data.currentSample[0] -- VOICE 1 | STEINER RES
    13 cvDac[3].data.currentSample[1] -- VOICE 0 | STEINER RES
    14 cvDac[3].data.currentSample[2] -- VOICE 0 | STEINER TO LADDER LEVEL
    15 cvDac[3].data.currentSample[3] -- VOICE 1 | STEINER TO LADDER LEVEL

    16 cvDac[4].data.currentSample[0] -- VOICE 1 | LADDER OUT LEVEL
    17 cvDac[4].data.currentSample[1] -- VOICE 0 | LADDER OUT LEVEL
    18 cvDac[4].data.currentSample[2] -- VOICE 0 | STEINER OUT LEVEL
    19 cvDac[4].data.currentSample[3] -- VOICE 1 | STEINER OUT LEVEL

    20 cvDac[5].data.currentSample[0] -- VOICE 3 | LADDER CUTOFF
    21 cvDac[5].data.currentSample[1] -- VOICE 2 | LADDER CUTOFF
    22 cvDac[5].data.currentSample[2] -- VOICE 2 | STEINER CUTOFF
    23 cvDac[5].data.currentSample[3] -- VOICE 3 | STEINER CUTOFF

    24 cvDac[6].data.currentSample[0] -- VOICE 3 | LADDER RES
    25 cvDac[6].data.currentSample[1] -- VOICE 2 | OUT LEVEL R
    26 cvDac[6].data.currentSample[2] -- VOICE 2 | OUT LEVEL L
    27 cvDac[6].data.currentSample[3] -- VOICE 3 | DISTORT LEVEL

    28 cvDac[7].data.currentSample[0] -- VOICE 3 | OUT LEVEL R
    29 cvDac[7].data.currentSample[1] -- VOICE 2 | LADDER RES
    30 cvDac[7].data.currentSample[2] -- VOICE 2 | DISTORT LEVEL
    31 cvDac[7].data.currentSample[3] -- VOICE 3 | OUT LEVEL L

    32 cvDac[8].data.currentSample[0] -- VOICE 3 | STEINER RES
    33 cvDac[8].data.currentSample[1] -- VOICE 2 | STEINER RES
    34 cvDac[8].data.currentSample[2] -- VOICE 2 | STEINER TO LADDER LEVEL
    35 cvDac[8].data.currentSample[3] -- VOICE 3 | STEINER TO LADDER LEVEL

    36 cvDac[9].data.currentSample[0] -- VOICE 3 | LADDER OUT LEVEL
    37 cvDac[9].data.currentSample[1] -- VOICE 2 | LADDER OUT LEVEL
    38 cvDac[9].data.currentSample[2] -- VOICE 2 | STEINER OUT LEVEL
    39 cvDac[9].data.currentSample[3] -- VOICE 3 | STEINER OUT LEVEL

*/

void initCVRendering() {

    cvDac[0].data.currentSample[0] = 4095; // VOICE 1 | LADDER CUTOFF
    cvDac[0].data.currentSample[1] = 4095; // VOICE 0 | LADDER CUTOFF
    cvDac[0].data.currentSample[2] = 0;    // VOICE 0 | STEINER CUTOFF
    cvDac[0].data.currentSample[3] = 0;    // VOICE 1 | STEINER CUTOFF

    cvDac[1].data.currentSample[0] = 4095; // VOICE 1 | LADDER RES
    cvDac[1].data.currentSample[1] = 4095; // VOICE 0 | OUT LEVEL R
    cvDac[1].data.currentSample[2] = 4095; // VOICE 0 | OUT LEVEL L
    cvDac[1].data.currentSample[3] = 4095; // VOICE 1 | DISTORT LEVEL

    cvDac[2].data.currentSample[0] = 4095; // VOICE 1 | OUT LEVEL R
    cvDac[2].data.currentSample[1] = 4095; // VOICE 0 | LADDER RES
    cvDac[2].data.currentSample[2] = 4095; // VOICE 0 | DISTORT LEVEL
    cvDac[2].data.currentSample[3] = 4095; // VOICE 1 | OUT LEVEL L

    cvDac[3].data.currentSample[0] = 4095; // VOICE 1 | STEINER RES
    cvDac[3].data.currentSample[1] = 4095; // VOICE 0 | STEINER RES
    cvDac[3].data.currentSample[2] = 4095; // VOICE 0 | STEINER TO LADDER LEVEL
    cvDac[3].data.currentSample[3] = 4095; // VOICE 1 | STEINER TO LADDER LEVEL

    cvDac[4].data.currentSample[0] = 4095; // VOICE 1 | LADDER OUT LEVEL
    cvDac[4].data.currentSample[1] = 4095; // VOICE 0 | LADDER OUT LEVEL
    cvDac[4].data.currentSample[2] = 4095; // VOICE 0 | STEINER OUT LEVEL
    cvDac[4].data.currentSample[3] = 4095; // VOICE 1 | STEINER OUT LEVEL

    cvDac[5].data.currentSample[0] = 4095; // VOICE 3 | LADDER CUTOFF
    cvDac[5].data.currentSample[1] = 4095; // VOICE 2 | LADDER CUTOFF
    cvDac[5].data.currentSample[2] = 0;    // VOICE 2 | STEINER CUTOFF
    cvDac[5].data.currentSample[3] = 0;    // VOICE 3 | STEINER CUTOFF

    cvDac[6].data.currentSample[0] = 4095; // VOICE 3 | LADDER RES
    cvDac[6].data.currentSample[1] = 4095; // VOICE 2 | OUT LEVEL R
    cvDac[6].data.currentSample[2] = 4095; // VOICE 2 | OUT LEVEL L
    cvDac[6].data.currentSample[3] = 4095; // VOICE 3 | DISTORT LEVEL

    cvDac[7].data.currentSample[0] = 4095; // VOICE 3 | OUT LEVEL R
    cvDac[7].data.currentSample[1] = 4095; // VOICE 2 | LADDER RES
    cvDac[7].data.currentSample[2] = 4095; // VOICE 2 | DISTORT LEVEL
    cvDac[7].data.currentSample[3] = 4095; // VOICE 3 | OUT LEVEL L

    cvDac[8].data.currentSample[0] = 4095; // VOICE 3 | STEINER RES
    cvDac[8].data.currentSample[1] = 4095; // VOICE 2 | STEINER RES
    cvDac[8].data.currentSample[2] = 4095; // VOICE 2 | STEINER TO LADDER LEVEL
    cvDac[8].data.currentSample[3] = 4095; // VOICE 3 | STEINER TO LADDER LEVEL

    cvDac[9].data.currentSample[0] = 4095; // VOICE 3 | LADDER OUT LEVEL
    cvDac[9].data.currentSample[1] = 4095; // VOICE 2 | LADDER OUT LEVEL
    cvDac[9].data.currentSample[2] = 4095; // VOICE 2 | STEINER OUT LEVEL
    cvDac[9].data.currentSample[3] = 4095; // VOICE 3 | STEINER OUT LEVEL

    for (uint16_t i; i < ALLDACS; i++)
        cvDac[0].sendCurrentBuffer();
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

    cvDac[0].data.currentSample[0] =
        (1.0f - layerA.ladder.cutoff.currentSample[1]) * 4095.0f; // VOICE 1 | LADDER CUTOFF
    cvDac[0].data.currentSample[1] =
        (1.0f - layerA.ladder.cutoff.currentSample[0]) * 4095.0f;                      // VOICE 0 | LADDER CUTOFF
    cvDac[0].data.currentSample[2] = layerA.steiner.cutoff.currentSample[0] * 4095.0f; // VOICE 0 | STEINER CUTOFF
    cvDac[0].data.currentSample[3] = layerA.steiner.cutoff.currentSample[1] * 4095.0f; // VOICE 1 | STEINER CUTOFF

    cvDac[1].data.currentSample[0] =
        (1.0f - layerA.ladder.resonance.currentSample[1]) * 4095.0f; // VOICE 1 | LADDER RES
    cvDac[1].data.currentSample[1] =
        (1.0f - layerA.globalModule.right.currentSample[0]) * 4095.0f; // VOICE 0 | OUT LEVEL R
    cvDac[1].data.currentSample[2] =
        (1.0f - layerA.globalModule.left.currentSample[0]) * 4095.0f; // VOICE 0 | OUT LEVEL L
    cvDac[1].data.currentSample[3] =
        (1.0f - layerA.distort.distort.currentSample[1]) * 4095.0f; // VOICE 1 | DISTORT LEVEL

    cvDac[2].data.currentSample[0] =
        (1.0f - layerA.globalModule.right.currentSample[1]) * 4095.0f; // VOICE 1 | OUT LEVEL R
    cvDac[2].data.currentSample[1] =
        (1.0f - layerA.ladder.resonance.currentSample[0]) * 4095.0f; // VOICE 0 | LADDER RES
    cvDac[2].data.currentSample[2] =
        (1.0f - layerA.distort.distort.currentSample[0]) * 4095.0f; // VOICE 0 | DISTORT LEVEL
    cvDac[2].data.currentSample[3] =
        (1.0f - layerA.globalModule.left.currentSample[1]) * 4095.0f; // VOICE 1 | OUT LEVEL L

    cvDac[3].data.currentSample[0] =
        (1.0f - layerA.steiner.resonance.currentSample[1]) * 4095.0f; // VOICE 1 | STEINER RES
    cvDac[3].data.currentSample[1] =
        (1.0f - layerA.steiner.resonance.currentSample[0]) * 4095.0f; // VOICE 0 | STEINER RES
    cvDac[3].data.currentSample[2] =
        (1.0f - layerA.steiner.toLadder.currentSample[0]) * 4095.0f; // VOICE 0 | STEINER TO LADDER LEVEL
    cvDac[3].data.currentSample[3] =
        (1.0f - layerA.steiner.toLadder.currentSample[1]) * 4095.0f; // VOICE 1 | STEINER TO LADDER LEVEL

    cvDac[4].data.currentSample[0] =
        (1.0f - layerA.ladder.level.currentSample[1]) * 4095.0f; // VOICE 1 | LADDER OUT LEVEL
    cvDac[4].data.currentSample[1] =
        (1.0f - layerA.ladder.level.currentSample[0]) * 4095.0f; // VOICE 0 | LADDER OUT LEVEL
    cvDac[4].data.currentSample[2] =
        (1.0f - layerA.steiner.level.currentSample[0]) * 4095.0f; // VOICE 0 | STEINER OUT LEVEL
    cvDac[4].data.currentSample[3] =
        (1.0f - layerA.steiner.level.currentSample[1]) * 4095.0f; // VOICE 1 | STEINER OUT LEVEL

    cvDac[5].data.currentSample[0] =
        (1.0f - layerA.ladder.cutoff.currentSample[3]) * 4095.0f; // VOICE 3 | LADDER CUTOFF
    cvDac[5].data.currentSample[1] =
        (1.0f - layerA.ladder.cutoff.currentSample[2]) * 4095.0f;                      // VOICE 2 | LADDER CUTOFF
    cvDac[5].data.currentSample[2] = layerA.steiner.cutoff.currentSample[2] * 4095.0f; // VOICE 2 | STEINER CUTOFF
    cvDac[5].data.currentSample[3] = layerA.steiner.cutoff.currentSample[3] * 4095.0f; // VOICE 3 | STEINER CUTOFF

    cvDac[6].data.currentSample[0] =
        (1.0f - layerA.ladder.resonance.currentSample[3]) * 4095.0f; // VOICE 3 | LADDER RES
    cvDac[6].data.currentSample[1] =
        (1.0f - layerA.globalModule.right.currentSample[2]) * 4095.0f; // VOICE 2 | OUT LEVEL R
    cvDac[6].data.currentSample[2] =
        (1.0f - layerA.globalModule.left.currentSample[2]) * 4095.0f; // VOICE 2 | OUT LEVEL L
    cvDac[6].data.currentSample[3] =
        (1.0f - layerA.distort.distort.currentSample[3]) * 4095.0f; // VOICE 3 | DISTORT LEVEL

    cvDac[7].data.currentSample[0] =
        (1.0f - layerA.globalModule.right.currentSample[3]) * 4095.0f; // VOICE 3 | OUT LEVEL R
    cvDac[7].data.currentSample[1] =
        (1.0f - layerA.ladder.resonance.currentSample[2]) * 4095.0f; // VOICE 2 | LADDER RES
    cvDac[7].data.currentSample[2] =
        (1.0f - layerA.distort.distort.currentSample[2]) * 4095.0f; // VOICE 2 | DISTORT LEVEL
    cvDac[7].data.currentSample[3] =
        (1.0f - layerA.globalModule.left.currentSample[3]) * 4095.0f; // VOICE 3 | OUT LEVEL L

    cvDac[8].data.currentSample[0] =
        (1.0f - layerA.steiner.resonance.currentSample[3]) * 4095.0f; // VOICE 3 | STEINER RES
    cvDac[8].data.currentSample[1] =
        (1.0f - layerA.steiner.resonance.currentSample[2]) * 4095.0f; // VOICE 2 | STEINER RES
    cvDac[8].data.currentSample[2] =
        (1.0f - layerA.steiner.toLadder.currentSample[2]) * 4095.0f; // VOICE 2 | STEINER TO LADDER LEVEL
    cvDac[8].data.currentSample[3] =
        (1.0f - layerA.steiner.toLadder.currentSample[3]) * 4095.0f; // VOICE 3 | STEINER TO LADDER LEVEL

    cvDac[9].data.currentSample[0] =
        (1.0f - layerA.ladder.level.currentSample[3]) * 4095.0f; // VOICE 3 | LADDER OUT LEVEL
    cvDac[9].data.currentSample[1] =
        (1.0f - layerA.ladder.level.currentSample[2]) * 4095.0f; // VOICE 2 | LADDER OUT LEVEL
    cvDac[9].data.currentSample[2] =
        (1.0f - layerA.steiner.level.currentSample[2]) * 4095.0f; // VOICE 2 | STEINER OUT LEVEL
    cvDac[9].data.currentSample[3] =
        (1.0f - layerA.steiner.level.currentSample[3]) * 4095.0f; // VOICE 3 | STEINER OUT LEVEL
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

    __HAL_TIM_SetCompare(
        &htim3, TIM_CHANNEL_1,
        ledLog.mapValue((layerA.globalModule.left.currentSample[2] + layerA.globalModule.right.currentSample[2]) *
                        0.5f) *
            1023.0f); // 2
    __HAL_TIM_SetCompare(
        &htim3, TIM_CHANNEL_2,
        ledLog.mapValue((layerA.globalModule.left.currentSample[3] + layerA.globalModule.right.currentSample[3]) *
                        0.5f) *
            1023.0f); // 3
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_1,
                         ledLog.mapValue(layerA.lfoA.out.currentSample[0] * 0.5f + 0.5f) * 1023.0f); // 5
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_2,
                         ledLog.mapValue(layerA.lfoB.out.currentSample[0] * 0.5f + 0.5f) * 1023.0f); // 6
    __HAL_TIM_SetCompare(
        &htim4, TIM_CHANNEL_1,
        ledLog.mapValue((layerA.globalModule.left.currentSample[1] + layerA.globalModule.right.currentSample[1]) *
                        0.5f) *
            1023.0f); // 1
    __HAL_TIM_SetCompare(
        &htim4, TIM_CHANNEL_2,
        ledLog.mapValue((layerA.globalModule.left.currentSample[0] + layerA.globalModule.right.currentSample[0]) *
                        0.5f) *
            1023.0f); // 0
    // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, ledLog.mapValue(layerA.adsrB.out.currentSample[2]) * 1023.0f);
    // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_4, ledLog.mapValue(layerA.adsrB.out.currentSample[3]) * 1023.0f);
}

// elapsedMicros rendertimecv;
void renderCVs() {
    // cvDacA.setLatchPin();

    // LED TIMING TEST
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
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
