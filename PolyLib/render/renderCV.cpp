#ifdef POLYRENDER

#include "renderAudioDef.h"

#include "datacore/dataHelperFunctions.hpp"
#include "renderADSR.hpp"
#include "renderCV.hpp"
#include "renderFeel.hpp"
#include "renderLFO.hpp"
#include "renderLadder.hpp"
#include "renderMidi.hpp"
#include "renderMixer.hpp"
#include "renderNoise.hpp"
#include "renderOSC.hpp"
#include "renderOutput.hpp"
#include "renderPhaseshaper.hpp"
#include "renderSteiner.hpp"
#include "renderSub.hpp"
#include "renderWaveshaper.hpp"

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

    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_SET);
    cvDac[7].sendCurrentBuffer();
    cvDac[4].sendCurrentBuffer();
    cvDac[0].sendCurrentBuffer();
    HAL_GPIO_WritePin(LDAC_1_GPIO_Port, LDAC_1_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_SET);
    cvDac[1].sendCurrentBuffer();
    cvDac[5].sendCurrentBuffer();
    cvDac[8].sendCurrentBuffer();
    HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_SET);
    cvDac[2].sendCurrentBuffer();
    cvDac[6].sendCurrentBuffer();
    cvDac[9].sendCurrentBuffer();
    HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LDAC_4_GPIO_Port, LDAC_4_Pin, GPIO_PIN_SET);
    cvDac[3].sendCurrentBuffer();
    HAL_GPIO_WritePin(LDAC_4_GPIO_Port, LDAC_4_Pin, GPIO_PIN_RESET);

    // set LEDs off
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0); // 2
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0); // 3
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_1, 0); // 5
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_2, 0); // 6
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, 0); // 1
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, 0); // 0
}

inline void collectAllCurrentInputs() {
    for (BaseModule *m : layerA.modules) {
        for (Input *i : m->inputs) {
            i->collectCurrentSample();
        }
    }
}

// inline void updateAllOutputSamples() {

//     // LayerRenBufferSw = !LayerRenBufferSw;

//     for (BaseModule *m : layerA.modules) {

//         // __disable_irq();
//         // for (Output *o : m->outputs) {
//         //     o->updateToNextSample();
//         // }
//         // __enable_irq();

//         // __disable_irq();
//         // for (RenderBuffer *b : m->renderBuffer) {
//         //     b->updateToNextSample();
//         // }
//         // __enable_irq();
//     }
// }

