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

    static vec<VOICESPERCHIP> sample;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;

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

    static vec<VOICESPERCHIP> sample;
    static vec<VOICESPERCHIP> phaseDifferenceOscA;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;

    const vec<VOICESPERCHIP> &bitcrusher = layerA.sub.bitcrusher;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.sub.samplecrusher;
    const vec<VOICESPERCHIP> &shape = layerA.sub.shape;

    static vec<VOICESPERCHIP> phase;
    static vec<VOICESPERCHIP> oscApreviousPhase;

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

    static vec<VOICESPERCHIP> sample;

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

    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > samplecrusher;

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

vec<VOICESPERCHIP> getOscBSample() {

    static vec<VOICESPERCHIP> sample;

    // cache step of osc A, if smaller, new phase has begun.
    const vec<VOICESPERCHIP> &oscAphase = layerA.oscA.phase;

    static vec<VOICESPERCHIP> cacheOscAPhase;
    static vec<VOICESPERCHIP> phase;
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

// https://www.desmos.com/calculator/38pwnjn7ci?lang=de
// x1 = threshold
// x2 = maxloudness
// q = threshold (x1)
// p = 1
// a = 0

// possible combinations threshold = 0.7, maxloudness = 4
// possible combinations threshold = 0.8, maxloudness = 4
// possible combinations threshold = 0.9, maxloudness = 4 (high n!)
// possible combinations threshold = 0.6, maxloudness = 3
// possible combinations threshold = 0.5, maxloudness = 4 (high n!)
inline float softLimit(const float &inputSample) {
    const float threshold = 0.8f;

    const float maxVal = 4.0f;

    const uint32_t n = -(maxVal - threshold) / (threshold - 1.0f); // careful, with lower threshold no int but float
    const float d = (threshold - 1.0f) / std::pow(maxVal - threshold, n);

    float sign = getSign(inputSample);
    float absInput = inputSample * sign;

    if (absInput <= threshold)
        return inputSample;

    float sample = d * powf(threshold - absInput, n) + 1.0f;

    return std::clamp(sample * sign, -1.0f, 1.0f);
}

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 */
void renderAudio(volatile int32_t *renderDest) {

    vec<VOICESPERCHIP> oscASample;
    vec<VOICESPERCHIP> oscBSample;
    vec<VOICESPERCHIP> noiseSample;
    vec<VOICESPERCHIP> subSample;

    for (uint32_t sample = 0; sample < SAIDMABUFFERSIZE; sample++) {

        oscASample = getOscASample();
        oscBSample = getOscBSample();
        noiseSample = getNoiseSample();
        subSample = getSubSample();

        vec<VOICESPERCHIP> sampleSteiner = noiseSample * layerA.mixer.noiseLevelSteiner;
        sampleSteiner += subSample * layerA.mixer.subLevelSteiner;
        sampleSteiner += oscASample * layerA.mixer.oscALevelSteiner;
        sampleSteiner += oscBSample * layerA.mixer.oscBLevelSteiner;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] = softLimit(sampleSteiner[i]);

        sampleSteiner *= 8388607.0f;

        vec<VOICESPERCHIP, int32_t> intSampleSteiner = sampleSteiner;

        renderDest[sample * AUDIOCHANNELS + 1 * 2] = intSampleSteiner[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2] = intSampleSteiner[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2] = intSampleSteiner[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2] = intSampleSteiner[3];

        vec<VOICESPERCHIP> sampleLadder = noiseSample * layerA.mixer.noiseLevelLadder;
        sampleLadder += subSample * layerA.mixer.subLevelLadder;
        sampleLadder += oscASample * layerA.mixer.oscALevelLadder;
        sampleLadder += oscBSample * layerA.mixer.oscBLevelLadder;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] = softLimit(sampleLadder[i]);

        sampleLadder *= 8388607.0f;

        vec<VOICESPERCHIP, int32_t> intSampleLadder = sampleLadder;

        renderDest[sample * AUDIOCHANNELS + 1 * 2 + 1] = intSampleLadder[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2 + 1] = intSampleLadder[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2 + 1] = intSampleLadder[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2 + 1] = intSampleLadder[3];
    }

    layerA.noise.out = noiseSample;
    layerA.sub.out = subSample;
    layerA.oscA.out = oscASample;
    layerA.oscB.out = oscBSample;
}

#endif