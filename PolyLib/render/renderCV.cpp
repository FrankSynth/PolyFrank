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
#include "renderTune.hpp"
#include "renderWaveshaper.hpp"

extern DUALBU22210 cvDac[2];
extern Layer layerA;
extern volatile float interchipLFOBuffer[4];

/*
List of assignement

    10 xxxxxx -- VOICE 1 | STEINER RES
    00 xxxxxx -- VOICE 0 | OUT LEVEL R

    11 xxxxxx -- VOICE 1 | STEINER TO LADDER LEVEL
    01 xxxxxx -- VOICE 0 | STEINER CUTOFF

    12 xxxxxx -- VOICE 1 | DISTORT LEVEL
    02 xxxxxx -- VOICE 0 | LADDER OUT LEVEL

    13 xxxxxx -- VOICE 0 | OUT LEVEL L
    03 xxxxxx -- VOICE 0 | STEINER OUT LEVEL

    14 xxxxxx -- VOICE 0 | LADDER CUTOFF
    04 xxxxxx -- VOICE 0 | STEINER RES

    15 xxxxxx -- VOICE 1 | LADDER RES
    05 xxxxxx -- VOICE 1 | STEINER CUTOFF

    16 xxxxxx -- VOICE 1 | OUT LEVEL R
    06 xxxxxx -- VOICE 1 | LADDER OUT LEVEL

    17 xxxxxx -- VOICE 1 | OUT LEVEL L
    07 xxxxxx -- VOICE 0 | STEINER TO LADDER LEVEL

    18 xxxxxx -- VOICE 1 | LADDER CUTOFF
    08 xxxxxx -- VOICE 0 | DISTORT LEVEL

    19 xxxxxx -- VOICE 1 | STEINER OUT LEVEL
    09 xxxxxx -- VOICE 0 | LADDER RES

    20 xxxxxx -- VOICE 2 | OUT LEVEL R
    21 xxxxxx -- VOICE 2 | STEINER CUTOFF
    22 xxxxxx -- VOICE 2 | LADDER OUT LEVEL
    23 xxxxxx -- VOICE 2 | STEINER OUT LEVEL

    24 xxxxxx -- VOICE 2 | STEINER RES
    25 xxxxxx -- VOICE 3 | STEINER CUTOFF
    26 xxxxxx -- VOICE 3 | LADDER OUT LEVEL
    27 xxxxxx -- VOICE 2 | STEINER TO LADDER LEVEL

    28 xxxxxx -- VOICE 2 | DISTORT LEVEL
    29 xxxxxx -- VOICE 2 | LADDER RES
    30 xxxxxx -- VOICE 3 | STEINER RES
    31 xxxxxx -- VOICE 3 | STEINER TO LADDER LEVEL

    32 xxxxxx -- VOICE 3 | DISTORT LEVEL
    33 xxxxxx -- VOICE 2 | OUT LEVEL L
    34 xxxxxx -- VOICE 2 | LADDER CUTOFF
    35 xxxxxx -- VOICE 3 | LADDER RES

    36 xxxxxx -- VOICE 3 | OUT LEVEL R
    37 xxxxxx -- VOICE 3 | OUT LEVEL L
    38 xxxxxx -- VOICE 3 | LADDER CUTOFF
    39 xxxxxx -- VOICE 3 | STEINER OUT LEVEL

*/

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

    cvDac[0].data[0] = (1.0f - layerA.out.right[0]) * 4095.0f;
    cvDac[0].data[1] = (1.0f - layerA.steiner.resonance[1]) * 4095.0f;

    cvDac[0].data[2] = layerA.steiner.cutoff[0] * 4095.0f;
    cvDac[0].data[3] = (1.0f - layerA.steiner.toLadder[1]) * 4095.0f;

    cvDac[0].data[4] = (1.0f - layerA.ladder.level[0]) * 4095.0f;
    cvDac[0].data[5] = (1.0f - layerA.out.distort[1]) * 4095.0f;

    cvDac[0].data[6] = (1.0f - layerA.steiner.level[0]) * 4095.0f;
    cvDac[0].data[7] = (1.0f - layerA.out.left[0]) * 4095.0f;

    cvDac[0].data[8] = (1.0f - layerA.steiner.resonance[0]) * 4095.0f;
    cvDac[0].data[9] = (1.0f - layerA.ladder.cutoff[0]) * 4095.0f;

    cvDac[0].data[10] = layerA.steiner.cutoff[1] * 4095.0f;
    cvDac[0].data[11] = (1.0f - layerA.ladder.resonance[1]) * 4095.0f;

    cvDac[0].data[12] = (1.0f - layerA.ladder.level[1]) * 4095.0f;
    cvDac[0].data[13] = (1.0f - layerA.out.right[1]) * 4095.0f;

    cvDac[0].data[14] = (1.0f - layerA.steiner.toLadder[0]) * 4095.0f;
    cvDac[0].data[15] = (1.0f - layerA.out.left[1]) * 4095.0f;

    cvDac[0].data[16] = (1.0f - layerA.out.distort[0]) * 4095.0f;
    cvDac[0].data[17] = (1.0f - layerA.ladder.cutoff[1]) * 4095.0f;

    cvDac[0].data[18] = (1.0f - layerA.ladder.resonance[0]) * 4095.0f;
    cvDac[0].data[19] = (1.0f - layerA.steiner.level[1]) * 4095.0f;

    cvDac[0].preparePackage();

    cvDac[1].data[0] = (1.0f - layerA.out.right[2]) * 4095.0f;
    cvDac[1].data[1] = (1.0f - layerA.steiner.resonance[3]) * 4095.0f;

    cvDac[1].data[2] = (layerA.steiner.cutoff[2]) * 4095.0f;
    cvDac[1].data[3] = (1.0f - layerA.steiner.toLadder[3]) * 4095.0f;

    cvDac[1].data[4] = (1.0f - layerA.ladder.level[2]) * 4095.0f;
    cvDac[1].data[5] = (1.0f - layerA.out.distort[3]) * 4095.0f;

    cvDac[1].data[6] = (1.0f - layerA.steiner.level[2]) * 4095.0f;
    cvDac[1].data[7] = (1.0f - layerA.out.left[2]) * 4095.0f;

    cvDac[1].data[8] = (1.0f - layerA.steiner.resonance[2]) * 4095.0f;
    cvDac[1].data[9] = (1.0f - layerA.ladder.cutoff[2]) * 4095.0f;

    cvDac[1].data[10] = (layerA.steiner.cutoff[3]) * 4095.0f;
    cvDac[1].data[11] = (1.0f - layerA.ladder.resonance[3]) * 4095.0f;

    cvDac[1].data[12] = (1.0f - layerA.ladder.level[3]) * 4095.0f;
    cvDac[1].data[13] = (1.0f - layerA.out.right[3]) * 4095.0f;

    cvDac[1].data[14] = (1.0f - layerA.steiner.toLadder[2]) * 4095.0f;
    cvDac[1].data[15] = (1.0f - layerA.out.left[3]) * 4095.0f;

    cvDac[1].data[16] = (1.0f - layerA.out.distort[2]) * 4095.0f;
    cvDac[1].data[17] = (1.0f - layerA.ladder.cutoff[3]) * 4095.0f;

    cvDac[1].data[18] = (1.0f - layerA.ladder.resonance[2]) * 4095.0f;
    cvDac[1].data[19] = (1.0f - layerA.steiner.level[3]) * 4095.0f;

    cvDac[1].preparePackage();
}

