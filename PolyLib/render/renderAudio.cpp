#ifdef POLYRENDER

#include "renderAudio.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "rng.h"
#include <cmath>
#include <mdma.h>

extern Layer layerA;

std::vector<const WaveTable *> wavetables;

const WaveTable *sourcesA[4] = {nullptr, nullptr, nullptr, nullptr};
const WaveTable *sourcesB[4] = {nullptr, nullptr, nullptr, nullptr};

RAM1 ALIGN_32BYTES(float oscAwavetableStorage[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscAwavetable[MAXWAVETABLESPERVOICE] = {{0, 0, 0, oscAwavetableStorage[0], "defaultName"},
                                                  {0, 0, 0, oscAwavetableStorage[1], "defaultName"},
                                                  {0, 0, 0, oscAwavetableStorage[2], "defaultName"},
                                                  {0, 0, 0, oscAwavetableStorage[3], "defaultName"}};

RAM1 ALIGN_32BYTES(float oscBwavetableStorage[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscBwavetable[MAXWAVETABLESPERVOICE]{{0, 0, 0, oscBwavetableStorage[0], "defaultName"},
                                               {0, 0, 0, oscBwavetableStorage[1], "defaultName"},
                                               {0, 0, 0, oscBwavetableStorage[2], "defaultName"},
                                               {0, 0, 0, oscBwavetableStorage[3], "defaultName"}};

/**
 * @brief switch OscA Wavetable at position x
 *
 * @param position position between 0 and WAVETABLESPERVOICE
 * @param wavetable pointer to new wavetable
 */
void switchOscAWavetable(uint32_t position, const WaveTable *wavetable) {

    if (wavetable == sourcesA[position])
        return;

    oscAwavetable[position].size = wavetable->size;
    oscAwavetable[position].cycles = wavetable->cycles;
    oscAwavetable[position].sizePerCycle = wavetable->sizePerCycle;
    oscAwavetable[position].name = wavetable->name;
    sourcesA[position] = wavetable;

    MDMA_HandleTypeDef *mdmaHandle = nullptr;

    switch (position) {
        case 0: mdmaHandle = &hmdma_mdma_channel41_sw_0; break;
        case 1: mdmaHandle = &hmdma_mdma_channel42_sw_0; break;
        case 2: mdmaHandle = &hmdma_mdma_channel43_sw_0; break;
        case 3: mdmaHandle = &hmdma_mdma_channel44_sw_0; break;

        default: PolyError_Handler("renderAudio | switchOscAWavetable | illegal position"); break;
    }

    // while (HAL_MDMA_GetState(mdmaHandle) == HAL_MDMA_STATE_BUSY) {
    //     // wait for mdma to finish
    // }
    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)wavetable->data, (uint32_t)oscAwavetableStorage[position],
                      oscAwavetable[position].size * 4, 1);
}
/**
 * @brief switch OscB Wavetable at position x
 *
 * @param position position between 0 and WAVETABLESPERVOICE
 * @param wavetable pointer to new wavetable
 */
void switchOscBWavetable(uint32_t position, const WaveTable *wavetable) {

    if (wavetable == sourcesB[position])
        return;

    oscBwavetable[position].size = wavetable->size;
    oscBwavetable[position].cycles = wavetable->cycles;
    oscBwavetable[position].sizePerCycle = wavetable->sizePerCycle;
    oscBwavetable[position].name = wavetable->name;
    sourcesB[position] = wavetable;

    MDMA_HandleTypeDef *mdmaHandle = nullptr;

    switch (position) {
        case 0: mdmaHandle = &hmdma_mdma_channel45_sw_0; break;
        case 1: mdmaHandle = &hmdma_mdma_channel46_sw_0; break;
        case 2: mdmaHandle = &hmdma_mdma_channel47_sw_0; break;
        case 3: mdmaHandle = &hmdma_mdma_channel48_sw_0; break;

        default: PolyError_Handler("renderAudio | switchOscBWavetable | illegal position"); break;
    }

    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)wavetable->data, (uint32_t)oscBwavetableStorage[position],
                      oscBwavetable[position].size * 4, 1);
}

