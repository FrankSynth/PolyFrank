#ifdef POLYRENDER

#include "renderAudio.hpp"
// #include "LowPassFilter.hpp"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "render/renderPhaseshaper.hpp"
#include "render/renderWaveshaper.hpp"
#include "rng.h"
#include <cmath>
#include <mdma.h>

extern Layer layerA;

const WaveTable *sourcesA[4] = {nullptr, nullptr, nullptr, nullptr};
const WaveTable *sourcesB[4] = {nullptr, nullptr, nullptr, nullptr};

RAM1 ALIGN_32BYTES(float oscAwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscAwavetable[MAXWAVETABLESPERVOICE] = {oscAwavetableRAM[0], oscAwavetableRAM[1], oscAwavetableRAM[2],
                                                  oscAwavetableRAM[3]};

RAM1 ALIGN_32BYTES(float oscBwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH]);
WaveTable oscBwavetable[MAXWAVETABLESPERVOICE] = {oscBwavetableRAM[0], oscBwavetableRAM[1], oscBwavetableRAM[2],
                                                  oscBwavetableRAM[3]};

RAM1 float subwavetableRAM[2][MAXWAVETABLELENGTH];
WaveTable subOscWavetable[2] = {subwavetableRAM[0], subwavetableRAM[1]};

/**
 * @brief switch OscA Wavetable at position x
 *
 * @param position position between 0 and WAVETABLESPERVOICE
 * @param wavetable pointer to new wavetable
 */
void switchOscAWavetable(uint32_t position, const WaveTable *wavetable) {

    if (wavetable == sourcesA[position])
        return;

    sourcesA[position] = wavetable;

    MDMA_HandleTypeDef *mdmaHandle = nullptr;

    switch (position) {
        case 0: mdmaHandle = &hmdma_mdma_channel41_sw_0; break;
        case 1: mdmaHandle = &hmdma_mdma_channel42_sw_0; break;
        case 2: mdmaHandle = &hmdma_mdma_channel43_sw_0; break;
        case 3: mdmaHandle = &hmdma_mdma_channel44_sw_0; break;

        default: PolyError_Handler("renderAudio | switchOscAWavetable | illegal position"); break;
    }
    while (mdmaHandle->State != HAL_MDMA_STATE_READY)
        ;

    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)(wavetable->data), (uint32_t)oscAwavetableRAM[position],
                      WaveTable::size * 4, 1);
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

    sourcesB[position] = wavetable;

    MDMA_HandleTypeDef *mdmaHandle = nullptr;

    switch (position) {
        case 0: mdmaHandle = &hmdma_mdma_channel45_sw_0; break;
        case 1: mdmaHandle = &hmdma_mdma_channel46_sw_0; break;
        case 2: mdmaHandle = &hmdma_mdma_channel47_sw_0; break;
        case 3: mdmaHandle = &hmdma_mdma_channel48_sw_0; break;

        default: PolyError_Handler("renderAudio | switchOscBWavetable | illegal position"); break;
    }
    while (mdmaHandle->State != HAL_MDMA_STATE_READY)
        ;
    HAL_MDMA_Start_IT(mdmaHandle, (uint32_t)(wavetable->data), (uint32_t)oscBwavetableRAM[position],
                      WaveTable::size * 4, 1);
}

void loadSubWavetables() {
    fast_copy_f32((uint32_t *)wavetable_Square.data, (uint32_t *)subOscWavetable[0].data, WaveTable::size);
    fast_copy_f32((uint32_t *)wavetable_Triangle.data, (uint32_t *)subOscWavetable[1].data, WaveTable::size);
}

/**
 * @brief set and load default wavetables
 *
 */
