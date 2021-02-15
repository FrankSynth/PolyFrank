#ifdef POLYRENDER

#include "renderAudio.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "rng.h"
#include <cmath>

#define SAMPLERATE 96000
#define FULLSCALE 0xFFFFFFFF

extern Layer layerA;

RAM1 float oscAwavetableA[MAXWAVETABLELENGTH];
uint32_t oscAwavetableASize;
uint32_t oscAwavetableACycles;
float oscAwavetableASizePerCycle;
RAM1 float oscAwavetableB[MAXWAVETABLELENGTH];
uint32_t oscAwavetableBSize;
uint32_t oscAwavetableBCycles;
float oscAwavetableBSizePerCycle;

RAM1 float oscBwavetableA[MAXWAVETABLELENGTH];
uint32_t oscBwavetableASize;
uint32_t oscBwavetableACycles;
float oscBwavetableASizePerCycle;
RAM1 float oscBwavetableB[MAXWAVETABLELENGTH];
uint32_t oscBwavetableBSize;
uint32_t oscBwavetableBCycles;
float oscBwavetableBSizePerCycle;

/**
 * @brief convert float to DAC compatible integer
 *
 * @param f
 * @return int32_t
 */
static inline int32_t convertAudioSample(float f) {
    // f = testFloat(f, -1, 1);
    float t = f * 8388607.0f;

    return (int32_t)(t);
}

/**
 * @brief switch OscA Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscAWavetableA(wavetable wavetable) {
    oscAwavetableASize = wavetable.size;
    oscAwavetableACycles = wavetable.cycles;
    oscAwavetableASizePerCycle = wavetable.size / wavetable.cycles;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscAwavetableA, oscAwavetableASize);
}

/**
 * @brief switch OscA Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscAWavetableB(wavetable wavetable) {
    oscAwavetableBSize = wavetable.size;
    oscAwavetableBCycles = wavetable.cycles;
    oscAwavetableBSizePerCycle = wavetable.size / wavetable.cycles;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscAwavetableB, oscAwavetableBSize);
}

/**
 * @brief switch OscB Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscBWavetableA(wavetable wavetable) {
    oscBwavetableASize = wavetable.size;
    oscBwavetableACycles = wavetable.cycles;
    oscBwavetableASizePerCycle = wavetable.size / wavetable.cycles;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscBwavetableA, oscBwavetableASize);
}

/**
 * @brief switch OscB Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscBWavetableB(wavetable wavetable) {
    oscBwavetableBSize = wavetable.size;
    oscBwavetableBCycles = wavetable.cycles;
    oscBwavetableBSizePerCycle = wavetable.size / wavetable.cycles;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscBwavetableB, oscBwavetableBSize);
}

/**
 * @brief set and load default wavetables
 *
 */
void initAudioRendering() {
    // Osc A
    switchOscAWavetableA(wavetable_sinus01);
    switchOscAWavetableB(wavetable_nylonGuitar01);

    // Osc B
    switchOscBWavetableA(wavetable_sinus01);
    switchOscBWavetableB(wavetable_wurli02);
}

inline float bitcrush(const float &sample, const float &bitcrush) {
    uint32_t bitcrushFloored = bitcrush;
    if (bitcrushFloored == 0)
        return sample;

    uint32_t moveAmount = 23 - bitcrushFloored;

    float mult = 1 << moveAmount;

    float crushedSample = std::round(mult * sample) / mult;

    return crushedSample;
}

float getNoiseSample(uint16_t voice) {
    uint32_t bitcrusher = layerA.noise.bitcrusher.currentSample[voice];

    uint32_t randomNumber;

    HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber);

    // TODO very low effect

    if (bitcrusher > 0) {
        uint32_t bitcrushBits = 0xFFFFFFFF << bitcrusher;

        randomNumber = randomNumber & bitcrushBits;
    }
    randomNumber = randomNumber & 0x00FFFFFF;

    // map to -1, 1
    return ((float)randomNumber / 8388607.0f) - 1.0f;
}

float getSubSample(uint16_t voice) {

    float oscAphase = layerA.oscA.phaseWavetableA[voice];
    const float &bitcrusher = layerA.sub.bitcrusher.currentSample[voice];
    const float shape = layerA.sub.shape.currentSample[voice];
    const uint32_t &octave = layerA.sub.dOctaveSwitch.valueMapped;
    float &phase = layerA.sub.phase[voice];
    float &oscApreviousPhase = layerA.sub.oscApreviousPhase[voice];

    float sample;

    if (phase >= 1.0f)
        phase -= 1.0f;

    if (phase < 0.5f) {
        sample = phase * (4.0f / shape) - 1.0f;
        if (sample > 1.0f)
            sample = 1.0f;
    }
    else {
        sample = ((phase - 0.5f) * (-4.0f / shape) + 1.0f);
        if (sample < -1.0f)
            sample = -1.0f;
    }

    sample = bitcrush(sample, bitcrusher);

    float phaseLength = octave == 0 ? 0.5f : 0.25f;

    if (oscAphase >= 1.0f)
        oscAphase -= std::floor(oscAphase);

    if (oscAphase < oscApreviousPhase)
        oscApreviousPhase -= 1.0f;

    float phaseDifferenceOscA = oscAphase - oscApreviousPhase;

    oscApreviousPhase = oscAphase;
    phase += phaseDifferenceOscA * phaseLength;

    return sample;
}