/**
 * @brief set and load default wavetables
 *
 */
void initAudioRendering() {

    wavetables.push_back(&wavetable_Sine);
    wavetables.push_back(&wavetable_Saw);
    wavetables.push_back(&wavetable_Triangle);
    wavetables.push_back(&wavetable_Square);
    // wavetables.push_back(&wavetable_FeltPianoLow);
    // wavetables.push_back(&wavetable_GuitarHigh);
    // wavetables.push_back(&wavetable_GuitarLow);
    wavetables.push_back(&wavetable_SSMMix01);
    wavetables.push_back(&wavetable_SSMMix02);
    wavetables.push_back(&wavetable_SSMMix03);
    wavetables.push_back(&wavetable_SSMMix04);
    // wavetables.push_back(&wavetable_SSMMix05);

    wavetables.push_back(&wavetable_SSMMix06);
    wavetables.push_back(&wavetable_SSMMix07);
    wavetables.push_back(&wavetable_SSMMix08);
    // wavetables.push_back(&wavetable_SSMSaw);
    // wavetables.push_back(&wavetable_SSMSquare);
    // wavetables.push_back(&wavetable_SSMTriangle);

    switchOscAWavetable(0, wavetables[layerA.oscA.dSample0.defaultValue]);
    switchOscAWavetable(1, wavetables[layerA.oscA.dSample1.defaultValue]);
    switchOscAWavetable(2, wavetables[layerA.oscA.dSample2.defaultValue]);
    switchOscAWavetable(3, wavetables[layerA.oscA.dSample3.defaultValue]);
    // switchOscAWavetableB(wavetable_nylonGuitar01);

    // Osc B
    switchOscBWavetable(0, wavetables[layerA.oscB.dSample0.defaultValue]);
    switchOscBWavetable(1, wavetables[layerA.oscB.dSample1.defaultValue]);
    switchOscBWavetable(2, wavetables[layerA.oscB.dSample2.defaultValue]);
    switchOscBWavetable(3, wavetables[layerA.oscB.dSample3.defaultValue]);
    // switchOscBWavetableA(wavetable_sinus01);
    // switchOscBWavetableB(wavetable_wurli02);
}