void loadInitialWavetables() {

    initWavetables();

    loadSubWavetables();

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

inline vec<VOICESPERCHIP> bitcrush(const vec<VOICESPERCHIP> &bitcrush, const vec<VOICESPERCHIP> &bitcrushInv,
                                   const vec<VOICESPERCHIP> &sample) {

    // vec<VOICESPERCHIP> mult = 1.0f / max(bitcrush, (1.0f / 8388607.0f));
    vec<VOICESPERCHIP> bitCrushedSample = bitcrushInv * sample;
    bitCrushedSample = round(bitCrushedSample);
    bitCrushedSample = bitCrushedSample * bitcrush;

    return bitCrushedSample;
}

inline vec<VOICESPERCHIP> getNoiseSample() {

    static vec<VOICESPERCHIP> sample;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;

    vec<VOICESPERCHIP, uint32_t> randomNumber;

    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber[i]);
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        randomNumber[i] = hrng.Instance->DR;

    vec<VOICESPERCHIP> randomNumberFlt = randomNumber & 0x00FFFFFF;
    // sampleCrushCount++;

    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > (layerA.noise.samplecrusher * 960.f);
    vec<VOICESPERCHIP> NotsampleCrushNow = !sampleCrushNow;

    sampleCrushCount *= NotsampleCrushNow;

    sample = (((randomNumberFlt / 8388607.0f) - 1.0f) * sampleCrushNow) + (sample * NotsampleCrushNow);

    // map to -1, 1
    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     if (sampleCrushCount[i] > (layerA.noise.samplecrusher[i]) * 960.f) {
    //         sample[i] = ((float)randomNumber[i] / 8388607.0f) - 1.0f;
    //         sampleCrushCount[i] = 0;
    //     }

    // sample = ((vec<VOICESPERCHIP>)randomNumber / 8388607.0f) - 1.0f;

    return sample;
}

inline vec<VOICESPERCHIP> getSubSample() {

    static vec<VOICESPERCHIP> sample;
    static vec<VOICESPERCHIP> phaseDifferenceOscA;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;

    // const vec<VOICESPERCHIP> &bitcrusher = layerA.oscA.bitcrusher;
    // const vec<VOICESPERCHIP> &samplecrusher = layerA.oscA.samplecrusher;
    const vec<VOICESPERCHIP> &shape = layerA.oscA.shapeSub;
    const vec<VOICESPERCHIP, uint32_t> &subWavetable = layerA.oscA.subWavetable;

    static vec<VOICESPERCHIP> phase;
    static vec<VOICESPERCHIP> oscApreviousPhase;

    const float &phaseLength = layerA.oscA.phaseLengthSub;

    oscApreviousPhase -= (layerA.oscA.phase < oscApreviousPhase);
    phaseDifferenceOscA = layerA.oscA.phase - oscApreviousPhase;
    oscApreviousPhase = layerA.oscA.phase;

    phase = phase + phaseDifferenceOscA * phaseLength;
    phase -= floor(phase);

    vec<VOICESPERCHIP> stepWavetable;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetable[i] = phase[i] * WaveTable::subSize[subWavetable[i]];

    vec<VOICESPERCHIP, uint32_t> positionA = stepWavetable;
    vec<VOICESPERCHIP, uint32_t> positionAb = positionA + 1U;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != WaveTable::subSize[subWavetable[i]]);

    vec<VOICESPERCHIP> sampleA;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = subOscWavetable[0].subData[subWavetable[i]][positionA[i]];
    vec<VOICESPERCHIP> sampleAb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = subOscWavetable[0].subData[subWavetable[i]][positionAb[i]];

    vec<VOICESPERCHIP> interSampleAPos = stepWavetable - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    vec<VOICESPERCHIP> sampleB;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = subOscWavetable[1].subData[subWavetable[i]][positionA[i]];
    vec<VOICESPERCHIP> sampleBb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = subOscWavetable[1].subData[subWavetable[i]][positionAb[i]];

    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleAPos);

    vec<VOICESPERCHIP> newSample = fast_lerp_f32(sampleA, sampleB, shape);
    // static vec<VOICESPERCHIP> prevNewSample = 0;
    // vec<VOICESPERCHIP> prevDerivSample1 = 0;

    // vec<VOICESPERCHIP> derivCorrection = 1.0f / phaseStep;

    // vec<VOICESPERCHIP> derivSample1 = (newSample - prevNewSample) * derivCorrection;

    // prevNewSample = newSample;
    return newSample;
}

