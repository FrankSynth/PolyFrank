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

#define UIAUDIOPHASESTEP 0.01f

extern WaveTable oscAwavetable[MAXWAVETABLESPERVOICE];
extern WaveTable oscBwavetable[MAXWAVETABLESPERVOICE];
extern WaveTable subOscWavetable[2];

extern WaveTable rippleOscAWavetable;

inline void bitcrush(const float &bitcrush, const float &bitcrushInv, float &sample) {
    sample = bitcrushInv * sample;
    sample = round(sample);
    sample = sample * bitcrush;
}

inline float getSubSample(float phase) {

    const float &shape = layerA.oscA.shapeSub[0];

    uint32_t positionA = phase * WaveTable::subSize[0];

    float sampleA = subOscWavetable[0].subData[0][positionA];
    float sampleB = subOscWavetable[1].subData[0][positionA];

    float newSample = faster_lerp_f32(sampleA, sampleB, shape);
    return newSample;
}

inline void getRippleSample(float &sampleRipple, float &newPhase) {
    static float ripplePhase = 0.0f;
    static float rippleDamp = 0.0f;
    static float oldPhase;

    // Phase
    float phaseProgress = newPhase - oldPhase;

    if (phaseProgress < 0.0f) { // negative value -> new Phase -> reset
        rippleDamp = 1.0f;
        ripplePhase = 0.0f;
        phaseProgress = newPhase; // remove sign
    }
    oldPhase = newPhase;

    // calculat Ripple progress
    ripplePhase = ripplePhase + phaseProgress * layerA.oscA.RippleRatio[0];
    rippleDamp = rippleDamp - phaseProgress * layerA.oscA.RippleDamp[0];

    // limit Ripple
    rippleDamp = fmaxf(rippleDamp, 0.0f);
    ripplePhase = ripplePhase - floorf(ripplePhase);

    // sample Sinus Wave
    float stepWavetable;
    uint32_t positionA;
    uint32_t positionB;
    float interSamplePos;

    // sample index

    stepWavetable = ripplePhase * MAXSUBWAVETABLELENGTH;
    positionA = stepWavetable;
    positionB = positionA + 1U;

    // interpolation
    interSamplePos = stepWavetable - positionA;

    float sampleA;
    float sampleAb;

    positionB = positionB * (positionB != MAXSUBWAVETABLELENGTH);
    sampleA = rippleOscAWavetable.subData[0][positionA];
    sampleAb = rippleOscAWavetable.subData[0][positionB];

    sampleRipple += faster_lerp_f32(sampleA, sampleAb, interSamplePos) * rippleDamp * layerA.oscA.RippleAmount[0];
}

inline float getOscASample(float phase) {
    static uint32_t sampleCrushCount = 0;
    sampleCrushCount = sampleCrushCount * (phase > 0.0001f); // first run = 0

    static float sample;

    // const float &morph = layerA.oscA.morph[0];
    float &bitcrusher = layerA.oscA.bitcrusher[0];
    float &bitcrusherInv = layerA.oscA.bitcrusherInv[0];
    float &samplecrusher = layerA.oscA.samplecrusher[0];
    float &morphFract = layerA.oscA.morphFract[0];

    uint32_t &waveTableSelectionA = layerA.oscA.waveTableSelectionA[0];
    uint32_t &waveTableSelectionB = layerA.oscA.waveTableSelectionB[0];

    // Phaseshaper
    float shapedPhase = renderPhaseshaperSample(phase, layerA.phaseshaperA);

    // sample
    uint32_t positionA = shapedPhase * WaveTable::subSize[0];

    float sampleA = oscAwavetable[waveTableSelectionA].subData[0][positionA];
    float sampleB = oscAwavetable[waveTableSelectionB].subData[0][positionA];

    float newSample = faster_lerp_f32(sampleA, sampleB, morphFract);

    // waveshaper
    newSample = renderWaveshaperSample(newSample, layerA.waveshaperA);

    // ripple
    getRippleSample(newSample, phase);

    // bitCrush
    bitcrush(bitcrusher, bitcrusherInv, newSample);

    // sampleCrush
    sampleCrushCount = sampleCrushCount + 24;

    bool sampleCrushNow = sampleCrushCount > (samplecrusher * 960.f);

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

float getOscBSample(float phase) {
    static uint32_t sampleCrushCount = 0;
    sampleCrushCount = sampleCrushCount * (phase > 0.0001f); // first run = 0

    static float sample;

    // const float &morph = layerA.oscB.morph[0];
    float &bitcrusher = layerA.oscB.bitcrusher[0];
    float &bitcrusherInv = layerA.oscB.bitcrusherInv[0];
    float &samplecrusher = layerA.oscB.samplecrusher[0];
    float &phaseoffset = layerA.oscB.phaseoffset[0];
    float &morphFract = layerA.oscB.morphFract[0];

    uint32_t &waveTableSelectionA = layerA.oscB.waveTableSelectionA[0];
    uint32_t &waveTableSelectionB = layerA.oscB.waveTableSelectionB[0];

    // static float prevPhase = -UIAUDIOPHASESTEP;

    // make sure to be in the right step range
    float phaseOffsetted = phase + phaseoffset;
    phaseOffsetted -= floor(phaseOffsetted);
    phaseOffsetted += (phaseOffsetted < 0.0f);

    // Phaseshaper
    float shapedPhase = renderPhaseshaperSample(phase, layerA.phaseshaperB);

    // sample
    uint32_t positionA = shapedPhase * WaveTable::subSize[0];

    float sampleA = oscBwavetable[waveTableSelectionA].subData[0][positionA];
    float sampleB = oscBwavetable[waveTableSelectionB].subData[0][positionA];

    float newSample = faster_lerp_f32(sampleA, sampleB, morphFract);

    // waveshaper
    newSample = renderWaveshaperSample(newSample, layerA.waveshaperB);

    // bitCrush
    bitcrush(bitcrusher, bitcrusherInv, newSample);

    // sampleCrush
    sampleCrushCount = sampleCrushCount + 24;

    bool sampleCrushNow = sampleCrushCount > (samplecrusher * 960.f);

    sampleCrushCount *= !sampleCrushNow;
    sample = newSample * sampleCrushNow + sample * !sampleCrushNow;

    return sample;
}

inline float softLimit(float inputSample) {

    const float threshold = 0.8f;
    const float maxVal = 3.0f;

    ///////////////////////////////
    // currently for threshold = 0.8, maxVal = 3.0
    // const uint32_t n = 11;
    const float d = -0.0000342279198712f;
    ///////////////////////////////

    // float sign = getSign(inputSample);
    float absInput = fabs(inputSample);

    if (absInput <= threshold)
        return inputSample;

    float sub = maxVal - absInput;

    sub = sub * sub * sub * sub * sub * sub * sub * sub * sub * sub * sub;

    float sample = d * sub + 1.0f;
    return fminf(sample, 1.0f) * getSign(inputSample);
}

void renderAudioUI(int8_t *renderDest) {
    float phase = 0;
    for (uint32_t sample = 0; sample < 100; sample++) {
        renderDest[sample] = (int8_t)(softLimit(getOscASample(phase)) * 127.0f);
        renderDest[sample + 100] = (int8_t)(softLimit(getOscBSample(phase)) * 127.0f);
        phase += 0.01f;
    }
}

#endif