float getOscASample(uint16_t voice) {
    float &phaseWavetableA = layerA.oscA.phaseWavetableA[voice];
    float &phaseWavetableB = layerA.oscA.phaseWavetableB[voice];
    const float &morph = layerA.oscA.morph.currentSample[voice];
    const float &noteStep = layerA.oscA.note.currentSample[voice];
    const float &bitcrusher = layerA.oscA.bitcrusher.currentSample[voice];

    float sample, sampleA, sampleB, stepWavetableA, stepWavetableB;

    // make sure to be in the right step range
    if (phaseWavetableA >= oscAwavetableACycles)
        phaseWavetableA -= std::floor(phaseWavetableA);

    if (phaseWavetableB >= oscAwavetableBCycles)
        phaseWavetableB -= std::floor(phaseWavetableB);

    stepWavetableA = phaseWavetableA * oscAwavetableASizePerCycle;
    stepWavetableB = phaseWavetableB * oscAwavetableBSizePerCycle;

    uint32_t positionA = stepWavetableA;
    sampleA = oscAwavetableA[positionA];

    // float interSampleA = stepWavetableA - positionA;
    // if (positionA < oscAwavetableASize) {
    //     sampleA = fast_lerp_f32(oscAwavetableA[positionA], oscAwavetableA[positionA + 1], interSampleA);
    // }
    // else {
    //     sampleA = fast_lerp_f32(oscAwavetableA[positionA], oscAwavetableA[0], interSampleA);
    // }

    uint32_t positionB = stepWavetableB;
    sampleB = oscAwavetableB[positionB];

    // float interSampleB = stepWavetableB - positionB;
    // if (positionB < oscAwavetableBSize) {
    //     sampleB = fast_lerp_f32(oscAwavetableB[positionB], oscAwavetableB[positionB + 1], interSampleB);
    // }
    // else {
    //     sampleB = fast_lerp_f32(oscAwavetableB[positionB], oscAwavetableB[0], interSampleB);
    // }

    sample = fast_lerp_f32(sampleA, sampleB, morph);

    phaseWavetableA += noteStep * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)
    phaseWavetableB += noteStep * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)

    sample = bitcrush(sample, bitcrusher);

    return sample;
}

float getOscBSample(uint16_t voice) {

    // cache step of osc A, if smaller, new phase has begun.
    const float &oscAphaseWavetableA = layerA.oscA.phaseWavetableA[voice];

    int32_t &sync = layerA.oscB.dSync.valueMapped;

    float &cacheOscAstep = layerA.oscB.cacheOscAstep[voice];

    float &phaseWavetableA = layerA.oscB.phaseWavetableA[voice];
    float &phaseWavetableB = layerA.oscB.phaseWavetableB[voice];
    const float &morph = layerA.oscB.morph.currentSample[voice];
    const float &noteStep = layerA.oscB.note.currentSample[voice];
    const float &bitcrusher = layerA.oscB.bitcrusher.currentSample[voice];

    float sample, sampleA, sampleB, stepWavetableA, stepWavetableB;

    float checkWavetableLength;

    checkWavetableLength = oscAphaseWavetableA;

    if (checkWavetableLength >= 1.0f)
        checkWavetableLength -= std::floor(oscAphaseWavetableA);

    if (sync) {
        if (cacheOscAstep > checkWavetableLength) {
            stepWavetableA = 0;
            stepWavetableB = 0;
        }
    }
    cacheOscAstep = checkWavetableLength;

    // make sure to be in the right step range
    if (phaseWavetableA >= (float)oscBwavetableACycles) {
        phaseWavetableA -= std::floor(phaseWavetableA);
    }
    if (phaseWavetableB >= (float)oscBwavetableBCycles) {
        phaseWavetableB -= std::floor(phaseWavetableB);
    }

    stepWavetableA = phaseWavetableA * oscBwavetableASizePerCycle;
    stepWavetableB = phaseWavetableB * oscBwavetableBSizePerCycle;

    uint32_t positionA = (uint32_t)stepWavetableA;

    sampleA = oscBwavetableA[positionA];

    // float interSampleA = stepWavetableA - positionA;
    // if (positionA < oscBwavetableASize) {
    //     sampleA = fast_lerp_f32(oscBwavetableA[positionA], oscBwavetableA[positionA + 1], interSampleA);
    // }
    // else {
    //     sampleA = fast_lerp_f32(oscBwavetableA[positionA], oscBwavetableA[0], interSampleA);
    // }

    uint32_t positionB = (uint32_t)stepWavetableB;
    sampleB = oscBwavetableB[positionB];

    // float interSampleB = stepWavetableB - positionB;
    // if (positionB < oscBwavetableBSize) {
    //     sampleB = fast_lerp_f32(oscBwavetableB[positionB], oscBwavetableB[positionB + 1], interSampleB);
    // }
    // else {
    //     sampleB = fast_lerp_f32(oscBwavetableB[positionB], oscBwavetableB[0], interSampleB);
    // }

    sample = fast_lerp_f32(sampleA, sampleB, morph);

    phaseWavetableA += noteStep * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)
    phaseWavetableB += noteStep * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)

    sample = bitcrush(sample, bitcrusher);

    return sample;
}