inline vec<VOICESPERCHIP> getOscASample() {

    vec<VOICESPERCHIP> &phase = layerA.oscA.phase;
    // vec<VOICESPERCHIP> &phaseWavetableUpper = layerA.oscA.phaseWavetableUpper;
    // const vec<VOICESPERCHIP> &morph = layerA.oscA.morph;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscA.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscA.bitcrusher;
    const vec<VOICESPERCHIP> &bitcrusherInv = layerA.oscA.bitcrusherInv;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscA.samplecrusher;
    const vec<VOICESPERCHIP> &morphFract = layerA.oscA.morphFract;
    const vec<VOICESPERCHIP, uint32_t> &subWavetable = layerA.oscA.subWavetable;

    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionLower = layerA.oscA.waveTableSelectionLower;
    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionUpper = layerA.oscA.waveTableSelectionUpper;

    phase += noteStep * PHASEPROGRESSPERRENDER;
    phase -= floor(phase);

    vec<VOICESPERCHIP> shapedPhase = renderPhaseshaperSample(phase, layerA.phaseshaperA);
    // vec<VOICESPERCHIP> phaseStep = min(fabs(shapedPhase - prevPhase), fabs(shapedPhase - prevPhase - 1.0f));
    // prevPhase = shapedPhase;

    vec<VOICESPERCHIP> stepWavetableLower;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = shapedPhase[i] * WaveTable::subSize[subWavetable[i]];

    vec<VOICESPERCHIP, uint32_t> positionA = stepWavetableLower;
    vec<VOICESPERCHIP, uint32_t> positionAb = positionA + 1U;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != WaveTable::subSize[subWavetable[i]]);

    vec<VOICESPERCHIP> sampleA;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscAwavetable[waveTableSelectionLower[i]].subData[subWavetable[i]][positionA[i]];

    vec<VOICESPERCHIP> sampleAb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = oscAwavetable[waveTableSelectionLower[i]].subData[subWavetable[i]][positionAb[i]];

    vec<VOICESPERCHIP> interSampleAPos = stepWavetableLower - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    vec<VOICESPERCHIP> sampleB;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscAwavetable[waveTableSelectionUpper[i]].subData[subWavetable[i]][positionA[i]];
    vec<VOICESPERCHIP> sampleBb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = oscAwavetable[waveTableSelectionUpper[i]].subData[subWavetable[i]][positionAb[i]];

    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleAPos);

    vec<VOICESPERCHIP> newSample = fast_lerp_f32(sampleA, sampleB, morphFract);

    newSample = renderWaveshaperSample(newSample, layerA.waveshaperA); // TODO WAVETABLES: <1 not allowed!!

    newSample = bitcrush(bitcrusher, bitcrusherInv, newSample);

    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > (samplecrusher * 960.f);
    vec<VOICESPERCHIP> NotsampleCrushNow = !sampleCrushNow;

    sampleCrushCount *= NotsampleCrushNow;

    static vec<VOICESPERCHIP> sample = 0;
    sample = (newSample * sampleCrushNow) + (sample * NotsampleCrushNow);

    return sample;
}

vec<VOICESPERCHIP> getOscBSample() {

    static vec<VOICESPERCHIP> sample;

    // cache step of osc A, if smaller, new phase has begun.
    const vec<VOICESPERCHIP> &oscAphase = layerA.oscA.phase;

    static vec<VOICESPERCHIP> cacheOscAPhase;
    static vec<VOICESPERCHIP> phase;
    // const vec<VOICESPERCHIP> &morph = layerA.oscB.morph;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscB.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscB.bitcrusher;
    const vec<VOICESPERCHIP> &bitcrusherInv = layerA.oscB.bitcrusherInv;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscB.samplecrusher;
    const vec<VOICESPERCHIP> &phaseoffset = layerA.oscB.phaseoffset;
    const vec<VOICESPERCHIP> &morphFract = layerA.oscB.morphFract;
    const vec<VOICESPERCHIP, uint32_t> &subWavetable = layerA.oscB.subWavetable;

    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionLower = layerA.oscB.waveTableSelectionLower;
    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionUpper = layerA.oscB.waveTableSelectionUpper;

    vec<VOICESPERCHIP, bool> syncWavetablesNow = (cacheOscAPhase > oscAphase) && layerA.oscB.dSync;
    phase = phase * !syncWavetablesNow;
    cacheOscAPhase = oscAphase;

    phase += noteStep * PHASEPROGRESSPERRENDER;
    phase -= floor(phase);

    vec<VOICESPERCHIP> phaseOffsetted = phase + phaseoffset + 1.0f;
    phaseOffsetted -= floor(phaseOffsetted);

    vec<VOICESPERCHIP> shapedPhase = renderPhaseshaperSample(phaseOffsetted, layerA.phaseshaperB);

    vec<VOICESPERCHIP> stepWavetableLower;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = shapedPhase[i] * WaveTable::subSize[subWavetable[i]];

    vec<VOICESPERCHIP, uint32_t> positionA = stepWavetableLower;
    vec<VOICESPERCHIP, uint32_t> positionAb = positionA + 1U;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionAb[i] = positionAb[i] * (positionAb[i] != WaveTable::subSize[subWavetable[i]]);

    vec<VOICESPERCHIP> sampleA;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscBwavetable[waveTableSelectionLower[i]].subData[subWavetable[i]][positionA[i]];
    vec<VOICESPERCHIP> sampleAb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleAb[i] = oscBwavetable[waveTableSelectionLower[i]].subData[subWavetable[i]][positionAb[i]];

    vec<VOICESPERCHIP> interSampleAPos = stepWavetableLower - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    vec<VOICESPERCHIP> sampleB;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscBwavetable[waveTableSelectionUpper[i]].subData[subWavetable[i]][positionA[i]];
    vec<VOICESPERCHIP> sampleBb;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleBb[i] = oscBwavetable[waveTableSelectionUpper[i]].subData[subWavetable[i]][positionAb[i]];

    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleAPos);

    vec<VOICESPERCHIP> newSample = fast_lerp_f32(sampleA, sampleB, morphFract);

    newSample = renderWaveshaperSample(newSample, layerA.waveshaperB); // TODO WAVETABLES: values>1 not allowed!!

    newSample = bitcrush(bitcrusher, bitcrusherInv, newSample);

    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > (samplecrusher * 960.f);
    vec<VOICESPERCHIP> NotsampleCrushNow = !sampleCrushNow;

    sampleCrushCount *= NotsampleCrushNow;
    sample = (newSample * sampleCrushNow) + (sample * NotsampleCrushNow);

    return sample;
}