inline void writeDataToDACBuffer() {

    cvDac[0].data.currentSample[0] = (1.0f - layerA.ladder.cutoff[1]) * 4095.0f; // VOICE 1 | LADDER CUTOFF
    cvDac[0].data.currentSample[1] = (1.0f - layerA.ladder.cutoff[0]) * 4095.0f; // VOICE 0 | LADDER CUTOFF
    cvDac[0].data.currentSample[2] = layerA.steiner.cutoff[0] * 4095.0f;         // VOICE 0 | STEINER CUTOFF
    cvDac[0].data.currentSample[3] = layerA.steiner.cutoff[1] * 4095.0f;         // VOICE 1 | STEINER CUTOFF

    cvDac[1].data.currentSample[0] = (1.0f - layerA.ladder.resonance[1]) * 4095.0f; // VOICE 1 | LADDER RES
    cvDac[1].data.currentSample[1] = (1.0f - layerA.out.right[0]) * 4095.0f;        // VOICE 0 | OUT LEVEL R
    cvDac[1].data.currentSample[2] = (1.0f - layerA.out.left[0]) * 4095.0f;         // VOICE 0 | OUT LEVEL L
    cvDac[1].data.currentSample[3] = (1.0f - layerA.out.distort[1]) * 4095.0f;      // VOICE 1 | DISTORT LEVEL

    cvDac[2].data.currentSample[0] = (1.0f - layerA.out.right[1]) * 4095.0f;        // VOICE 1 | OUT LEVEL R
    cvDac[2].data.currentSample[1] = (1.0f - layerA.ladder.resonance[0]) * 4095.0f; // VOICE 0 | LADDER RES
    cvDac[2].data.currentSample[2] = (1.0f - layerA.out.distort[0]) * 4095.0f;      // VOICE 0 | DISTORT LEVEL
    cvDac[2].data.currentSample[3] = (1.0f - layerA.out.left[1]) * 4095.0f;         // VOICE 1 | OUT LEVEL L

    cvDac[3].data.currentSample[0] = (1.0f - layerA.steiner.resonance[1]) * 4095.0f; // VOICE 1 | STEINER RES
    cvDac[3].data.currentSample[1] = (1.0f - layerA.steiner.resonance[0]) * 4095.0f; // VOICE 0 | STEINER RES
    cvDac[3].data.currentSample[2] = (1.0f - layerA.steiner.toLadder[0]) * 4095.0f; // VOICE 0 | STEINER TO LADDER LEVEL
    cvDac[3].data.currentSample[3] = (1.0f - layerA.steiner.toLadder[1]) * 4095.0f; // VOICE 1 | STEINER TO LADDER LEVEL

    cvDac[4].data.currentSample[0] = (1.0f - layerA.ladder.level[1]) * 4095.0f;  // VOICE 1 | LADDER OUT LEVEL
    cvDac[4].data.currentSample[1] = (1.0f - layerA.ladder.level[0]) * 4095.0f;  // VOICE 0 | LADDER OUT LEVEL
    cvDac[4].data.currentSample[2] = (1.0f - layerA.steiner.level[0]) * 4095.0f; // VOICE 0 | STEINER OUT LEVEL
    cvDac[4].data.currentSample[3] = (1.0f - layerA.steiner.level[1]) * 4095.0f; // VOICE 1 | STEINER OUT LEVEL

    cvDac[5].data.currentSample[0] = (1.0f - layerA.ladder.cutoff[3]) * 4095.0f; // VOICE 3 | LADDER CUTOFF
    cvDac[5].data.currentSample[1] = (1.0f - layerA.ladder.cutoff[2]) * 4095.0f; // VOICE 2 | LADDER CUTOFF
    cvDac[5].data.currentSample[2] = layerA.steiner.cutoff[2] * 4095.0f;         // VOICE 2 | STEINER CUTOFF
    cvDac[5].data.currentSample[3] = layerA.steiner.cutoff[3] * 4095.0f;         // VOICE 3 | STEINER CUTOFF

    cvDac[6].data.currentSample[0] = (1.0f - layerA.ladder.resonance[3]) * 4095.0f; // VOICE 3 | LADDER RES
    cvDac[6].data.currentSample[1] = (1.0f - layerA.out.right[2]) * 4095.0f;        // VOICE 2 | OUT LEVEL R
    cvDac[6].data.currentSample[2] = (1.0f - layerA.out.left[2]) * 4095.0f;         // VOICE 2 | OUT LEVEL L
    cvDac[6].data.currentSample[3] = (1.0f - layerA.out.distort[3]) * 4095.0f;      // VOICE 3 | DISTORT LEVEL

    cvDac[7].data.currentSample[0] = (1.0f - layerA.out.right[3]) * 4095.0f;        // VOICE 3 | OUT LEVEL R
    cvDac[7].data.currentSample[1] = (1.0f - layerA.ladder.resonance[2]) * 4095.0f; // VOICE 2 | LADDER RES
    cvDac[7].data.currentSample[2] = (1.0f - layerA.out.distort[2]) * 4095.0f;      // VOICE 2 | DISTORT LEVEL
    cvDac[7].data.currentSample[3] = (1.0f - layerA.out.left[3]) * 4095.0f;         // VOICE 3 | OUT LEVEL L

    cvDac[8].data.currentSample[0] = (1.0f - layerA.steiner.resonance[3]) * 4095.0f; // VOICE 3 | STEINER RES
    cvDac[8].data.currentSample[1] = (1.0f - layerA.steiner.resonance[2]) * 4095.0f; // VOICE 2 | STEINER RES
    cvDac[8].data.currentSample[2] = (1.0f - layerA.steiner.toLadder[2]) * 4095.0f; // VOICE 2 | STEINER TO LADDER LEVEL
    cvDac[8].data.currentSample[3] = (1.0f - layerA.steiner.toLadder[3]) * 4095.0f; // VOICE 3 | STEINER TO LADDER LEVEL

    cvDac[9].data.currentSample[0] = (1.0f - layerA.ladder.level[3]) * 4095.0f;  // VOICE 3 | LADDER OUT LEVEL
    cvDac[9].data.currentSample[1] = (1.0f - layerA.ladder.level[2]) * 4095.0f;  // VOICE 2 | LADDER OUT LEVEL
    cvDac[9].data.currentSample[2] = (1.0f - layerA.steiner.level[2]) * 4095.0f; // VOICE 2 | STEINER OUT LEVEL
    cvDac[9].data.currentSample[3] = (1.0f - layerA.steiner.level[3]) * 4095.0f; // VOICE 3 | STEINER OUT LEVEL
}

