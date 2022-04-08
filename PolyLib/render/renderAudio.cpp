#ifdef POLYRENDER

#include "renderAudio.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "rng.h"
#include <cmath>
#include <mdma.h>

extern Layer layerA;

// std::vector<const WaveTable *> wavetables;

const WaveTable *sourcesA[4] = {nullptr, nullptr, nullptr, nullptr};
const WaveTable *sourcesB[4] = {nullptr, nullptr, nullptr, nullptr};

RAM1 ALIGN_32BYTES(float oscAwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscAwavetable[MAXWAVETABLESPERVOICE] = {{0, 0, 0, oscAwavetableRAM[0], "defaultName"},
                                                  {0, 0, 0, oscAwavetableRAM[1], "defaultName"},
                                                  {0, 0, 0, oscAwavetableRAM[2], "defaultName"},
                                                  {0, 0, 0, oscAwavetableRAM[3], "defaultName"}};

RAM1 ALIGN_32BYTES(float oscBwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscBwavetable[MAXWAVETABLESPERVOICE] = {{0, 0, 0, oscBwavetableRAM[0], "defaultName"},
                                                  {0, 0, 0, oscBwavetableRAM[1], "defaultName"},
                                                  {0, 0, 0, oscBwavetableRAM[2], "defaultName"},
                                                  {0, 0, 0, oscBwavetableRAM[3], "defaultName"}};

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

    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)wavetable->data, (uint32_t)oscAwavetableRAM[position],
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
    sourcesB[position] = wavetable;

    MDMA_HandleTypeDef *mdmaHandle = nullptr;

    switch (position) {
        case 0: mdmaHandle = &hmdma_mdma_channel45_sw_0; break;
        case 1: mdmaHandle = &hmdma_mdma_channel46_sw_0; break;
        case 2: mdmaHandle = &hmdma_mdma_channel47_sw_0; break;
        case 3: mdmaHandle = &hmdma_mdma_channel48_sw_0; break;

        default: PolyError_Handler("renderAudio | switchOscBWavetable | illegal position"); break;
    }

    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)wavetable->data, (uint32_t)oscBwavetableRAM[position],
                      oscBwavetable[position].size * 4, 1);
}

/**
 * @brief set and load default wavetables
 *
 */
void loadInitialWavetables() {

    switchOscAWavetable(0, wavetables[layerA.oscA.dSample0.defaultValue]);
    switchOscAWavetable(1, wavetables[layerA.oscA.dSample1.defaultValue]);
    switchOscAWavetable(2, wavetables[layerA.oscA.dSample2.defaultValue]);
    switchOscAWavetable(3, wavetables[layerA.oscA.dSample3.defaultValue]);

    // Osc B
    switchOscBWavetable(0, wavetables[layerA.oscB.dSample0.defaultValue]);
    switchOscBWavetable(1, wavetables[layerA.oscB.dSample1.defaultValue]);
    switchOscBWavetable(2, wavetables[layerA.oscB.dSample2.defaultValue]);
    switchOscBWavetable(3, wavetables[layerA.oscB.dSample3.defaultValue]);
}

// TODO new bitcrusher von Jacob??
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

    uint32_t randomNumber[VOICESPERCHIP];

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        randomNumber[i] = randomNumber[i] & 0x00FFFFFF;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;
    // map to -1, 1
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (sampleCrushCount[i] > layerA.noise.samplecrusher[i]) {
            sample[i] = ((float)randomNumber[i] / 8388607.0f) - 1.0f;
            sampleCrushCount[i] = 0;
        }
}