/**
 * @brief render specific voice
 *
 * @param renderDest output buffer
 * @param samples amount of samples to render
 * @param outSteiner out channel on chip to Steiner filter
 * @param outLadder out channel on chip to Ladder filter
 */
void renderVoice(int32_t *renderDest, uint16_t samples, uint16_t voice, uint16_t outSteiner, uint16_t outLadder) {

    const float &noiseLevelLadder = layerA.noise.levelLadder.currentSample[voice];
    const float &noiseLevelSteiner = layerA.noise.levelSteiner.currentSample[voice];
    float &noiseOut = layerA.noise.out.nextSample[voice];

    const float &subLevelLadder = layerA.sub.levelLadder.currentSample[voice];
    const float &subLevelSteiner = layerA.sub.levelSteiner.currentSample[voice];
    float &subOut = layerA.sub.out.nextSample[voice];

    const float &oscALevelLadder = layerA.oscA.levelLadder.currentSample[voice];
    const float &oscALevelSteiner = layerA.oscA.levelSteiner.currentSample[voice];
    float &oscAOut = layerA.oscA.out.nextSample[voice];

    const float &oscBLevelLadder = layerA.oscB.levelLadder.currentSample[voice];
    const float &oscBLevelSteiner = layerA.oscB.levelSteiner.currentSample[voice];
    float &oscBOut = layerA.oscB.out.nextSample[voice];

    for (uint32_t sample = 0; sample < samples; sample++) {

        float oscASample = getOscASample(voice);
        float oscBSample = getOscBSample(voice);
        float noiseSample = getNoiseSample(voice);
        float subSample = getSubSample(voice);

        float dampSteiner;
        float dampLadder;

        float maxVolSteiner = noiseLevelSteiner + subLevelSteiner + oscALevelSteiner + oscBLevelSteiner;
        float maxVolLadder = noiseLevelLadder + subLevelLadder + oscALevelLadder + oscBLevelLadder;

        if (maxVolSteiner > MAXPOSSIBLEVOLUME) {
            dampSteiner = MAXPOSSIBLEVOLUME / maxVolSteiner;
        }
        else {
            dampSteiner = 1;
        }

        if (maxVolLadder > MAXPOSSIBLEVOLUME) {
            dampLadder = MAXPOSSIBLEVOLUME / maxVolLadder;
        }
        else {
            dampLadder = 1;
        }

        float sampleSteiner = noiseSample * noiseLevelSteiner;
        float sampleLadder = noiseSample * noiseLevelLadder;

        sampleSteiner += subSample * subLevelSteiner;
        sampleLadder += subSample * subLevelLadder;

        sampleSteiner += oscASample * oscALevelSteiner;
        sampleLadder += oscASample * oscALevelLadder;

        sampleSteiner += oscBSample * oscBLevelSteiner;
        sampleLadder += oscBSample * oscBLevelLadder;

        int32_t intSampleSteiner = convertAudioSample(sampleSteiner * MAXVOLUMEPERMODULE * dampSteiner);
        int32_t intSampleLadder = convertAudioSample(sampleLadder * MAXVOLUMEPERMODULE * dampLadder);

        renderDest[outSteiner + sample * AUDIOCHANNELS] = intSampleSteiner;
        renderDest[outLadder + sample * AUDIOCHANNELS] = intSampleLadder;

        if (sample == 0) {
            noiseOut = noiseSample;
            subOut = subSample;
            oscAOut = oscASample;
            oscBOut = oscBSample;
        }
    }
}

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 * @param samples amount of samples to render
 */
void renderAudio(int32_t *renderDest, uint16_t samples) {

    // TODO final output assigment

    // render voice 1
    renderVoice(renderDest, samples, 0, 3, 4);
    // render voice 2
    renderVoice(renderDest, samples, 1, 5, 2);
    // render voice 3
    renderVoice(renderDest, samples, 2, 1, 0);
    // render voice 4
    renderVoice(renderDest, samples, 3, 7, 6);
}

#endif