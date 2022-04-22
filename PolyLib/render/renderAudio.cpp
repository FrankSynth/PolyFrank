#ifdef POLYRENDER

#include "renderAudio.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "render/renderWaveshaper.hpp"
#include "rng.h"
#include <cmath>
#include <mdma.h>

extern Layer layerA;

// std::vector<const WaveTable *> wavetables;

const WaveTable *sourcesA[4] = {nullptr, nullptr, nullptr, nullptr};
const WaveTable *sourcesB[4] = {nullptr, nullptr, nullptr, nullptr};

RAM1 ALIGN_32BYTES(float oscAwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscAwavetable[MAXWAVETABLESPERVOICE] = {{0, oscAwavetableRAM[0], "defaultName"},
                                                  {0, oscAwavetableRAM[1], "defaultName"},
                                                  {0, oscAwavetableRAM[2], "defaultName"},
                                                  {0, oscAwavetableRAM[3], "defaultName"}};

RAM1 ALIGN_32BYTES(float oscBwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscBwavetable[MAXWAVETABLESPERVOICE] = {{0, oscBwavetableRAM[0], "defaultName"},
                                                  {0, oscBwavetableRAM[1], "defaultName"},
                                                  {0, oscBwavetableRAM[2], "defaultName"},
                                                  {0, oscBwavetableRAM[3], "defaultName"}};

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
    // oscAwavetable[position].cycles = wavetable->cycles;
    // oscAwavetable[position].sizePerCycle = wavetable->sizePerCycle;
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
    // oscBwavetable[position].cycles = wavetable->cycles;
    // oscBwavetable[position].sizePerCycle = wavetable->sizePerCycle;
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

    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)wavetable->data, (uint32_t)oscBwavetableRAM[position],
                      oscBwavetable[position].size * 4, 1);
}

/**
 * @brief set and load default wavetables
 *
 */
void loadInitialWavetables() {

    initWavetables();

    switchOscAWavetable(0, wavetables[0]);
    switchOscAWavetable(1, wavetables[0]);
    switchOscAWavetable(2, wavetables[0]);
    switchOscAWavetable(3, wavetables[0]);

    // Osc B
    switchOscBWavetable(0, wavetables[0]);
    switchOscBWavetable(1, wavetables[0]);
    switchOscBWavetable(2, wavetables[0]);
    switchOscBWavetable(3, wavetables[0]);
}

// TODO new bitcrusher von Jacob??
inline vec<VOICESPERCHIP> bitcrush(const vec<VOICESPERCHIP> &bitcrush, const vec<VOICESPERCHIP> &sample) {

    vec<VOICESPERCHIP> mult = 1.0f / max(bitcrush, (1.0f / 8388607.0f));
    vec<VOICESPERCHIP> bitCrushedSample = mult * sample;
    bitCrushedSample = round(bitCrushedSample);
    bitCrushedSample = bitCrushedSample / mult;

    return bitCrushedSample;
}

inline vec<VOICESPERCHIP> getNoiseSample() {

    static vec<VOICESPERCHIP> sample = 0;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;

    vec<VOICESPERCHIP, uint32_t> randomNumber;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber[i]);

    randomNumber = randomNumber & 0x00FFFFFF;
    sampleCrushCount++;

    // map to -1, 1
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (sampleCrushCount[i] > layerA.noise.samplecrusher[i]) {
            sample[i] = ((float)randomNumber[i] / 8388607.0f) - 1.0f;
            sampleCrushCount[i] = 0;
        }

    return sample;
}