// https://www.desmos.com/calculator/38pwnjn7ci?lang=de
// x1 = threshold
// x2 = maxloudness
// q = threshold (x1)
// p = 1
// a = 0

// possible combinations threshold = 0.5, maxloudness = 4 n=7       careful, go check for d!
// possible combinations threshold = 0.6, maxloudness = 3 n=6       careful, go check for d!
// possible combinations threshold = 0.7, maxloudness = 4 n=11      careful, go check for d!
// possible combinations threshold = 0.8, maxloudness = 4 n=16      careful, go check for d!
// possible combinations threshold = 0.8, maxloudness = 3 n=11      careful, go check for d!
// possible combinations threshold = 0.9, maxloudness = 4 (high n!) careful, go check for d!
inline float softLimit(float inputSample) {

    const float threshold = 0.8f;
    const float maxVal = 3.0f;

    // const uint32_t n = -(maxVal - threshold) / (threshold - 1.0f); // careful, with lower threshold no int but float
    // const float d = (threshold - 1.0f) / std::pow(maxVal - threshold, n);

    ///////////////////////////////
    // currently for threshold = 0.8, maxVal = 3.0
    // const uint32_t n = 11;
    const float d = -0.0000342279198712f;
    ///////////////////////////////

    // float sign = getSign(inputSample);
    float absInput = fabs(inputSample);

    if (absInput <= threshold)
        return inputSample;
    // if (absInput >= maxVal)
    //     return sign;

    float sub = maxVal - absInput;

    sub = sub * sub * sub * sub * sub * sub * sub * sub * sub * sub * sub;

    float sample = d * sub + 1.0f;
    return std::min(sample, 1.0f) * getSign(inputSample);
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

        vec<VOICESPERCHIP> sampleSteiner;
        vec<VOICESPERCHIP> sampleLadder;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            sampleSteiner[i] = noiseSample[i] * layerA.mixer.noiseLevelSteiner[i];
            sampleLadder[i] = noiseSample[i] * layerA.mixer.noiseLevelLadder[i];
            sampleSteiner[i] += subSample[i] * layerA.mixer.subLevelSteiner[i];
            sampleLadder[i] += subSample[i] * layerA.mixer.subLevelLadder[i];
            sampleSteiner[i] += oscASample[i] * layerA.mixer.oscALevelSteiner[i];
            sampleLadder[i] += oscASample[i] * layerA.mixer.oscALevelLadder[i];
            sampleSteiner[i] += oscBSample[i] * layerA.mixer.oscBLevelSteiner[i];
            sampleLadder[i] += oscBSample[i] * layerA.mixer.oscBLevelLadder[i];
        }

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] = softLimit(sampleSteiner[i]);

        sampleSteiner *= 8388607.0f;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] = softLimit(sampleLadder[i]);

        sampleLadder *= 8388607.0f;

        vec<VOICESPERCHIP, int32_t> intSampleSteiner = sampleSteiner;
        vec<VOICESPERCHIP, int32_t> intSampleLadder = sampleLadder;

        renderDest[sample * AUDIOCHANNELS + 1 * 2] = intSampleLadder[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2] = intSampleLadder[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2] = intSampleLadder[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2] = intSampleLadder[3];

        renderDest[sample * AUDIOCHANNELS + 1 * 2 + 1] = intSampleSteiner[0];
        renderDest[sample * AUDIOCHANNELS + 0 * 2 + 1] = intSampleSteiner[1];
        renderDest[sample * AUDIOCHANNELS + 3 * 2 + 1] = intSampleSteiner[2];
        renderDest[sample * AUDIOCHANNELS + 2 * 2 + 1] = intSampleSteiner[3];
    }

    layerA.noise.out = noiseSample;
    layerA.oscA.outSub = subSample;
    layerA.oscA.out = oscASample;
    layerA.oscB.out = oscBSample;
}

#endif