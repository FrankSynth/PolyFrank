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
RAM1 float ripplewavetableRAM[MAXWAVETABLELENGTH];

WaveTable subOscWavetable[2] = {subwavetableRAM[0], subwavetableRAM[1]};

WaveTable rippleOscAWavetable = {ripplewavetableRAM};

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

void loadRippleWavetable() {
    fast_copy_f32((uint32_t *)wavetable_Sine.data, (uint32_t *)rippleOscAWavetable.data, WaveTable::size);
}

/**
 * @brief set and load default wavetables
 *
 */
void loadInitialWavetables() {

    initWavetables();

    loadSubWavetables();
    loadRippleWavetable();

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

inline void bitcrush(const vec<VOICESPERCHIP> &bitcrush, const vec<VOICESPERCHIP> &bitcrushInv,
                     vec<VOICESPERCHIP> &sample) {
    sample = bitcrushInv * sample;
    sample = round(sample);
    sample = sample * bitcrush;
}

inline vec<VOICESPERCHIP> getNoiseSample() {

    static vec<VOICESPERCHIP> sample;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;

    vec<VOICESPERCHIP> randomNumber;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        randomNumber[i] = float(hrng.Instance->DR & 0x00FFFFFF);

    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > (layerA.noise.samplecrusher * 960.f);

    sampleCrushCount *= !sampleCrushNow;
    sample = ((randomNumber * 1.192093038e-07F - 1.0f) * sampleCrushNow) + (sample * !sampleCrushNow);

    return sample;
}

inline vec<VOICESPERCHIP> getSubSample() {

    static vec<VOICESPERCHIP> sample;
    static vec<VOICESPERCHIP> phaseDifferenceOscA;
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount;

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
    vec<VOICESPERCHIP, uint32_t> positionA;
    vec<VOICESPERCHIP, uint32_t> positionAb;
    vec<VOICESPERCHIP> sampleA;
    vec<VOICESPERCHIP> sampleAb;
    vec<VOICESPERCHIP> interSamplePos;
    vec<VOICESPERCHIP> sampleB;
    vec<VOICESPERCHIP> sampleBb;
    vec<VOICESPERCHIP> newSample;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetable[i] = phase[i] * WaveTable::subSize[subWavetable[i]];

    positionA = stepWavetable;
    positionAb = positionA + 1U;
    interSamplePos = stepWavetable - positionA;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        uint32_t subWavetableIndex = subWavetable[i];

        positionAb[i] = positionAb[i] * (positionAb[i] != WaveTable::subSize[subWavetableIndex]);
        sampleA[i] = subOscWavetable[0].subData[subWavetableIndex][positionA[i]];
        sampleAb[i] = subOscWavetable[0].subData[subWavetableIndex][positionAb[i]];
        sampleB[i] = subOscWavetable[1].subData[subWavetableIndex][positionA[i]];
        sampleBb[i] = subOscWavetable[1].subData[subWavetableIndex][positionAb[i]];
    }

    newSample = faster_CrossLerp_f32(sampleA, sampleAb, sampleB, sampleBb, interSamplePos, shape);

    // sampleA = faster_lerp_f32(sampleA, sampleAb, interSamplePos);
    // sampleB = faster_lerp_f32(sampleB, sampleBb, interSamplePos);
    // newSample = faster_lerp_f32(sampleA, sampleB, shape);

    return newSample;
}

inline void getRippleSample(vec<VOICESPERCHIP> &sampleRipple, vec<VOICESPERCHIP> &newPhase) {
    static vec<VOICESPERCHIP> ripplePhase = 0.0f;
    static vec<VOICESPERCHIP> rippleDamp = 0.0f;
    static vec<VOICESPERCHIP> oldPhase;

    // Phase
    vec<VOICESPERCHIP> phaseProgress = newPhase - oldPhase;

    if (phaseProgress < 0.0f) { // negative value -> new Phase -> reset
        rippleDamp = 1.0f;
        ripplePhase = 0.0f;
        phaseProgress = newPhase; // remove sign
    }
    oldPhase = newPhase;

    // calculat Ripple progress
    ripplePhase = ripplePhase + phaseProgress * layerA.oscA.RippleRatio;

    rippleDamp = rippleDamp - phaseProgress * layerA.oscA.RippleDamp;

    // limit Ripple
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        rippleDamp[i] = fmaxf(rippleDamp[i], 0.0f);
        ripplePhase[i] = ripplePhase[i] - floorf(ripplePhase[i]);
    }

    // sample Sinus Wave
    vec<VOICESPERCHIP> stepWavetable;
    vec<VOICESPERCHIP, uint32_t> positionA;
    vec<VOICESPERCHIP, uint32_t> positionB;
    vec<VOICESPERCHIP> interSamplePos;

    // sample index

    stepWavetable = ripplePhase * MAXSUBWAVETABLELENGTH;
    positionA = stepWavetable;
    positionB = positionA + 1U;

    // interpolation
    interSamplePos = stepWavetable - positionA;

    vec<VOICESPERCHIP> sampleA;
    vec<VOICESPERCHIP> sampleAb;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        positionB[i] = positionB[i] * (positionB[i] != MAXSUBWAVETABLELENGTH);
        sampleA[i] = rippleOscAWavetable.subData[0][positionA[i]];
        sampleAb[i] = rippleOscAWavetable.subData[0][positionB[i]];
    }

    sampleRipple += faster_lerp_f32(sampleA, sampleAb, interSamplePos) * rippleDamp * layerA.oscA.RippleAmount;
}