inline void setSwitches() {

    const int32_t &steinerMode = layerA.steiner.dMode.valueMapped;
    const int32_t &ladderMode = layerA.ladder.dSlope.valueMapped;

    HAL_GPIO_WritePin(SWITCH_2_B_GPIO_Port, SWITCH_2_B_Pin, (GPIO_PinState)(!(steinerMode & 0b1)));
    HAL_GPIO_WritePin(SWITCH_2_A_GPIO_Port, SWITCH_2_A_Pin, (GPIO_PinState)(steinerMode < 2));

    HAL_GPIO_WritePin(SWITCH_1_B_GPIO_Port, SWITCH_1_B_Pin, (GPIO_PinState)(ladderMode < 2));
    HAL_GPIO_WritePin(SWITCH_1_A_GPIO_Port, SWITCH_1_A_Pin, (GPIO_PinState)(!(ladderMode & 0b1)));
}

void renderCVs() {
    static uint32_t voice = 0;

    collectAllCurrentInputs();

    renderMidi();
    renderOSC_A();
    renderOSC_B();
    renderSub();
    renderNoise(layerA.noise);
    renderMixer(layerA.mixer);
    renderTune(layerA.tune);
    renderSteiner(layerA.steiner);
    renderLadder(layerA.ladder);
    renderLFO(layerA.lfoA);
    renderLFO(layerA.lfoB);
    renderADSR(layerA.envA);
    renderADSR(layerA.envF);
    renderOut(layerA.out);
    renderFeel(layerA.feel);
    renderWaveshaper(layerA.waveshaperA, layerA.oscA.effect, voice);
    renderWaveshaper(layerA.waveshaperB, layerA.oscB.effect, voice);
    renderPhaseshaper(layerA.phaseshaperA, layerA.oscA.effect);
    renderPhaseshaper(layerA.phaseshaperB, layerA.oscB.effect);

    // updateAllOutputSamples();
    writeDataToDACBuffer();

    setSwitches();

    if (layerA.chipID == 0) { // chip A
        interchipLFOBuffer[0] = layerA.lfoA.alignFloatBuffer;
        interchipLFOBuffer[1] = layerA.lfoB.alignFloatBuffer;
        interchipLFOBuffer[2] = layerA.lfoA.alignPhaseBuffer;
        interchipLFOBuffer[3] = layerA.lfoB.alignPhaseBuffer;
    }
    else {
        layerA.lfoA.alignFloatBuffer = interchipLFOBuffer[0];
        layerA.lfoB.alignFloatBuffer = interchipLFOBuffer[1];
        layerA.lfoA.alignPhaseBuffer = interchipLFOBuffer[2];
        layerA.lfoB.alignPhaseBuffer = interchipLFOBuffer[3];
    }

    voice++;
    voice = voice & 0b11;
}

#endif