inline vec<VOICESPERCHIP> getSubSample() {

    static vec<VOICESPERCHIP> sample = 0;
    static vec<VOICESPERCHIP> phaseDifferenceOscA = 0;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;

    const vec<VOICESPERCHIP> &bitcrusher = layerA.sub.bitcrusher;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.sub.samplecrusher;
    const vec<VOICESPERCHIP> &shape = layerA.sub.shape;

    static vec<VOICESPERCHIP> phase = 0;
    static vec<VOICESPERCHIP> oscApreviousPhase = 0;

    vec<VOICESPERCHIP> newSample;

    vec<VOICESPERCHIP> shapeDiv = 4.0f / max(0.01f, shape);
    phase -= floor(phase);
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

    newSample = bitcrush(bitcrusher, newSample);

    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > samplecrusher;
    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    float phaseLength = 0.5f * !layerA.sub.dOctaveSwitch + 0.25f * layerA.sub.dOctaveSwitch;

    oscApreviousPhase -= (layerA.oscA.phase < oscApreviousPhase);
    phaseDifferenceOscA = layerA.oscA.phase - oscApreviousPhase;
    oscApreviousPhase = layerA.oscA.phase;

    phase += phaseDifferenceOscA * phaseLength;

    return sample;
}

inline vec<VOICESPERCHIP> getOscASample() {

    static vec<VOICESPERCHIP> sample = 0;

    vec<VOICESPERCHIP> &phase = layerA.oscA.phase;
    // vec<VOICESPERCHIP> &phaseWavetableUpper = layerA.oscA.phaseWavetableUpper;
    const vec<VOICESPERCHIP> &morph = layerA.oscA.morph;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscA.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscA.bitcrusher;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscA.samplecrusher;

    vec<VOICESPERCHIP, uint32_t> waveTableSelectionLower = morph;
    vec<VOICESPERCHIP, uint32_t> waveTableSelectionUpper = ceil(morph);

    WaveTable *wavetableLower[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableLower[i] = &oscAwavetable[waveTableSelectionLower[i]];

    WaveTable *wavetableUpper[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableUpper[i] = &oscAwavetable[waveTableSelectionUpper[i]];

    vec<VOICESPERCHIP> stepWavetableLower;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = phase[i] * wavetableLower[i]->size;

    vec<VOICESPERCHIP> stepWavetableUpper;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableUpper[i] = phase[i] * wavetableUpper[i]->size;

    phase += noteStep * PHASEPROGRESSPERRENDER;
    phase -= floor(phase);

    vec<VOICESPERCHIP, uint32_t> positionA = stepWavetableLower;
    vec<VOICESPERCHIP, uint32_t> positionAb = positionA + 1U;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != wavetableLower[i]->size);

    vec<VOICESPERCHIP> sampleA;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscAwavetableRAM[waveTableSelectionLower[i]][positionA[i]];

    vec<VOICESPERCHIP> sampleAb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = oscAwavetableRAM[waveTableSelectionLower[i]][positionAb[i]];

    vec<VOICESPERCHIP> interSampleAPos = stepWavetableLower - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    vec<VOICESPERCHIP, uint32_t> positionB = stepWavetableUpper;
    vec<VOICESPERCHIP, uint32_t> positionBb = positionB + 1U;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionBb[i] * (positionBb[i] != wavetableUpper[i]->size);

    vec<VOICESPERCHIP> sampleB;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscAwavetableRAM[waveTableSelectionUpper[i]][positionB[i]];
    vec<VOICESPERCHIP> sampleBb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = oscAwavetableRAM[waveTableSelectionUpper[i]][positionBb[i]];

    vec<VOICESPERCHIP> interSampleBPos = stepWavetableUpper - positionB;
    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleBPos);

    vec<VOICESPERCHIP> tempMorph = morph - waveTableSelectionLower;
    vec<VOICESPERCHIP> newSample = fast_lerp_f32(sampleA, sampleB, tempMorph);

    newSample = renderWaveshaperSample(newSample);

    newSample = bitcrush(bitcrusher, newSample);

    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > samplecrusher;

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

vec<VOICESPERCHIP> getOscBSample() {

    static vec<VOICESPERCHIP> sample = 0;

    // cache step of osc A, if smaller, new phase has begun.
    const vec<VOICESPERCHIP> &oscAphase = layerA.oscA.phase;

    static vec<VOICESPERCHIP> cacheOscAPhase = 0;
    static vec<VOICESPERCHIP> phase = 0;
    const vec<VOICESPERCHIP> &morph = layerA.oscB.morph;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscB.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscB.bitcrusher;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscB.samplecrusher;
    const vec<VOICESPERCHIP> &phaseoffset = layerA.oscB.phaseoffset;

    vec<VOICESPERCHIP, uint32_t> waveTableSelectionLower = morph;
    vec<VOICESPERCHIP, uint32_t> waveTableSelectionUpper = ceil(morph);

    WaveTable *wavetableLower[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableLower[i] = &oscBwavetable[waveTableSelectionLower[i]];
    WaveTable *wavetableUpper[VOICESPERCHIP];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        wavetableUpper[i] = &oscBwavetable[waveTableSelectionUpper[i]];

    vec<VOICESPERCHIP, bool> syncWavetablesNow = (cacheOscAPhase > oscAphase) && layerA.oscB.dSync;
    phase = phase * !syncWavetablesNow;
    cacheOscAPhase = oscAphase;

    // make sure to be in the right step range

    vec<VOICESPERCHIP> phaseOffsetted = phase + phaseoffset;
    phase += noteStep * PHASEPROGRESSPERRENDER;
    phase -= floor(phase);
    phaseOffsetted -= floor(phaseOffsetted);
    phaseOffsetted += (phaseOffsetted < 0.0f);

    vec<VOICESPERCHIP> stepWavetableLower;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = phaseOffsetted[i] * wavetableLower[i]->size;
    vec<VOICESPERCHIP> stepWavetableUpper;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableUpper[i] = phaseOffsetted[i] * wavetableUpper[i]->size;

    vec<VOICESPERCHIP, uint32_t> positionA = stepWavetableLower;
    vec<VOICESPERCHIP, uint32_t> positionAb = positionA + 1U;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != wavetableLower[i]->size);

    vec<VOICESPERCHIP> sampleA;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscBwavetableRAM[waveTableSelectionLower[i]][positionA[i]];
    vec<VOICESPERCHIP> sampleAb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = oscBwavetableRAM[waveTableSelectionLower[i]][positionAb[i]];

    vec<VOICESPERCHIP> interSampleAPos = stepWavetableLower - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    vec<VOICESPERCHIP, uint32_t> positionB = stepWavetableUpper;
    vec<VOICESPERCHIP, uint32_t> positionBb = positionB + 1U;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionBb[i] = positionBb[i] * (positionBb[i] != wavetableUpper[i]->size);

    vec<VOICESPERCHIP> sampleB;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscBwavetableRAM[waveTableSelectionUpper[i]][positionB[i]];
    vec<VOICESPERCHIP> sampleBb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = oscBwavetableRAM[waveTableSelectionUpper[i]][positionBb[i]];

    vec<VOICESPERCHIP> interSampleBPos = stepWavetableUpper - positionB;
    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleBPos);

    vec<VOICESPERCHIP> tempMorph = morph - waveTableSelectionLower;
    vec<VOICESPERCHIP> newSample = fast_lerp_f32(sampleA, sampleB, tempMorph);

    newSample = renderWaveshaperSample(newSample);

    newSample = bitcrush(bitcrusher, newSample);

    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > samplecrusher;

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 */
void renderAudio(volatile int32_t *renderDest) {

    const vec<VOICESPERCHIP> &noiseLevelLadder = layerA.mixer.noiseLevelLadder;
    const vec<VOICESPERCHIP> &noiseLevelSteiner = layerA.mixer.noiseLevelSteiner;

    const vec<VOICESPERCHIP> &subLevelLadder = layerA.mixer.subLevelLadder;
    const vec<VOICESPERCHIP> &subLevelSteiner = layerA.mixer.subLevelSteiner;

    const vec<VOICESPERCHIP> &oscALevelLadder = layerA.mixer.oscALevelLadder;
    const vec<VOICESPERCHIP> &oscALevelSteiner = layerA.mixer.oscALevelSteiner;

    const vec<VOICESPERCHIP> &oscBLevelLadder = layerA.mixer.oscBLevelLadder;
    const vec<VOICESPERCHIP> &oscBLevelSteiner = layerA.mixer.oscBLevelSteiner;

    vec<VOICESPERCHIP> dampSteiner;
    vec<VOICESPERCHIP> dampLadder;

    vec<VOICESPERCHIP> maxVolSteiner;
    vec<VOICESPERCHIP> maxVolLadder;

    vec<VOICESPERCHIP> sampleSteiner;
    vec<VOICESPERCHIP> sampleLadder;

    int32_t intSampleSteiner[VOICESPERCHIP];
    int32_t intSampleLadder[VOICESPERCHIP];

    for (uint32_t sample = 0; sample < SAIDMABUFFERSIZE; sample++) {

        vec<VOICESPERCHIP> oscASample = getOscASample();
        vec<VOICESPERCHIP> oscBSample = getOscBSample();
        vec<VOICESPERCHIP> noiseSample = getNoiseSample();
        vec<VOICESPERCHIP> subSample = getSubSample();

        maxVolSteiner = noiseLevelSteiner;
        maxVolSteiner += subLevelSteiner;
        maxVolSteiner += oscALevelSteiner;
        maxVolSteiner += oscBLevelSteiner;

        maxVolSteiner = max(maxVolSteiner, 1.0f);
        vec<VOICESPERCHIP, bool> clipSteiner = maxVolSteiner > MAXPOSSIBLEVOLUME;
        dampSteiner = (MAXPOSSIBLEVOLUME / maxVolSteiner) * clipSteiner + (!clipSteiner);

        sampleSteiner = noiseSample * noiseLevelSteiner;
        sampleSteiner += subSample * subLevelSteiner;
        sampleSteiner += oscASample * oscALevelSteiner;
        sampleSteiner += oscBSample * oscBLevelSteiner;
        sampleSteiner *= dampSteiner;
        sampleSteiner *= MAXVOLUMEPERMODULE * 8388607.0f;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            intSampleSteiner[i] = sampleSteiner[i];

        renderDest[sample * AUDIOCHANNELS + 1 * 2] = intSampleSteiner[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2] = intSampleSteiner[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2] = intSampleSteiner[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2] = intSampleSteiner[3];

        maxVolLadder = noiseLevelLadder;
        maxVolLadder += subLevelLadder;
        maxVolLadder += oscALevelLadder;
        maxVolLadder += oscBLevelLadder;

        maxVolLadder = max(maxVolLadder, 1.0f);

        vec<VOICESPERCHIP, bool> clipLadder = maxVolLadder > MAXPOSSIBLEVOLUME;
        dampLadder = (MAXPOSSIBLEVOLUME / maxVolLadder) * (clipLadder) + (!clipLadder);

        sampleLadder = noiseSample * noiseLevelLadder;
        sampleLadder += subSample * subLevelLadder;
        sampleLadder += oscASample * oscALevelLadder;
        sampleLadder += oscBSample * oscBLevelLadder;
        sampleLadder *= dampLadder;
        sampleLadder *= MAXVOLUMEPERMODULE * 8388607.0f;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            intSampleLadder[i] = sampleLadder[i];

        renderDest[sample * AUDIOCHANNELS + 1 * 2 + 1] = intSampleLadder[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2 + 1] = intSampleLadder[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2 + 1] = intSampleLadder[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2 + 1] = intSampleLadder[3];

        if (sample == 0) {
            layerA.noise.out = noiseSample;
            layerA.sub.out = subSample;
            layerA.oscA.out = oscASample;
            layerA.oscB.out = oscBSample;
        }
    }
}

#endif