inline void setSwitches() {

    const int32_t &steinerMode = layerA.steiner.dMode.valueMapped;
    const int32_t &ladderMode = layerA.ladder.dSlope.valueMapped;

    HAL_GPIO_WritePin(switch_1_A_GPIO_Port, switch_1_A_Pin, (GPIO_PinState)(steinerMode < 2));
    HAL_GPIO_WritePin(switch_1_B_GPIO_Port, switch_1_B_Pin, (GPIO_PinState)(!(steinerMode & 0x1)));

    HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, (GPIO_PinState)(ladderMode < 2));
    HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, (GPIO_PinState)(!(ladderMode & 0x1)));

    // switch (ladderMode) {
    //     case 0:
    //         HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_SET);
    //         HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_SET);
    //         break;
    //     case 1:
    //         HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_RESET);
    //         HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_SET);
    //         break;
    //     case 2:
    //         HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_SET);
    //         HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_RESET);
    //         break;
    //     case 3:
    //         HAL_GPIO_WritePin(switch_2_A_GPIO_Port, switch_2_A_Pin, GPIO_PIN_RESET);
    //         HAL_GPIO_WritePin(switch_2_B_GPIO_Port, switch_2_B_Pin, GPIO_PIN_RESET);
    //         break;
    //
    //     default: PolyError_Handler("renderCV | setSwitches | wrong ladderMode"); break;
    // }
}

LogCurve ledLog(64, 0.1);

inline void setLEDs() {
    // max brightness 1023

    // TODO LED assigment
    // TODO LED Brightness setting?

    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1,
                         ledLog.mapValue((layerA.out.left[2] + layerA.out.right[2]) * 0.5f) * 1023.0f); // 2
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2,
                         ledLog.mapValue((layerA.out.left[3] + layerA.out.right[3]) * 0.5f) * 1023.0f); // 3
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_1,
                         ledLog.mapValue(layerA.lfoA.out[0] * 0.5f + 0.5f) * 1023.0f); // 5
    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_2,
                         ledLog.mapValue(layerA.lfoB.out[0] * 0.5f + 0.5f) * 1023.0f); // 6
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1,
                         ledLog.mapValue((layerA.out.left[1] + layerA.out.right[1]) * 0.5f) * 1023.0f); // 1
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2,
                         ledLog.mapValue((layerA.out.left[0] + layerA.out.right[0]) * 0.5f) * 1023.0f); // 0
    // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, ledLog.mapValue(layerA.adsrB.out.currentSample[2]) * 1023.0f);
    // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_4, ledLog.mapValue(layerA.adsrB.out.currentSample[3]) * 1023.0f);
}

void renderCVs() {
    static uint32_t voice = 0;

    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);

    collectAllCurrentInputs();

    renderMidi();
    renderOSC_A();
    renderOSC_B();
    renderSub();
    renderNoise(layerA.noise);
    renderMixer(layerA.mixer);
    renderSteiner(layerA.steiner);
    renderLadder(layerA.ladder);
    renderLFO(layerA.lfoA);
    renderLFO(layerA.lfoB);
    renderADSR(layerA.envA);
    renderADSR(layerA.envF);
    renderOut(layerA.out);
    renderFeel(layerA.feel);
    renderWaveshaper(layerA.waveshaperA, voice);
    renderWaveshaper(layerA.waveshaperB, voice);
    renderPhaseshaper(layerA.phaseshaperA);
    renderPhaseshaper(layerA.phaseshaperB);

    // updateAllOutputSamples();
    writeDataToDACBuffer();

    setSwitches();
    setLEDs();

    voice++;
    if (voice >= VOICESPERCHIP)
        voice = 0;
}

#endif
