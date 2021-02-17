#ifdef POLYRENDER

#include "renderAudio.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "render/renderAudioDef.h"
#include "rng.h"
#include <cmath>
#include <valarray>

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

static inline void bitcrush(float *dst, const float *bitcrush) {

    uint32_t moveAmount[VOICESPERCHIP];
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

static inline void getNoiseSample(float *sample) {
    uint32_t bitcrusher[VOICESPERCHIP];
    uint32_t randomNumber[VOICESPERCHIP];
    uint32_t bitcrushBits[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        bitcrusher[i] = layerA.noise.bitcrusher.currentSample[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        bitcrushBits[i] = (0xFFFFFFFF << bitcrusher[i]) & 0x00FFFFFF;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        randomNumber[i] = randomNumber[i] & bitcrushBits[i];

    // map to -1, 1
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sample[i] = ((float)randomNumber[i] / 8388607.0f) - 1.0f;
}

static inline void getSubSample(float *sample) {

    float oscAphase[VOICESPERCHIP];

    static const float *bitcrusher = layerA.sub.bitcrusher.currentSample;
    static const float *shape = layerA.sub.shape.currentSample;

    static const uint32_t &octave = layerA.sub.dOctaveSwitch.valueMapped;

    static float *phase = layerA.sub.phase;
    static float *oscApreviousPhase = layerA.sub.oscApreviousPhase;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        oscAphase[i] = layerA.oscA.phaseWavetableA[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (phase[i] >= 1.0f)
            phase[i] -= 1.0f;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phase[i] < 0.5f) {
            sample[i] = phase[i] * (4.0f / shape[i]) - 1.0f;
            std::min(sample[i], 1.0f);
        }
        else {
            sample[i] = ((phase[i] - 0.5f) * (-4.0f / shape[i]) + 1.0f);
            std::max(sample[i], -1.0f);
        }
    }

    bitcrush(sample, bitcrusher);

    float phaseLength = octave == 0 ? 0.5f : 0.25f;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (oscAphase[i] >= 1.0f)
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

static inline void getOscASample(float *sample) {
    static float *phaseWavetableA = layerA.oscA.phaseWavetableA;
    static float *phaseWavetableB = layerA.oscA.phaseWavetableB;
    static const float *morph = layerA.oscA.morph.currentSample;
    static const float *noteStep = layerA.oscA.note.currentSample;
    static const float *bitcrusher = layerA.oscA.bitcrusher.currentSample;

    float sampleA[VOICESPERCHIP];
    float sampleB[VOICESPERCHIP];
    float stepWavetableA[VOICESPERCHIP];
    float stepWavetableB[VOICESPERCHIP];

    // make sure to be in the right step range
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableA[i] >= oscAwavetableACycles)
            phaseWavetableA[i] -= std::floor(phaseWavetableA[i]);
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableB[i] >= oscAwavetableBCycles)
            phaseWavetableB[i] -= std::floor(phaseWavetableB[i]);
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableA[i] = phaseWavetableA[i] * oscAwavetableASizePerCycle;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableB[i] = phaseWavetableB[i] * oscAwavetableBSizePerCycle;

    uint32_t positionA[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionA[i] = stepWavetableA[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscAwavetableA[positionA[i]];

    // float interSampleA = stepWavetableA - positionA;
    // if (positionA < oscAwavetableASize) {
    //     sampleA = fast_lerp_f32(oscAwavetableA[positionA], oscAwavetableA[positionA + 1], interSampleA);
    // }
    // else {
    //     sampleA = fast_lerp_f32(oscAwavetableA[positionA], oscAwavetableA[0], interSampleA);
    // }
    uint32_t positionB[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        positionB[i] = stepWavetableB[i];
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscAwavetableB[positionB[i]];

    // float interSampleB = stepWavetableB - positionB;
    // if (positionB < oscAwavetableBSize) {
    //     sampleB = fast_lerp_f32(oscAwavetableB[positionB], oscAwavetableB[positionB + 1], interSampleB);
    // }
    // else {
    //     sampleB = fast_lerp_f32(oscAwavetableB[positionB], oscAwavetableB[0], interSampleB);
    // }

    fast_lerp_f32(sampleA, sampleB, morph, sample);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableA[i] += noteStep[i] * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableB[i] += noteStep[i] * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)

    bitcrush(sample, bitcrusher);
}

static inline void getOscBSample(float *sample) {

    // cache step of osc A, if smaller, new phase has begun.
    const float *oscAphaseWavetableA = layerA.oscA.phaseWavetableA;

    int32_t &sync = layerA.oscB.dSync.valueMapped;

    float *cacheOscAstep = layerA.oscB.cacheOscAstep;
    float *phaseWavetableA = layerA.oscB.phaseWavetableA;
    float *phaseWavetableB = layerA.oscB.phaseWavetableB;
    const float *morph = layerA.oscB.morph.currentSample;
    const float *noteStep = layerA.oscB.note.currentSample;
    const float *bitcrusher = layerA.oscB.bitcrusher.currentSample;

    float sampleA[VOICESPERCHIP];
    float sampleB[VOICESPERCHIP];
    float stepWavetableA[VOICESPERCHIP];
    float stepWavetableB[VOICESPERCHIP];
    static float checkWavetableLength[VOICESPERCHIP] = {0};

    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (oscAphaseWavetableA[i] >= 1.0f)
            checkWavetableLength[i] -= std::floor(oscAphaseWavetableA[i]);
        else
            checkWavetableLength[i] = oscAphaseWavetableA[i];
    }

    if (sync) {
        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            if (cacheOscAstep[i] > checkWavetableLength[i]) {
                stepWavetableA[i] = 0;
                stepWavetableB[i] = 0;
            }
        }
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        cacheOscAstep[i] = checkWavetableLength[i];

    // make sure to be in the right step range
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableA[i] >= oscBwavetableACycles) {
            phaseWavetableA[i] -= std::floor(phaseWavetableA[i]);
        }
    }
    for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
        if (phaseWavetableB[i] >= oscBwavetableBCycles) {
            phaseWavetableB[i] -= std::floor(phaseWavetableB[i]);
        }
    }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableA[i] = phaseWavetableA[i] * oscBwavetableASizePerCycle;
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        stepWavetableB[i] = phaseWavetableB[i] * oscBwavetableBSizePerCycle;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleA[i] = oscBwavetableA[(uint32_t)stepWavetableA[i]];

    // float interSampleA = stepWavetableA - positionA;
    // if (positionA < oscBwavetableASize) {
    //     sampleA = fast_lerp_f32(oscBwavetableA[positionA], oscBwavetableA[positionA + 1], interSampleA);
    // }
    // else {
    //     sampleA = fast_lerp_f32(oscBwavetableA[positionA], oscBwavetableA[0], interSampleA);
    // }

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sampleB[i] = oscBwavetableB[(uint32_t)stepWavetableB[i]];

    // float interSampleB = stepWavetableB - positionB;
    // if (positionB < oscBwavetableBSize) {
    //     sampleB = fast_lerp_f32(oscBwavetableB[positionB], oscBwavetableB[positionB + 1], interSampleB);
    // }
    // else {
    //     sampleB = fast_lerp_f32(oscBwavetableB[positionB], oscBwavetableB[0], interSampleB);
    // }

    fast_lerp_f32(sampleA, sampleB, morph, sample);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableA[i] += noteStep[i] * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        phaseWavetableB[i] += noteStep[i] * 0.0002537427f; // 1.0f / (float)MINWAVETABLELENGTH)

    bitcrush(sample, bitcrusher);
}