inline void bitcrush(float *dst, const float *bitcrush) {

    uint8_t moveAmount[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        moveAmount[i] = 23 - std::floor(bitcrush[i]);

    float mult[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        mult[i] = (1 << moveAmount[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        dst[i] = mult[i] * dst[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        dst[i] = std::round(dst[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        dst[i] = dst[i] / mult[i];
}

inline void getNoiseSample(float *sample) {
    // uint32_t bitcrusher[VOICESPERCHIP];
    uint32_t randomNumber[VOICESPERCHIP];
    // uint32_t bitcrushBits[VOICESPERCHIP];

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};

    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     bitcrusher[i] = layerA.noise.bitcrusher.currentSample[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber[i]);

    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     bitcrushBits[i] = (0xFFFFFFFF << bitcrusher[i]) & 0x00FFFFFF;

    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     randomNumber[i] = randomNumber[i] & bitcrushBits[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        randomNumber[i] = randomNumber[i] & 0x00FFFFFF;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;
    // map to -1, 1
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (sampleCrushCount[i] > layerA.noise.samplecrusher.currentSample[i]) {
            sample[i] = ((float)randomNumber[i] / 8388607.0f) - 1.0f;
            sampleCrushCount[i] = 0;
        }
}

inline void getSubSample(float *sample) {

    float oscAphase[VOICESPERCHIP];

    const float *bitcrusher = layerA.sub.bitcrusher.currentSample;
    const float *samplecrusher = layerA.sub.samplecrusher.currentSample;
    const float *shape = layerA.sub.shape.currentSample;

    const uint32_t &octave = layerA.sub.dOctaveSwitch.valueMapped;

    static float *phase = layerA.sub.phase;
    static float *oscApreviousPhase = layerA.sub.oscApreviousPhase;
    float newSample[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (phase[i] >= 1.0f)
            phase[i] -= 1.0f;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phase[i] < 0.5f) {
            newSample[i] = phase[i] * (4.0f / shape[i]) - 1.0f;
            if (newSample[i] > 1.0f)
                newSample[i] = 1.0f;
        }
        else {
            newSample[i] = (phase[i] - 0.5f) * (-4.0f / shape[i]) + 1.0f;
            if (newSample[i] < -1.0f)
                newSample[i] = -1.0f;
        }
    }
    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     newSample[i] = testFloat(newSample[i], -1.0f, 1.0f);

    bitcrush(newSample, bitcrusher);

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (sampleCrushCount[i] > samplecrusher[i]) {
            sample[i] = newSample[i];
            sampleCrushCount[i] = 0;
        }

    float phaseLength = octave == 0 ? 0.5f : 0.25f;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        oscAphase[i] = layerA.oscA.phaseWavetableA[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (oscAphase[i] >= 1.0f)
            oscAphase[i] -= std::floor(oscAphase[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (oscAphase[i] < oscApreviousPhase[i])
            oscApreviousPhase[i] -= 1.0f;

    static float phaseDifferenceOscA[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseDifferenceOscA[i] = oscAphase[i] - oscApreviousPhase[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        oscApreviousPhase[i] = oscAphase[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phase[i] += phaseDifferenceOscA[i] * phaseLength;
}

inline void getOscASample(float *sample) {
    static float *phaseWavetableA = layerA.oscA.phaseWavetableA;
    static float *phaseWavetableB = layerA.oscA.phaseWavetableB;
    const float *morph = layerA.oscA.morph.currentSample;
    const float *noteStep = layerA.oscA.note.currentSample;
    const float *bitcrusher = layerA.oscA.bitcrusher.currentSample;
    const float *samplecrusher = layerA.oscA.samplecrusher.currentSample;

    float stepWavetableA[VOICESPERCHIP];
    float stepWavetableB[VOICESPERCHIP];

    WaveTable *wavetableA[VOICESPERCHIP] = {nullptr};
    WaveTable *wavetableB[VOICESPERCHIP] = {nullptr};

    static uint8_t waveTableSelectionA[VOICESPERCHIP] = {0};
    static uint8_t waveTableSelectionB[VOICESPERCHIP] = {0};
    static uint8_t oldwaveTableSelectionA[VOICESPERCHIP] = {0};

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionA[i] = (uint8_t)morph[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionB[i] = (uint8_t)std::ceil(morph[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (waveTableSelectionA[i] == oldwaveTableSelectionA[i]) {
        }
        else if (waveTableSelectionA[i] < oldwaveTableSelectionA[i]) {
            phaseWavetableB[i] = phaseWavetableA[i];
            oldwaveTableSelectionA[i] = waveTableSelectionA[i];
        }
        else {
            phaseWavetableA[i] = phaseWavetableB[i];
            oldwaveTableSelectionA[i] = waveTableSelectionA[i];
        }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableA[i] = &oscAwavetable[waveTableSelectionA[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableB[i] = &oscAwavetable[waveTableSelectionB[i]];

    // make sure to be in the right step range
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableA[i] >= wavetableA[i]->cycles)
            phaseWavetableA[i] -= std::floor(phaseWavetableA[i]);
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableB[i] >= wavetableB[i]->cycles)
            phaseWavetableB[i] -= std::floor(phaseWavetableB[i]);
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableA[i] = phaseWavetableA[i] * wavetableA[i]->sizePerCycle;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableB[i] = phaseWavetableB[i] * wavetableB[i]->sizePerCycle;

    uint16_t positionA[VOICESPERCHIP];
    uint16_t positionAb[VOICESPERCHIP];
    float interSampleAPos[VOICESPERCHIP];
    float sampleA[VOICESPERCHIP];
    float sampleAb[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionA[i] = stepWavetableA[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionA[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (positionAb[i] == wavetableA[i]->size)
            positionAb[i] = 0;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleAPos[i] = stepWavetableA[i] - positionA[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = wavetableA[i]->data[positionA[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = wavetableA[i]->data[positionAb[i]];

    fast_lerp_f32(sampleA, sampleAb, interSampleAPos, sampleA);

    uint16_t positionB[VOICESPERCHIP];
    uint16_t positionBb[VOICESPERCHIP];
    float interSampleBPos[VOICESPERCHIP];
    float sampleB[VOICESPERCHIP];
    float sampleBb[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionB[i] = stepWavetableB[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionB[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (positionBb[i] == wavetableB[i]->size)
            positionBb[i] = 0;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleBPos[i] = stepWavetableB[i] - positionB[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = wavetableB[i]->data[positionB[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = wavetableB[i]->data[positionBb[i]];

    fast_lerp_f32(sampleB, sampleBb, interSampleBPos, sampleB);

    static float newSample[VOICESPERCHIP];

    float tempMorph[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        tempMorph[i] = morph[i] - std::floor(morph[i]);

    fast_lerp_f32(sampleA, sampleB, tempMorph, newSample);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableA[i] += noteStep[i] * PHASEPROGRESSPERRENDER;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableB[i] += noteStep[i] * PHASEPROGRESSPERRENDER;

    bitcrush(newSample, bitcrusher);

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (sampleCrushCount[i] > samplecrusher[i]) {
            sample[i] = newSample[i];
            sampleCrushCount[i] = 0;
        }
}

inline void getOscBSample(float *sample) {

    // cache step of osc A, if smaller, new phase has begun.
    static const float *oscAphaseWavetableA = layerA.oscA.phaseWavetableA;

    static int32_t &sync = layerA.oscB.dSync.valueMapped;

    static float *cacheOscAstep = layerA.oscB.cacheOscAstep;
    static float *phaseWavetableA = layerA.oscB.phaseWavetableA;
    static float *phaseWavetableB = layerA.oscB.phaseWavetableB;
    const float *morph = layerA.oscB.morph.currentSample;
    const float *noteStep = layerA.oscB.note.currentSample;
    const float *bitcrusher = layerA.oscB.bitcrusher.currentSample;
    const float *samplecrusher = layerA.oscB.samplecrusher.currentSample;

    float stepWavetableA[VOICESPERCHIP];
    float stepWavetableB[VOICESPERCHIP];
    WaveTable *wavetableA[VOICESPERCHIP] = {nullptr};
    WaveTable *wavetableB[VOICESPERCHIP] = {nullptr};
    static float checkWavetableLength[VOICESPERCHIP] = {0};

    static uint8_t waveTableSelectionA[VOICESPERCHIP] = {0};
    static uint8_t waveTableSelectionB[VOICESPERCHIP] = {0};
    static uint8_t oldwaveTableSelectionA[VOICESPERCHIP] = {0};

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionA[i] = (uint8_t)morph[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionB[i] = (uint8_t)std::ceil(morph[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (waveTableSelectionA[i] == oldwaveTableSelectionA[i]) {
        }
        else if (waveTableSelectionA[i] < oldwaveTableSelectionA[i]) {
            phaseWavetableB[i] = phaseWavetableA[i];
            oldwaveTableSelectionA[i] = waveTableSelectionA[i];
        }
        else {
            phaseWavetableA[i] = phaseWavetableB[i];
            oldwaveTableSelectionA[i] = waveTableSelectionA[i];
        }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableA[i] = &oscAwavetable[waveTableSelectionA[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableB[i] = &oscAwavetable[waveTableSelectionB[i]];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (oscAphaseWavetableA[i] >= 1.0f)
            checkWavetableLength[i] -= std::floor(oscAphaseWavetableA[i]);
        else
            checkWavetableLength[i] = oscAphaseWavetableA[i];
    }

    if (sync) {
        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            if (cacheOscAstep[i] > checkWavetableLength[i]) {
                stepWavetableA[i] = 0;
                stepWavetableB[i] = 0;
            }
        }
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        cacheOscAstep[i] = checkWavetableLength[i];

    // make sure to be in the right step range
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableA[i] >= wavetableA[i]->cycles) {
            phaseWavetableA[i] -= std::floor(phaseWavetableA[i]);
        }
    }
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableB[i] >= wavetableB[i]->cycles) {
            phaseWavetableB[i] -= std::floor(phaseWavetableB[i]);
        }
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableA[i] = phaseWavetableA[i] * wavetableA[i]->sizePerCycle;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableB[i] = phaseWavetableB[i] * wavetableB[i]->sizePerCycle;

    uint16_t positionA[VOICESPERCHIP];
    uint16_t positionAb[VOICESPERCHIP];
    float interSampleAPos[VOICESPERCHIP];
    float sampleA[VOICESPERCHIP];
    float sampleAb[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionA[i] = stepWavetableA[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionA[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (positionAb[i] == wavetableA[i]->size)
            positionAb[i] = 0;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleAPos[i] = stepWavetableA[i] - positionA[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = wavetableA[i]->data[positionA[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = wavetableA[i]->data[positionAb[i]];

    fast_lerp_f32(sampleA, sampleAb, interSampleAPos, sampleA);

    uint16_t positionB[VOICESPERCHIP];
    uint16_t positionBb[VOICESPERCHIP];
    float interSampleBPos[VOICESPERCHIP];
    float sampleB[VOICESPERCHIP];
    float sampleBb[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionB[i] = stepWavetableB[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionB[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (positionBb[i] == wavetableB[i]->size)
            positionBb[i] = 0;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleBPos[i] = stepWavetableB[i] - positionB[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = wavetableB[i]->data[positionB[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = wavetableB[i]->data[positionBb[i]];

    fast_lerp_f32(sampleB, sampleBb, interSampleBPos, sampleB);

    static float newSample[VOICESPERCHIP];

    float tempMorph[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        tempMorph[i] = morph[i] - std::floor(morph[i]);

    fast_lerp_f32(sampleA, sampleB, tempMorph, newSample);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableA[i] += noteStep[i] * PHASEPROGRESSPERRENDER;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableB[i] += noteStep[i] * PHASEPROGRESSPERRENDER;

    bitcrush(newSample, bitcrusher);

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (sampleCrushCount[i] > samplecrusher[i]) {
            sample[i] = newSample[i];
            sampleCrushCount[i] = 0;
        }
}

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 */
void renderAudio(int32_t *renderDest) {
    // speed test led - obsolete
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

    const float *noiseLevelLadder = layerA.noise.levelLadder.currentSample;
    const float *noiseLevelSteiner = layerA.noise.levelSteiner.currentSample;
    float *noiseOut = layerA.noise.out.nextSample;

    const float *subLevelLadder = layerA.sub.levelLadder.currentSample;
    const float *subLevelSteiner = layerA.sub.levelSteiner.currentSample;
    float *subOut = layerA.sub.out.nextSample;

    const float *oscALevelLadder = layerA.oscA.levelLadder.currentSample;
    const float *oscALevelSteiner = layerA.oscA.levelSteiner.currentSample;
    float *oscAOut = layerA.oscA.out.nextSample;

    const float *oscBLevelLadder = layerA.oscB.levelLadder.currentSample;
    const float *oscBLevelSteiner = layerA.oscB.levelSteiner.currentSample;
    float *oscBOut = layerA.oscB.out.nextSample;

    static float oscASample[VOICESPERCHIP] = {0};
    static float oscBSample[VOICESPERCHIP] = {0};
    static float noiseSample[VOICESPERCHIP] = {0};
    static float subSample[VOICESPERCHIP] = {0};

    float dampSteiner[VOICESPERCHIP];
    float dampLadder[VOICESPERCHIP];

    float maxVolSteiner[VOICESPERCHIP];
    float maxVolLadder[VOICESPERCHIP];

    float sampleSteiner[VOICESPERCHIP];
    float sampleLadder[VOICESPERCHIP];

    int32_t intSampleSteiner[VOICESPERCHIP];
    int32_t intSampleLadder[VOICESPERCHIP];

    // Osc A
    switchOscAWavetable(0, wavetables[layerA.oscA.dSample0.valueMapped]);
    switchOscAWavetable(1, wavetables[layerA.oscA.dSample1.valueMapped]);
    switchOscAWavetable(2, wavetables[layerA.oscA.dSample2.valueMapped]);
    switchOscAWavetable(3, wavetables[layerA.oscA.dSample3.valueMapped]);

    // Osc B
    switchOscBWavetable(0, wavetables[layerA.oscB.dSample0.valueMapped]);
    switchOscBWavetable(1, wavetables[layerA.oscB.dSample1.valueMapped]);
    switchOscBWavetable(2, wavetables[layerA.oscB.dSample2.valueMapped]);
    switchOscBWavetable(3, wavetables[layerA.oscB.dSample3.valueMapped]);

    for (uint32_t sample = 0; sample < SAIDMABUFFERSIZE; sample++) {

        getOscASample(oscASample);
        getOscBSample(oscBSample);
        getNoiseSample(noiseSample);
        getSubSample(subSample);

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] = noiseLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] += subLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] += oscALevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] += oscBLevelSteiner[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] = noiseLevelLadder[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] += subLevelLadder[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] += oscALevelLadder[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] += oscBLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            if (maxVolSteiner[i] > MAXPOSSIBLEVOLUME) {
                dampSteiner[i] = MAXPOSSIBLEVOLUME / maxVolSteiner[i];
            }
            else {
                dampSteiner[i] = 1;
            }
        }

        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            if (maxVolLadder[i] > MAXPOSSIBLEVOLUME) {
                dampLadder[i] = MAXPOSSIBLEVOLUME / maxVolLadder[i];
            }
            else {
                dampLadder[i] = 1;
            }
        }

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] = noiseSample[i] * noiseLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] = noiseSample[i] * noiseLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] += subSample[i] * subLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] += subSample[i] * subLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] += oscASample[i] * oscALevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] += oscASample[i] * oscALevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] += oscBSample[i] * oscBLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] += oscBSample[i] * oscBLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] = sampleSteiner[i] * dampSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] = sampleLadder[i] * dampLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] *= MAXVOLUMEPERMODULE;
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] *= MAXVOLUMEPERMODULE;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] *= 8388607.0f;
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] *= 8388607.0f;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            intSampleSteiner[i] = sampleSteiner[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            intSampleLadder[i] = sampleLadder[i];

        renderDest[sample * AUDIOCHANNELS + 1 * 2 + 1] = intSampleLadder[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2 + 1] = intSampleLadder[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2 + 1] = intSampleLadder[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2 + 1] = intSampleLadder[3];

        renderDest[sample * AUDIOCHANNELS + 1 * 2] = intSampleSteiner[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2] = intSampleSteiner[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2] = intSampleSteiner[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2] = intSampleSteiner[3];

        if (sample == 0) {
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                noiseOut[i] = noiseSample[i];
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                subOut[i] = subSample[i];
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                oscAOut[i] = oscASample[i];
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                oscBOut[i] = oscBSample[i];
        }
    }
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
}

#endif