#ifdef POLYRENDER

#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "render/renderPhaseshaper.hpp"
#include "render/renderWaveshaper.hpp"
#include "renderAudio.hpp"
#include "rng.h"
#include <cmath>
#include <mdma.h>

extern Layer layerA;

// std::vector<const WaveTable *> wavetables;

extern const WaveTable *sourcesA[4];
extern const WaveTable *sourcesB[4];

extern float oscAwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH];
extern WaveTable oscAwavetable[MAXWAVETABLESPERVOICE];

extern float oscBwavetableRAM[MAXWAVETABLESPERVOICE][MAXWAVETABLELENGTH];
extern WaveTable oscBwavetable[MAXWAVETABLESPERVOICE];

inline float bitcrush(float bitcrush, float sample) {

    float mult = 1.0f / std::max(bitcrush, (1.0f / 8388607.0f));
    float bitCrushedSample = mult * sample;
    bitCrushedSample = std::round(bitCrushedSample);
    bitCrushedSample = bitCrushedSample / mult;

    return bitCrushedSample;
}

inline float getSubSample(float phase) {

    static float sample;
    // static uint32_t sampleCrushCount = 0;

    // const float &bitcrusher = layerA.sub.bitcrusher[0];
    // const float &samplecrusher = layerA.sub.samplecrusher[0];
    const float &shape = layerA.sub.shape[0];

    float newSample;

    float shapeDiv = 4.0f / std::max(0.01f, shape);

    if (phase < 0.5f) {
        newSample = phase * (shapeDiv)-1.0f;
        newSample = std::min(newSample, 1.0f);
    }
    else {
        newSample = (phase - 0.5f) * (-shapeDiv) + 1.0f;
        newSample = std::max(newSample, -1.0f);
    }

    // newSample = bitcrush(bitcrusher, newSample);
    // sampleCrushCount = sampleCrushCount * std::ceil(phase);

    // sampleCrushCount = sampleCrushCount + 24;

    // bool sampleCrushNow = (sampleCrushCount) > (samplecrusher * 960.f);
    // sampleCrushCount *= !sampleCrushNow;
    // sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

inline float getOscASample(float phase) {

    static float sample;

    const float &morph = layerA.oscA.morph[0];
    const float &bitcrusher = layerA.oscA.bitcrusher[0];
    const float &samplecrusher = layerA.oscA.samplecrusher[0];

    uint32_t waveTableSelectionLower = morph;
    uint32_t waveTableSelectionUpper = std::ceil(morph);

    WaveTable *wavetableLower = &oscAwavetable[waveTableSelectionLower];

    WaveTable *wavetableUpper = &oscAwavetable[waveTableSelectionUpper];

    // TODO set dedicated shaper when ready
    float shapedPhase = renderPhaseshaperSample(phase, layerA.phaseshaperA);

    float stepWavetableLower;
    stepWavetableLower = shapedPhase * wavetableLower->stepRange;

    float stepWavetableUpper;
    stepWavetableUpper = shapedPhase * wavetableUpper->stepRange;

    uint32_t positionA = stepWavetableLower;
    // uint32_t positionAb = positionA + 1U;
    // positionAb = positionAb * (positionAb != wavetableLower->stepRange);

    float sampleA;
    sampleA = oscAwavetableRAM[waveTableSelectionLower][positionA];

    // float sampleAb;
    // sampleAb = oscAwavetableRAM[waveTableSelectionLower][positionAb];

    // float interSampleAPos = stepWavetableLower - positionA;
    // sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    uint32_t positionB = stepWavetableUpper;
    // uint32_t positionBb = positionB + 1U;
    // positionBb = positionBb * (positionBb != wavetableUpper->size);

    float sampleB;
    sampleB = oscAwavetableRAM[waveTableSelectionUpper][positionB];
    // float sampleBb;
    // sampleBb = oscAwavetableRAM[waveTableSelectionUpper][positionBb];

    // float interSampleBPos = stepWavetableUpper - positionB;
    // sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleBPos);

    float tempMorph = morph - waveTableSelectionLower;
    float newSample = fast_lerp_f32(sampleA, sampleB, tempMorph);

    // TODO set dedicated shaper when ready
    newSample = renderWaveshaperSample(newSample, layerA.waveshaperA);

    newSample = bitcrush(bitcrusher, newSample);

    static uint32_t sampleCrushCount = 0;
    sampleCrushCount = sampleCrushCount * std::ceil(phase);

    sampleCrushCount = sampleCrushCount + 24;

    bool sampleCrushNow = sampleCrushCount > (samplecrusher * 960.f);

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

float getOscBSample(float phase) {

    static float sample;

    // cache step of osc A, if smaller, new phase has begun.

    const float &morph = layerA.oscB.morph[0];
    const float &bitcrusher = layerA.oscB.bitcrusher[0];
    const float &samplecrusher = layerA.oscB.samplecrusher[0];
    const float &phaseoffset = layerA.oscB.phaseoffset[0];

    uint32_t waveTableSelectionLower = morph;
    uint32_t waveTableSelectionUpper = ceil(morph);

    WaveTable *wavetableLower = &oscBwavetable[waveTableSelectionLower];
    WaveTable *wavetableUpper = &oscBwavetable[waveTableSelectionUpper];

    // make sure to be in the right step range
    float phaseOffsetted = phase + phaseoffset;
    phaseOffsetted -= floor(phaseOffsetted);
    phaseOffsetted += (phaseOffsetted < 0.0f);

    // TODO set dedicated shaper when ready
    phaseOffsetted = renderPhaseshaperSample(phaseOffsetted, layerA.phaseshaperB);

    float stepWavetableLower;
    stepWavetableLower = phaseOffsetted * wavetableLower->stepRange;
    float stepWavetableUpper;
    stepWavetableUpper = phaseOffsetted * wavetableUpper->stepRange;

    uint32_t positionA = stepWavetableLower;
    // uint32_t positionAb = positionA + 1U;
    // positionAb = positionAb * (positionAb != wavetableLower->size);

    float sampleA;
    sampleA = oscBwavetableRAM[waveTableSelectionLower][positionA];
    // float sampleAb;
    // sampleAb = oscBwavetableRAM[waveTableSelectionLower][positionAb];

    // float interSampleAPos = stepWavetableLower - positionA;
    // sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    uint32_t positionB = stepWavetableUpper;
    // uint32_t positionBb = positionB + 1U;
    // positionBb = positionBb * (positionBb != wavetableUpper->size);

    float sampleB;
    sampleB = oscBwavetableRAM[waveTableSelectionUpper][positionB];
    // float sampleBb;
    // sampleBb = oscBwavetableRAM[waveTableSelectionUpper][positionBb];

    // float interSampleBPos = stepWavetableUpper - positionB;
    // sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleBPos);

    float tempMorph = morph - waveTableSelectionLower;
    float newSample = fast_lerp_f32(sampleA, sampleB, tempMorph);

    // TODO set dedicated shaper when ready
    newSample = renderWaveshaperSample(newSample, layerA.waveshaperB);

    newSample = bitcrush(bitcrusher, newSample);

    static uint32_t sampleCrushCount = 0;

    sampleCrushCount = sampleCrushCount * std::ceil(phase);
    sampleCrushCount = sampleCrushCount + 24;
    bool sampleCrushNow = sampleCrushCount > (samplecrusher * 960.f);

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

inline float softLimit(float inputSample) {
    const float threshold = 0.7f;

    const float maxVal = 4.0f;

    const uint32_t n = -(maxVal - threshold) / (threshold - 1.0f); // careful, with lower threshold no int but float
    const float d = (threshold - 1.0f) / std::pow(maxVal - threshold, n);

    float sign = getSign(inputSample);
    float absInput = inputSample * sign;

    if (absInput <= threshold)
        return inputSample;
    if (absInput >= maxVal)
        return sign;

    float sample = d * powf(maxVal - absInput, n) + 1.0f;

    return std::clamp(sample * sign, -1.0f, 1.0f);
}

void renderAudioUI(int8_t *renderDest) {

    float phase = 0;

    for (uint32_t sample = 0; sample < 100; sample++) {

        renderDest[sample] = (int8_t)(softLimit(getOscASample(phase)) * 127.0f);
        renderDest[sample + 100] = (int8_t)(softLimit(getOscBSample(phase)) * 127.0f);
        renderDest[sample + 200] = (int8_t)(softLimit(getSubSample(phase)) * 127.0f);

        phase += 0.01f;
    }
}

#endif