/**
 * @brief render all Audio Samples
 *
 * @param renderDest output buffer
 */
void renderAudio(int32_t *renderDest) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

    static const float *noiseLevelLadder = layerA.noise.levelLadder.currentSample;
    static const float *noiseLevelSteiner = layerA.noise.levelSteiner.currentSample;
    static float *noiseOut = layerA.noise.out.nextSample;

    static const float *subLevelLadder = layerA.sub.levelLadder.currentSample;
    static const float *subLevelSteiner = layerA.sub.levelSteiner.currentSample;
    static float *subOut = layerA.sub.out.nextSample;

    static const float *oscALevelLadder = layerA.oscA.levelLadder.currentSample;
    static const float *oscALevelSteiner = layerA.oscA.levelSteiner.currentSample;
    static float *oscAOut = layerA.oscA.out.nextSample;

    static const float *oscBLevelLadder = layerA.oscB.levelLadder.currentSample;
    static const float *oscBLevelSteiner = layerA.oscB.levelSteiner.currentSample;
    static float *oscBOut = layerA.oscB.out.nextSample;

    float oscASample[VOICESPERCHIP];
    float oscBSample[VOICESPERCHIP];
    float noiseSample[VOICESPERCHIP];
    float subSample[VOICESPERCHIP];

    float dampSteiner[VOICESPERCHIP];
    float dampLadder[VOICESPERCHIP];

    float maxVolSteiner[VOICESPERCHIP];
    float maxVolLadder[VOICESPERCHIP];

    float sampleSteiner[VOICESPERCHIP];
    float sampleLadder[VOICESPERCHIP];

    int32_t intSampleSteiner[VOICESPERCHIP];
    int32_t intSampleLadder[VOICESPERCHIP];

    for (uint32_t sample = 0; sample < SAIDMABUFFERSIZE; sample++) {

        getOscASample(oscASample);
        getOscBSample(oscBSample);
        getNoiseSample(noiseSample);
        getSubSample(subSample);

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] = noiseLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] += subLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] += oscALevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolSteiner[i] += oscBLevelSteiner[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] = noiseLevelLadder[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] += subLevelLadder[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] += oscALevelLadder[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            maxVolLadder[i] += oscBLevelLadder[i];

        // static std::valarray<bool> checkDampSteiner[VOICESPERCHIP];
        // static std::valarray<bool> checkDampLadder[VOICESPERCHIP];

        // checkDampSteiner = maxVolSteiner > MAXPOSSIBLEVOLUME;
        // checkDampLadder = maxVolLadder > MAXPOSSIBLEVOLUME;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            if (maxVolSteiner[i] > MAXPOSSIBLEVOLUME) {
                dampSteiner[i] = MAXPOSSIBLEVOLUME / maxVolSteiner[i];
            }
            else {
                dampSteiner[i] = 1;
            }
        }

        for (uint32_t i = 0; i < VOICESPERCHIP; i++) {
            if (maxVolLadder[i] > MAXPOSSIBLEVOLUME) {
                dampLadder[i] = MAXPOSSIBLEVOLUME / maxVolLadder[i];
            }
            else {
                dampLadder[i] = 1;
            }
        }

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] = noiseSample[i] * noiseLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] = noiseSample[i] * noiseLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] += subSample[i] * subLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] += subSample[i] * subLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] += oscASample[i] * oscALevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] += oscASample[i] * oscALevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] += oscBSample[i] * oscBLevelSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] += oscBSample[i] * oscBLevelLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] = sampleSteiner[i] * dampSteiner[i];
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] = sampleLadder[i] * dampLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] *= MAXVOLUMEPERMODULE;
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] *= MAXVOLUMEPERMODULE;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleSteiner[i] *= 8388607.0f;
        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            sampleLadder[i] *= 8388607.0f;

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            intSampleSteiner[i] = sampleSteiner[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            intSampleLadder[i] = sampleLadder[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            renderDest[sample * AUDIOCHANNELS + i] = intSampleSteiner[i];

        for (uint32_t i = 0; i < VOICESPERCHIP; i++)
            renderDest[sample * AUDIOCHANNELS + i] = intSampleLadder[i];

        if (sample == 0) {
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                noiseOut[i] = noiseSample[i];
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                subOut[i] = subSample[i];
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                oscAOut[i] = oscASample[i];
            for (uint32_t i = 0; i < VOICESPERCHIP; i++)
                oscBOut[i] = oscBSample[i];
        }
    }
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
}

#endif