inline void getWavetableSampleOSCA(vec<VOICESPERCHIP> &sample, vec<VOICESPERCHIP> &phase) {

    const vec<VOICESPERCHIP, uint32_t> &subWavetable = layerA.oscA.subWavetable;
    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionLower = layerA.oscA.waveTableSelectionLower;
    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionUpper = layerA.oscA.waveTableSelectionUpper;
    const vec<VOICESPERCHIP> &morphFract = layerA.oscA.morphFract;

    vec<VOICESPERCHIP> stepWavetableLower;

    vec<VOICESPERCHIP, uint32_t> positionA;
    vec<VOICESPERCHIP, uint32_t> positionAb;
    vec<VOICESPERCHIP> sampleA;
    vec<VOICESPERCHIP> sampleAb;
    vec<VOICESPERCHIP> interSamplePos;
    vec<VOICESPERCHIP> sampleB;
    vec<VOICESPERCHIP> sampleBb;

    uint32_t i;

    for (i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = phase[i] * WaveTable::subSize[subWavetable[i]];

    positionA = stepWavetableLower;
    positionAb = positionA + 1U;

    interSamplePos = stepWavetableLower - positionA;

    for (i = 0; i < VOICESPERCHIP; i++) {
        uint32_t subWavetableIndex = subWavetable[i];

        positionAb[i] = positionAb[i] * (positionAb[i] != WaveTable::subSize[subWavetableIndex]);

        sampleA[i] = oscAwavetable[waveTableSelectionLower[i]].subData[subWavetableIndex][positionA[i]];
        sampleAb[i] = oscAwavetable[waveTableSelectionLower[i]].subData[subWavetableIndex][positionAb[i]];

        sampleB[i] = oscAwavetable[waveTableSelectionUpper[i]].subData[subWavetableIndex][positionA[i]];
        sampleBb[i] = oscAwavetable[waveTableSelectionUpper[i]].subData[subWavetableIndex][positionAb[i]];
    }

    sample = faster_CrossLerp_f32(sampleA, sampleAb, sampleB, sampleBb, interSamplePos, morphFract);

    // sampleA = faster_lerp_f32(sampleA, sampleAb, interSamplePos);
    // sampleB = faster_lerp_f32(sampleB, sampleBb, interSamplePos);
    // sample = faster_lerp_f32(sampleA, sampleB, morphFract);
}

inline void getWavetableSampleOSCB(vec<VOICESPERCHIP> &sample, vec<VOICESPERCHIP> &phase) {

    const vec<VOICESPERCHIP, uint32_t> &subWavetable = layerA.oscB.subWavetable;
    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionLower = layerA.oscB.waveTableSelectionLower;
    const vec<VOICESPERCHIP, uint32_t> &waveTableSelectionUpper = layerA.oscB.waveTableSelectionUpper;
    const vec<VOICESPERCHIP> &morphFract = layerA.oscB.morphFract;

    vec<VOICESPERCHIP> stepWavetableLower;

    vec<VOICESPERCHIP, uint32_t> positionA;
    vec<VOICESPERCHIP, uint32_t> positionAb;
    vec<VOICESPERCHIP> sampleA;
    vec<VOICESPERCHIP> sampleAb;
    vec<VOICESPERCHIP> interSamplePos;
    vec<VOICESPERCHIP> sampleB;
    vec<VOICESPERCHIP> sampleBb;

    uint32_t i;

    for (i = 0; i < VOICESPERCHIP; i++)
        stepWavetableLower[i] = phase[i] * WaveTable::subSize[subWavetable[i]];

    positionA = stepWavetableLower;
    positionAb = positionA + 1U;

    interSamplePos = stepWavetableLower - positionA;

    for (i = 0; i < VOICESPERCHIP; i++) {
        uint32_t subWavetableIndex = subWavetable[i];

        positionAb[i] = positionAb[i] * (positionAb[i] != WaveTable::subSize[subWavetableIndex]);

        sampleA[i] = oscBwavetable[waveTableSelectionLower[i]].subData[subWavetableIndex][positionA[i]];
        sampleAb[i] = oscBwavetable[waveTableSelectionLower[i]].subData[subWavetableIndex][positionAb[i]];

        sampleB[i] = oscBwavetable[waveTableSelectionUpper[i]].subData[subWavetableIndex][positionA[i]];
        sampleBb[i] = oscBwavetable[waveTableSelectionUpper[i]].subData[subWavetableIndex][positionAb[i]];
    }

    sample = faster_CrossLerp_f32(sampleA, sampleAb, sampleB, sampleBb, interSamplePos, morphFract);

    // sampleA = faster_lerp_f32(sampleA, sampleAb, interSamplePos);
    // sampleB = faster_lerp_f32(sampleB, sampleBb, interSamplePos);
    // sample = faster_lerp_f32(sampleA, sampleB, morphFract);
}

inline vec<VOICESPERCHIP> getOscASample() {
    static vec<VOICESPERCHIP> sample;

    vec<VOICESPERCHIP> newSample;
    vec<VOICESPERCHIP> sampleRipple;
    vec<VOICESPERCHIP> shapedPhase;

    vec<VOICESPERCHIP> &phase = layerA.oscA.phase;
    const vec<VOICESPERCHIP> &noteStep = layerA.oscA.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscA.bitcrusher;
    const vec<VOICESPERCHIP> &bitcrusherInv = layerA.oscA.bitcrusherInv;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscA.samplecrusher;

    // calculate phase progress
    phase += noteStep * PHASEPROGRESSPERRENDER;
    phase -= floor(phase);

    // Phaseshaper Effect
    shapedPhase = renderPhaseshaperSample(phase, layerA.phaseshaperA);

    // get OSCA Sample
    getWavetableSampleOSCA(newSample, shapedPhase);

    // Waveshaper Effect
    renderWaveshaperSample(newSample, layerA.waveshaperA); //  WAVETABLES: <1 not allowed!!

    // Ripple Effect
    getRippleSample(newSample, phase);

    // BitCrush Effect
    bitcrush(bitcrusher, bitcrusherInv, newSample);

    // SampleCrush Effect
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > (samplecrusher * 960.f);
    vec<VOICESPERCHIP> NotsampleCrushNow = !sampleCrushNow;

    sampleCrushCount *= NotsampleCrushNow;

    sample = (newSample * sampleCrushNow) + (sample * NotsampleCrushNow);

    // sum sample

    return sample;
}

vec<VOICESPERCHIP> getOscBSample() {

    static vec<VOICESPERCHIP> sample;
    vec<VOICESPERCHIP> newSample;

    vec<VOICESPERCHIP> shapedPhase;

    const vec<VOICESPERCHIP> &oscAphase = layerA.oscA.phase;
    static vec<VOICESPERCHIP> cacheOscAPhase;

    static vec<VOICESPERCHIP> phase;
    const vec<VOICESPERCHIP> &phaseoffset = layerA.oscB.phaseoffset;

    const vec<VOICESPERCHIP> &noteStep = layerA.oscB.note;
    const vec<VOICESPERCHIP> &bitcrusher = layerA.oscB.bitcrusher;
    const vec<VOICESPERCHIP> &bitcrusherInv = layerA.oscB.bitcrusherInv;
    const vec<VOICESPERCHIP> &samplecrusher = layerA.oscB.samplecrusher;

    // calculate phase progress
    vec<VOICESPERCHIP, bool> syncWavetablesNow = (cacheOscAPhase > oscAphase) && layerA.oscB.dSync;
    phase = phase * !syncWavetablesNow;
    cacheOscAPhase = oscAphase;

    phase += noteStep * PHASEPROGRESSPERRENDER;
    phase -= floor(phase);

    vec<VOICESPERCHIP> phaseOffsetted = phase + phaseoffset + 1.0f;
    phaseOffsetted -= floor(phaseOffsetted);

    // Phaseshaper Effect
    shapedPhase = renderPhaseshaperSample(phaseOffsetted, layerA.phaseshaperB);

    // get oscB Sample
    getWavetableSampleOSCB(newSample, shapedPhase);

    // Waveshaper Effect
    renderWaveshaperSample(newSample, layerA.waveshaperB); //  WAVETABLES: <1 not allowed!!

    // BitCrush Effect
    bitcrush(bitcrusher, bitcrusherInv, newSample);

    // SampleCrush Effect
    static vec<VOICESPERCHIP, uint32_t> sampleCrushCount = 0;
    vec<VOICESPERCHIP, bool> sampleCrushNow = (++sampleCrushCount) > (samplecrusher * 960.f);
    vec<VOICESPERCHIP> NotsampleCrushNow = !sampleCrushNow;

    sampleCrushCount *= NotsampleCrushNow;
    sample = (newSample * sampleCrushNow) + (sample * NotsampleCrushNow);

    // sum sample
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
inline float softLimit(float &inputSample) {

    const float threshold = 0.8f;
    const float maxVal = 3.0f;

    // const uint32_t n = -(maxVal - threshold) / (threshold - 1.0f); // careful, with lower threshold no int but
    // float const float d = (threshold - 1.0f) / std::pow(maxVal - threshold, n);

    ///////////////////////////////
    // currently for threshold = 0.8, maxVal = 3.0
    // const uint32_t n = 11;
    const float d = -0.0000342279198712f;
    ///////////////////////////////

    // float sign = getSign(inputSample);
    float absInput = fabsf(inputSample);

    if (absInput <= threshold)
        return inputSample;

    float sub = maxVal - absInput;

    sub = sub * sub * sub * sub * sub * sub * sub * sub * sub * sub * sub;

    float sample = d * sub + 1.0f;
    return fminf(sample, 1.0f) * getSign(inputSample);
}

// float gain = 0.001023217638470900374761396456335660332f;

// float sos[2][3] = {{1.f, 2.f, 1.f},
//                    {1.f, -1.907501626046076426135300607711542397738f, 0.911594496599959946969704560615355148911f}};
// vec<VOICESPERCHIP> bufferX[2];
// vec<VOICESPERCHIP> bufferY[2];

// inline void biquad(vec<VOICESPERCHIP> &input, vec<VOICESPERCHIP> &output) {

//     input *= gain;

//     output = (sos[0][0] * input) + (sos[0][1] * bufferX[0]) + (sos[0][2] * bufferX[1]) + (sos[1][1] * bufferY[0]) +
//              (sos[1][2] * bufferY[1]);

//     bufferX[1] = bufferX[0];
//     bufferX[0] = input;

//     bufferY[1] = bufferY[0];
//     bufferY[0] = output;
// }

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 */
void renderAudio(volatile int32_t *renderDest) {
    static vec<VOICESPERCHIP> sampleSteinerAcc;
    static vec<VOICESPERCHIP> sampleLadderAcc;

    vec<VOICESPERCHIP> oscASample;
    vec<VOICESPERCHIP> oscBSample;
    vec<VOICESPERCHIP> noiseSample;
    vec<VOICESPERCHIP> subSample;

    vec<VOICESPERCHIP> sampleSteiner;
    vec<VOICESPERCHIP> sampleLadder;

    for (uint32_t sample = 0; sample < SAIDMABUFFERSIZE; sample++) {

        // get Sample
        oscASample = getOscASample();
        oscBSample = getOscBSample();
        noiseSample = getNoiseSample();
        subSample = getSubSample();

        // sum
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

        // 1pole filter
        sampleSteinerAcc += 0.06151176F * ((sampleSteiner)-sampleSteinerAcc);
        sampleLadderAcc += 0.06151176F * ((sampleLadder)-sampleLadderAcc);

        // limit and scale
        renderDest[sample * AUDIOCHANNELS + 1 * 2] = softLimit(sampleLadderAcc[0]) * 8388607.0f;
        renderDest[sample * AUDIOCHANNELS + 0 * 2] = softLimit(sampleLadderAcc[1]) * 8388607.0f;
        renderDest[sample * AUDIOCHANNELS + 3 * 2] = softLimit(sampleLadderAcc[2]) * 8388607.0f;
        renderDest[sample * AUDIOCHANNELS + 2 * 2] = softLimit(sampleLadderAcc[3]) * 8388607.0f;

        renderDest[sample * AUDIOCHANNELS + 1 * 2 + 1] = softLimit(sampleSteinerAcc[0]) * 8388607.0f;
        renderDest[sample * AUDIOCHANNELS + 0 * 2 + 1] = softLimit(sampleSteinerAcc[1]) * 8388607.0f;
        renderDest[sample * AUDIOCHANNELS + 3 * 2 + 1] = softLimit(sampleSteinerAcc[2]) * 8388607.0f;
        renderDest[sample * AUDIOCHANNELS + 2 * 2 + 1] = softLimit(sampleSteinerAcc[3]) * 8388607.0f;
    }

    layerA.noise.out = noiseSample;
    layerA.oscA.outSub = subSample;
    layerA.oscA.out = oscASample;
    layerA.oscB.out = oscBSample;
}

#endif