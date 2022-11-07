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

inline float bitcrush(float bitcrush, float bitcrushInv, float sample) {

    // vec<VOICESPERCHIP> mult = 1.0f / max(bitcrush, (1.0f / 8388607.0f));
    float bitCrushedSample = bitcrushInv * sample;
    bitCrushedSample = std::round(bitCrushedSample);
    bitCrushedSample = bitCrushedSample * bitcrush;

    return bitCrushedSample;
}

inline float getSubSample(float phase) {

    const float &shape = layerA.oscA.shapeSub[0];

    uint32_t positionA = phase * WaveTable::subSize[0];

    float sampleA = subOscWavetable[0].subData[0][positionA];
    float sampleB = subOscWavetable[1].subData[0][positionA];

    float newSample = fast_lerp_f32(sampleA, sampleB, shape);

    // static float prevNewSample = 0;

    // float derivCorrection = 1.0f / UIAUDIOPHASESTEP;

    // float derivSample1 = (newSample - prevNewSample) * derivCorrection;

    // prevNewSample = newSample;

    return newSample;
}

inline float getOscASample(float phase) {
    static uint32_t sampleCrushCount = 0;
    sampleCrushCount = sampleCrushCount * (phase > 0.0001f); // first run = 0

    static float sample;

    // const float &morph = layerA.oscA.morph[0];
    const float &bitcrusher = layerA.oscA.bitcrusher[0];
    const float &bitcrusherInv = layerA.oscA.bitcrusherInv[0];
    const float &samplecrusher = layerA.oscA.samplecrusher[0];
    const float &morphFract = layerA.oscA.morphFract[0];

    const uint32_t &waveTableSelectionLower = layerA.oscA.waveTableSelectionLower[0];
    const uint32_t &waveTableSelectionUpper = layerA.oscA.waveTableSelectionUpper[0];

    float shapedPhase = renderPhaseshaperSample(phase, layerA.phaseshaperA);

    uint32_t positionA = shapedPhase * WaveTable::subSize[0];

    float sampleA = oscAwavetable[waveTableSelectionLower].subData[0][positionA];
    float sampleB = oscAwavetable[waveTableSelectionUpper].subData[0][positionA];

    float newSample = fast_lerp_f32(sampleA, sampleB, morphFract);

    newSample = renderWaveshaperSample(newSample, layerA.waveshaperA);
    newSample = bitcrush(bitcrusher, bitcrusherInv, newSample);

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
    const float &bitcrusher = layerA.oscB.bitcrusher[0];
    const float &bitcrusherInv = layerA.oscB.bitcrusherInv[0];
    const float &samplecrusher = layerA.oscB.samplecrusher[0];
    const float &phaseoffset = layerA.oscB.phaseoffset[0];
    const float &morphFract = layerA.oscB.morphFract[0];

    const uint32_t &waveTableSelectionLower = layerA.oscB.waveTableSelectionLower[0];
    const uint32_t &waveTableSelectionUpper = layerA.oscB.waveTableSelectionUpper[0];

    // static float prevPhase = -UIAUDIOPHASESTEP;

    // make sure to be in the right step range
    float phaseOffsetted = phase + phaseoffset;
    phaseOffsetted -= floor(phaseOffsetted);
    phaseOffsetted += (phaseOffsetted < 0.0f);

    float shapedPhase = renderPhaseshaperSample(phaseOffsetted, layerA.phaseshaperB);

    uint32_t positionA = shapedPhase * WaveTable::subSize[0];

    float sampleA = oscBwavetable[waveTableSelectionLower].subData[0][positionA];
    float sampleB = oscBwavetable[waveTableSelectionUpper].subData[0][positionA];
    float newSample = fast_lerp_f32(sampleA, sampleB, morphFract);

    newSample = renderWaveshaperSample(newSample, layerA.waveshaperB);
    newSample = bitcrush(bitcrusher, bitcrusherInv, newSample);

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
    return std::min(sample, 1.0f) * getSign(inputSample);
}

void renderAudioUI(int8_t *renderDest) {
    float phase = 0;
    for (uint32_t sample = 0; sample < 100; sample++) {
        renderDest[sample] = (int8_t)(softLimit(getOscASample(phase)) * 127.0f);
        renderDest[sample + 100] = (int8_t)(softLimit(getOscBSample(phase)) * 127.0f);
        renderDest[sample + (100 * 2)] = (int8_t)(softLimit(getSubSample(phase)) * 127.0f); // TODO SUB Obsolete
        phase += 0.01f;
    }
}

#endif