inline void getSubSample(float *sample) {

    float oscAphase[VOICESPERCHIP];

    const float *bitcrusher = layerA.sub.bitcrusher;
    const float *samplecrusher = layerA.sub.samplecrusher;
    const float *shape = layerA.sub.shape;

    const uint32_t &octave = layerA.sub.dOctaveSwitch.valueMapped;

    static float *phase = layerA.sub.phase;
    static float *oscApreviousPhase = layerA.sub.oscApreviousPhase;
    float newSample[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phase[i] -= std::floor(phase[i]);

    float shapeDiv[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        shapeDiv[i] = 4.0f / shape[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phase[i] < 0.5f) {
            newSample[i] = phase[i] * (shapeDiv[i]) - 1.0f;
            newSample[i] = std::min(newSample[i], 1.0f);
        }
        else {
            newSample[i] = (phase[i] - 0.5f) * (-shapeDiv[i]) + 1.0f;
            newSample[i] = std::max(newSample[i], -1.0f);
        }
    }

    bitcrush(newSample, bitcrusher);

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};
    bool sampleCrushNow[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushNow[i] = sampleCrushCount[i] > samplecrusher[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i] = !sampleCrushNow;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sample[i] = newSample[i] * sampleCrushNow[i] + sample[i] * !sampleCrushNow[i];

    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     if (sampleCrushCount[i] > samplecrusher[i]) {
    //         sample[i] = newSample[i];
    //         sampleCrushCount[i] = 0;
    //     }

    float phaseLength = octave == 0 ? 0.5f : 0.25f;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        oscAphase[i] = layerA.oscA.phaseWavetableLower[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
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
    vec<VOICESPERCHIP> &phaseWavetableLower = layerA.oscA.phaseWavetableLower;
    vec<VOICESPERCHIP> &phaseWavetableUpper = layerA.oscA.phaseWavetableUpper;
    const vec<VOICESPERCHIP> &morph = layerA.oscA.morph;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscA.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscA.bitcrusher;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscA.samplecrusher;
    const vec<VOICESPERCHIP> &squircle = layerA.oscA.squircle;

    float stepWavetableLower[VOICESPERCHIP];
    float stepWavetableUpper[VOICESPERCHIP];

    WaveTable *wavetableLower[VOICESPERCHIP] = {nullptr};
    WaveTable *wavetableUpper[VOICESPERCHIP] = {nullptr};

    static uint8_t waveTableSelectionLower[VOICESPERCHIP] = {0};
    static uint8_t waveTableSelectionUpper[VOICESPERCHIP] = {0};
    static uint8_t oldwaveTableSelectionLower[VOICESPERCHIP] = {0};

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionLower[i] = (uint8_t)morph[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionUpper[i] = (uint8_t)std::ceil(morph[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (waveTableSelectionLower[i] == oldwaveTableSelectionLower[i]) {
        }
        else if (waveTableSelectionLower[i] < oldwaveTableSelectionLower[i]) {
            phaseWavetableUpper[i] = phaseWavetableLower[i];
            oldwaveTableSelectionLower[i] = waveTableSelectionLower[i];
        }
        else {
            phaseWavetableLower[i] = phaseWavetableUpper[i];
            oldwaveTableSelectionLower[i] = waveTableSelectionLower[i];
        }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableLower[i] = &oscAwavetable[waveTableSelectionLower[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableUpper[i] = &oscAwavetable[waveTableSelectionUpper[i]];

    // make sure to be in the right step range
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableLower[i] -=
            std::floor(phaseWavetableLower[i]) * (phaseWavetableLower[i] >= wavetableLower[i]->cycles);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableUpper[i] -=
            std::floor(phaseWavetableUpper[i]) * (phaseWavetableUpper[i] >= wavetableUpper[i]->cycles);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = phaseWavetableLower[i] * wavetableLower[i]->sizePerCycle;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableUpper[i] = phaseWavetableUpper[i] * wavetableUpper[i]->sizePerCycle;

    uint16_t positionA[VOICESPERCHIP];
    uint16_t positionAb[VOICESPERCHIP];
    float interSampleAPos[VOICESPERCHIP];
    float sampleA[VOICESPERCHIP];
    float sampleAb[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionA[i] = stepWavetableLower[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionA[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != wavetableLower[i]->size);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleAPos[i] = stepWavetableLower[i] - positionA[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscAwavetableRAM[waveTableSelectionLower[i]][positionA[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = oscAwavetableRAM[waveTableSelectionLower[i]][positionAb[i]];

    fast_lerp_f32(sampleA, sampleAb, interSampleAPos, sampleA);

    uint16_t positionB[VOICESPERCHIP];
    uint16_t positionBb[VOICESPERCHIP];
    float interSampleBPos[VOICESPERCHIP];
    float sampleB[VOICESPERCHIP];
    float sampleBb[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionB[i] = stepWavetableUpper[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionB[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionBb[i] * (positionBb[i] != wavetableUpper[i]->size);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleBPos[i] = stepWavetableUpper[i] - positionB[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscAwavetableRAM[waveTableSelectionUpper[i]][positionB[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = oscAwavetableRAM[waveTableSelectionUpper[i]][positionBb[i]];

    fast_lerp_f32(sampleB, sampleBb, interSampleBPos, sampleB);

    static vec<VOICESPERCHIP> newSample;

    float tempMorph[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        tempMorph[i] = morph[i] - std::floor(morph[i]);

    fast_lerp_f32(sampleA, sampleB, tempMorph, newSample);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableLower[i] += noteStep[i] * PHASEPROGRESSPERRENDER;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableUpper[i] += noteStep[i] * PHASEPROGRESSPERRENDER;

    // calcSquircleSimplified(newSample, squircle);
    newSample = simpleBezier1D(squircle, newSample);

    bitcrush(newSample, bitcrusher);

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};
    bool sampleCrushNow[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushNow[i] = sampleCrushCount[i] > samplecrusher[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i] = !sampleCrushNow;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sample[i] = newSample[i] * sampleCrushNow[i] + sample[i] * !sampleCrushNow[i];
}

void getOscBSample(vec<VOICESPERCHIP> &sample) {

    // cache step of osc A, if smaller, new phase has begun.
    const vec<VOICESPERCHIP> &oscAphaseWavetableLower = layerA.oscA.phaseWavetableLower;

    static int32_t &sync = layerA.oscB.dSync.valueMapped;

    vec<VOICESPERCHIP> &cacheOscAPhase = layerA.oscB.cacheOscAPhase;
    vec<VOICESPERCHIP> &phaseWavetableLower = layerA.oscB.phaseWavetableLower;
    vec<VOICESPERCHIP> &phaseWavetableUpper = layerA.oscB.phaseWavetableUpper;
    const vec<VOICESPERCHIP> &morph = layerA.oscB.morph;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscB.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscB.bitcrusher;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscB.samplecrusher;
    const vec<VOICESPERCHIP> &phaseoffset = layerA.oscB.phaseoffset;
    const vec<VOICESPERCHIP> &squircle = layerA.oscA.squircle;

    vec<VOICESPERCHIP> stepWavetableLower;
    vec<VOICESPERCHIP> stepWavetableUpper;
    WaveTable *wavetableLower[VOICESPERCHIP] = {nullptr};
    WaveTable *wavetableUpper[VOICESPERCHIP] = {nullptr};
    static vec<VOICESPERCHIP> checkWavetableLength;

    static uint8_t waveTableSelectionLower[VOICESPERCHIP] = {0};
    static uint8_t waveTableSelectionUpper[VOICESPERCHIP] = {0};
    static uint8_t oldwaveTableSelectionLower[VOICESPERCHIP] = {0};

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionLower[i] = (uint8_t)morph[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        waveTableSelectionUpper[i] = (uint8_t)std::ceil(morph[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (waveTableSelectionLower[i] == oldwaveTableSelectionLower[i]) {
        }
        else if (waveTableSelectionLower[i] < oldwaveTableSelectionLower[i]) {
            phaseWavetableUpper[i] = phaseWavetableLower[i];
            oldwaveTableSelectionLower[i] = waveTableSelectionLower[i];
        }
        else {
            phaseWavetableLower[i] = phaseWavetableUpper[i];
            oldwaveTableSelectionLower[i] = waveTableSelectionLower[i];
        }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableLower[i] = &oscBwavetable[waveTableSelectionLower[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableUpper[i] = &oscBwavetable[waveTableSelectionUpper[i]];

    checkWavetableLength = oscAphaseWavetableLower - floor(oscAphaseWavetableLower);

    bool syncWavetablesNow[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        syncWavetablesNow[i] = sync && (cacheOscAPhase[i] > checkWavetableLength[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableLower[i] = phaseWavetableLower[i] * !syncWavetablesNow[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableUpper[i] = phaseWavetableUpper[i] * !syncWavetablesNow[i];
    phaseWavetableUpper = phaseWavetableUpper * !syncWavetablesNow;

    cacheOscAPhase = checkWavetableLength;

    // make sure to be in the right step range
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableLower[i] -=
            std::floor(phaseWavetableLower[i]) * (phaseWavetableLower[i] >= wavetableLower[i]->cycles);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableUpper[i] -=
            std::floor(phaseWavetableUpper[i]) * (phaseWavetableUpper[i] >= wavetableUpper[i]->cycles);

    float phaseOffsettedLower[VOICESPERCHIP];
    float phaseOffsettedUpper[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseOffsettedLower[i] = phaseWavetableLower[i] + phaseoffset[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseOffsettedUpper[i] = phaseWavetableUpper[i] + phaseoffset[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseOffsettedLower[i] -=
            std::floor(phaseOffsettedLower[i]) * (phaseOffsettedLower[i] >= wavetableLower[i]->cycles);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseOffsettedLower[i] =
            phaseOffsettedLower[i] * !(phaseOffsettedLower[i] < 0) +
            (wavetableLower[i]->cycles - phaseOffsettedLower[i] - std::floor(phaseOffsettedLower[i])) *
                (phaseOffsettedLower[i] < 0);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseOffsettedUpper[i] -=
            std::floor(phaseOffsettedUpper[i]) * (phaseOffsettedUpper[i] >= wavetableUpper[i]->cycles);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseOffsettedUpper[i] =
            phaseOffsettedUpper[i] * !(phaseOffsettedUpper[i] < 0) +
            (wavetableUpper[i]->cycles - phaseOffsettedUpper[i] - std::floor(phaseOffsettedUpper[i])) *
                (phaseOffsettedUpper[i] < 0);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = phaseOffsettedLower[i] * wavetableLower[i]->sizePerCycle;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableUpper[i] = phaseOffsettedUpper[i] * wavetableUpper[i]->sizePerCycle;

    uint16_t positionA[VOICESPERCHIP];
    uint16_t positionAb[VOICESPERCHIP];
    vec<VOICESPERCHIP> interSampleAPos;
    vec<VOICESPERCHIP> sampleA;
    vec<VOICESPERCHIP> sampleAb;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionA[i] = stepWavetableLower[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionA[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != wavetableLower[i]->size);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleAPos[i] = stepWavetableLower[i] - positionA[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscBwavetableRAM[waveTableSelectionLower[i]][positionA[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = oscBwavetableRAM[waveTableSelectionLower[i]][positionAb[i]];

    fast_lerp_f32(sampleA, sampleAb, interSampleAPos, sampleA);

    uint16_t positionB[VOICESPERCHIP];
    uint16_t positionBb[VOICESPERCHIP];
    vec<VOICESPERCHIP> interSampleBPos;
    vec<VOICESPERCHIP> sampleB;
    vec<VOICESPERCHIP> sampleBb;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionB[i] = stepWavetableUpper[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionB[i] + 1;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionBb[i] * (positionBb[i] != wavetableUpper[i]->size);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        interSampleBPos[i] = stepWavetableUpper[i] - positionB[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscBwavetableRAM[waveTableSelectionUpper[i]][positionB[i]];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = oscBwavetableRAM[waveTableSelectionUpper[i]][positionBb[i]];

    fast_lerp_f32(sampleB, sampleBb, interSampleBPos, sampleB);

    static vec<VOICESPERCHIP> newSample;

    float tempMorph[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        tempMorph[i] = morph[i] - std::floor(morph[i]);

    fast_lerp_f32(sampleA, sampleB, tempMorph, newSample);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableLower[i] += noteStep[i] * PHASEPROGRESSPERRENDER;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableUpper[i] += noteStep[i] * PHASEPROGRESSPERRENDER;

    // calcSquircleSimplified(newSample, squircle);
    newSample = simpleBezier1D(squircle, newSample);

    bitcrush(newSample, bitcrusher);

    static uint16_t sampleCrushCount[VOICESPERCHIP] = {0};
    bool sampleCrushNow[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i]++;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushNow[i] = sampleCrushCount[i] > samplecrusher[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleCrushCount[i] = !sampleCrushNow;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sample[i] = newSample[i] * sampleCrushNow[i] + sample[i] * !sampleCrushNow[i];
}

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 */
void renderAudio(int32_t *renderDest) {

    const vec<VOICESPERCHIP> &noiseLevelLadder = layerA.mixer.noiseLevelLadder;
    const vec<VOICESPERCHIP> &noiseLevelSteiner = layerA.mixer.noiseLevelSteiner;

    const vec<VOICESPERCHIP> &subLevelLadder = layerA.mixer.subLevelLadder;
    const vec<VOICESPERCHIP> &subLevelSteiner = layerA.mixer.subLevelSteiner;

    const vec<VOICESPERCHIP> &oscALevelLadder = layerA.mixer.oscALevelLadder;
    const vec<VOICESPERCHIP> &oscALevelSteiner = layerA.mixer.oscALevelSteiner;

    const vec<VOICESPERCHIP> &oscBLevelLadder = layerA.mixer.oscBLevelLadder;
    const vec<VOICESPERCHIP> &oscBLevelSteiner = layerA.mixer.oscBLevelSteiner;

    static vec<VOICESPERCHIP> oscASample;
    static vec<VOICESPERCHIP> oscBSample;
    static vec<VOICESPERCHIP> noiseSample;
    static vec<VOICESPERCHIP> subSample;

    vec<VOICESPERCHIP> dampSteiner;
    vec<VOICESPERCHIP> dampLadder;

    vec<VOICESPERCHIP> maxVolSteiner;
    vec<VOICESPERCHIP> maxVolLadder;

    vec<VOICESPERCHIP> sampleSteiner;
    vec<VOICESPERCHIP> sampleLadder;

    int32_t intSampleSteiner[VOICESPERCHIP];
    int32_t intSampleLadder[VOICESPERCHIP];

    for (uint32_t sample = 0; sample < SAIDMABUFFERSIZE; sample++) {

        getOscASample(oscASample);
        getOscBSample(oscBSample);
        getNoiseSample(noiseSample);
        getSubSample(subSample);

        maxVolSteiner = noiseLevelSteiner;

        maxVolSteiner += subLevelSteiner;

        maxVolSteiner += oscALevelSteiner;

        maxVolSteiner += oscBLevelSteiner;

        maxVolLadder = noiseLevelLadder;

        maxVolLadder += subLevelLadder;

        maxVolLadder += oscALevelLadder;

        maxVolLadder += oscBLevelLadder;

        dampSteiner = (MAXPOSSIBLEVOLUME / maxVolSteiner) * (maxVolSteiner > MAXPOSSIBLEVOLUME) +
                      (maxVolSteiner <= MAXPOSSIBLEVOLUME);

        dampLadder = (MAXPOSSIBLEVOLUME / maxVolLadder) * (maxVolLadder > MAXPOSSIBLEVOLUME) +
                     (maxVolLadder <= MAXPOSSIBLEVOLUME);

        sampleSteiner = noiseSample * noiseLevelSteiner;

        sampleLadder = noiseSample * noiseLevelLadder;

        sampleSteiner += subSample * subLevelSteiner;

        sampleLadder += subSample * subLevelLadder;

        sampleSteiner += oscASample * oscALevelSteiner;

        sampleLadder += oscASample * oscALevelLadder;

        sampleSteiner += oscBSample * oscBLevelSteiner;

        sampleLadder += oscBSample * oscBLevelLadder;

        sampleSteiner = sampleSteiner * dampSteiner;

        sampleLadder = sampleLadder * dampLadder;

        sampleSteiner *= MAXVOLUMEPERMODULE;

        sampleLadder *= MAXVOLUMEPERMODULE;

        sampleSteiner *= 8388607.0f;

        sampleLadder *= 8388607.0f;

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
            layerA.noise.out = noiseSample;
            layerA.sub.out = subSample;
            layerA.oscA.out = oscASample;
            layerA.oscB.out = oscBSample;
        }
    }
}

#endif