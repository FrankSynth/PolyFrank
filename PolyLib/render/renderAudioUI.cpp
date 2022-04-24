#ifdef POLYRENDER

#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
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
    static uint32_t sampleCrushCount;

    const float &bitcrusher = layerA.sub.bitcrusher[0];
    const float &samplecrusher = layerA.sub.samplecrusher[0];
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

    newSample = bitcrush(bitcrusher, newSample);

    sampleCrushCount = sampleCrushCount + 480;

    bool sampleCrushNow = (sampleCrushCount) > samplecrusher;
    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

inline float getOscASample(float phase) {

    static float sample;

    const float &morph = layerA.oscA.morph[0];
    const float &bitcrusher = layerA.oscA.bitcrusher[0];
    const float &samplecrusher = layerA.oscA.samplecrusher[0];

    uint32_t waveTableSelectionLower = morph;
    uint32_t waveTableSelectionUpper = ceil(morph);

    WaveTable *wavetableLower = &oscAwavetable[waveTableSelectionLower];

    WaveTable *wavetableUpper = &oscAwavetable[waveTableSelectionUpper];

    float stepWavetableLower;
    stepWavetableLower = phase * wavetableLower->size;

    float stepWavetableUpper;
    stepWavetableUpper = phase * wavetableUpper->size;

    uint32_t positionA = stepWavetableLower;
    uint32_t positionAb = positionA + 1U;
    positionAb = positionAb * (positionAb != wavetableLower->size);

    float sampleA;
    sampleA = oscAwavetableRAM[waveTableSelectionLower][positionA];

    float sampleAb;
    sampleAb = oscAwavetableRAM[waveTableSelectionLower][positionAb];

    float interSampleAPos = stepWavetableLower - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    uint32_t positionB = stepWavetableUpper;
    uint32_t positionBb = positionB + 1U;
    positionBb = positionBb * (positionBb != wavetableUpper->size);

    float sampleB;
    sampleB = oscAwavetableRAM[waveTableSelectionUpper][positionB];
    float sampleBb;
    sampleBb = oscAwavetableRAM[waveTableSelectionUpper][positionBb];

    float interSampleBPos = stepWavetableUpper - positionB;
    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleBPos);

    float tempMorph = morph - waveTableSelectionLower;
    float newSample = fast_lerp_f32(sampleA, sampleB, tempMorph);

    newSample = renderWaveshaperSample(newSample, layerA);

    newSample = bitcrush(bitcrusher, newSample);

    static uint32_t sampleCrushCount;

    sampleCrushCount = sampleCrushCount + 480;

    bool sampleCrushNow = sampleCrushCount > samplecrusher;

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

    float stepWavetableLower;
    stepWavetableLower = phaseOffsetted * wavetableLower->size;
    float stepWavetableUpper;
    stepWavetableUpper = phaseOffsetted * wavetableUpper->size;

    uint32_t positionA = stepWavetableLower;
    uint32_t positionAb = positionA + 1U;
    positionAb = positionAb * (positionAb != wavetableLower->size);

    float sampleA;
    sampleA = oscBwavetableRAM[waveTableSelectionLower][positionA];
    float sampleAb;
    sampleAb = oscBwavetableRAM[waveTableSelectionLower][positionAb];

    float interSampleAPos = stepWavetableLower - positionA;
    sampleA = fast_lerp_f32(sampleA, sampleAb, interSampleAPos);

    uint32_t positionB = stepWavetableUpper;
    uint32_t positionBb = positionB + 1U;
    positionBb = positionBb * (positionBb != wavetableUpper->size);

    float sampleB;
    sampleB = oscBwavetableRAM[waveTableSelectionUpper][positionB];
    float sampleBb;
    sampleBb = oscBwavetableRAM[waveTableSelectionUpper][positionBb];

    float interSampleBPos = stepWavetableUpper - positionB;
    sampleB = fast_lerp_f32(sampleB, sampleBb, interSampleBPos);

    float tempMorph = morph - waveTableSelectionLower;
    float newSample = fast_lerp_f32(sampleA, sampleB, tempMorph);

    newSample = renderWaveshaperSample(newSample, layerA);

    newSample = bitcrush(bitcrusher, newSample);

    static uint32_t sampleCrushCount = 0;
    sampleCrushCount = sampleCrushCount + 480;
    bool sampleCrushNow = sampleCrushCount > samplecrusher;

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

void renderAudioUI(int8_t *renderDest) {

    float phase = 0;

    for (uint32_t sample = 0; sample < 100; sample++) {

        renderDest[sample] = (int8_t)(getOscASample(phase) * 127.0f);
        renderDest[sample + 100] = (int8_t)(getOscBSample(phase) * 127.0f);
        renderDest[sample + 200] = (int8_t)(getSubSample(phase) * 127.0f);

        phase += 0.01f;
    }
}

#endif