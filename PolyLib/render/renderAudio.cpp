#ifdef POLYRENDER

#include "renderAudio.hpp"
#include "render/renderAudioDef.h"
#include "rng.h"

#define SAMPLERATE 96000
#define FULLSCALE 0xFFFFFFFF

extern Layer layerA;

RAM1 float oscAwavetableA[MAXWAVETABLELENGTH];
uint32_t oscAwavetableASize;
RAM1 float oscAwavetableB[MAXWAVETABLELENGTH];
uint32_t oscAwavetableBSize;

RAM1 float oscBwavetableA[MAXWAVETABLELENGTH];
uint32_t oscBwavetableASize;
RAM1 float oscBwavetableB[MAXWAVETABLELENGTH];
uint32_t oscBwavetableBSize;

/**
 * @brief convert float to DAC compatible integer
 *
 * @param f
 * @return int32_t
 */
static inline int32_t convertAudioSample(float f) {
    f = testFloat(f, -1, 1);
    double t = (double)f * 8388607.0;

    return (int32_t)(t);
}

/**
 * @brief switch OscA Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscAWavetableA(wavetable wavetable) {
    oscAwavetableASize = wavetable.size;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscAwavetableA, oscAwavetableASize);
}

/**
 * @brief switch OscA Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscAWavetableB(wavetable wavetable) {
    oscAwavetableBSize = wavetable.size;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscAwavetableB, oscAwavetableBSize);
}

/**
 * @brief switch OscB Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscBWavetableA(wavetable wavetable) {
    oscBwavetableASize = wavetable.size;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscBwavetableA, oscBwavetableASize);
}

/**
 * @brief switch OscB Wavetable Slot B
 *
 * @param wavetable new wavetable
 */
void switchOscBWavetableB(wavetable wavetable) {
    oscBwavetableBSize = wavetable.size;
    fast_copy_f32((uint32_t *)wavetable.data, (uint32_t *)oscBwavetableB, oscBwavetableBSize);
}

/**
 * @brief set and load default wavetables
 *
 */
void initAudioRendering() {
    // Osc A
    switchOscAWavetableA(wavetable_strings01);
    switchOscAWavetableB(wavetable_nylonGuitar01);

    // Osc B
    switchOscBWavetableA(wavetable_strings01);
    switchOscBWavetableB(wavetable_nylonGuitar01);
}

float getNoiseSample(uint16_t voice) {
    float &bitcrusher = layerA.noise.bitcrusher.currentSample[voice];

    uint32_t randomNumber;

    if (HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber) != HAL_OK) {
        Error_Handler();
    }

    uint32_t bitcrushBits = bitcrusher;

    randomNumber = randomNumber << bitcrushBits;

    randomNumber = randomNumber & 0x00FFFFFF;

    // map to -1, 1
    return (float)((double)randomNumber / (double)(0x00FFFFFF / 2)) - 1;
}

float getSubSample(uint16_t voice) {
    // float &bitcrusher = layerA.sub.bitcrusher.currentSample[voice];

    return 0;
}

float getOscASample(uint16_t voice) {
    // float &bitcrusher = layerA.oscA.bitcrusher.currentSample[voice];
    float &stepWavetableA = layerA.oscA.stepWavetableA[voice];
    float &stepWavetableB = layerA.oscA.stepWavetableB[voice];
    float &morph = layerA.oscA.morph.currentSample[voice];
    float &noteStep = layerA.oscA.note.currentSample[voice];
    float sample;

    // make sure to be in the right step range
    while (stepWavetableA >= oscAwavetableASize)
        stepWavetableA -= (float)oscAwavetableASize;
    while (stepWavetableB >= (float)oscAwavetableBSize)
        stepWavetableB -= (float)oscAwavetableBSize;

    sample = oscAwavetableA[(uint32_t)stepWavetableA] * morph + oscAwavetableB[(uint32_t)stepWavetableB] * (1 - morph);

    stepWavetableA += noteStep;
    stepWavetableB += noteStep;

    // TODO bitcrushing

    return sample;
}
float getOscBSample(uint16_t voice) {
    // float &oscBbitcrusher = layerA.oscB.bitcrusher.currentSample[voice];
    // float &oscBMorph = layerA.oscB.morph.currentSample[voice];
    float sample = 0;

    return sample;
}

// TODO Audio rendering, split into render voice, with output assignement, also render into module outs
/**
 * @brief render specific voice
 *
 * @param renderDest output buffer
 * @param samples amount of samples to render
 * @param outSteiner out channel on chip to Steiner filter
 * @param outLadder out channel on chip to Ladder filter
 */
void renderVoice(int32_t *renderDest, uint16_t samples, uint16_t voice, uint16_t outSteiner, uint16_t outLadder) {

    float &noiseLevelLadder = layerA.noise.levelLadder.currentSample[voice];
    float &noiseLevelSteiner = layerA.noise.levelSteiner.currentSample[voice];
    float &noiseOut = layerA.noise.out.nextSample[voice];

    float &subLevelLadder = layerA.sub.levelLadder.currentSample[voice];
    float &subLevelSteiner = layerA.sub.levelSteiner.currentSample[voice];
    float &subOut = layerA.sub.out.nextSample[voice];

    float &oscALevelLadder = layerA.oscA.levelLadder.currentSample[voice];
    float &oscALevelSteiner = layerA.oscA.levelSteiner.currentSample[voice];
    float &oscAOut = layerA.oscA.out.nextSample[voice];

    float &oscBLevelLadder = layerA.oscB.levelLadder.currentSample[voice];
    float &oscBLevelSteiner = layerA.oscB.levelSteiner.currentSample[voice];
    float &oscBOut = layerA.oscB.out.nextSample[voice];

    for (uint32_t sample = 0; sample < samples; sample++) {

        float noiseSample = getNoiseSample(voice);
        float subSample = getSubSample(voice);
        float oscASample = getOscASample(voice);
        float oscBSample = getOscBSample(voice);

        float sampleSteiner = noiseSample * noiseLevelSteiner;
        float sampleLadder = noiseSample * noiseLevelLadder;

        sampleSteiner += subSample * subLevelSteiner;
        sampleLadder += subSample * subLevelLadder;

        sampleSteiner += oscASample * oscALevelSteiner;
        sampleLadder += oscASample * oscALevelLadder;

        sampleSteiner += oscBSample * oscBLevelSteiner;
        sampleLadder += oscBSample * oscBLevelLadder;

        int32_t intSampleSteiner = convertAudioSample(sampleSteiner * MAXVOLUMEPERMODULE);
        int32_t intSampleLadder = convertAudioSample(sampleLadder * MAXVOLUMEPERMODULE);

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
    // render voice 1
    renderVoice(renderDest, samples, 0, 2, 3);
    // render voice 2
    renderVoice(renderDest, samples, 1, 0, 1);
    // render voice 3
    renderVoice(renderDest, samples, 2, 4, 5);
    // render voice 4
    renderVoice(renderDest, samples, 3, 6, 7);
}

// void renderAudio(int32_t *renderDest, uint32_t samples, uint32_t channels) {

//     // static const wavetable *currentWavetable1 = &wavetable_strings01;
//     static float stepWavetable1[8] = {0};
//     // static const wavetable *currentWavetable2 = &wavetable_nylonGuitar01;
//     static float stepWavetable2[8] = {0};

//     float balance = fastMapCached(layerA.adsrA.aAttack.valueMapped, 0, 10, 0, 1);

//     for (uint32_t i = 0; i < samples * channels; i += channels) {

//         for (uint32_t chan = 0; chan < channels; chan++) {

//             while (stepWavetable1[chan] >= (float)wabetableASize)
//                 stepWavetable1[chan] -= (float)wabetableASize;
//             while (stepWavetable2[chan] >= (float)wabetableBSize)
//                 stepWavetable2[chan] -= (float)wabetableBSize;

//             float audioSample = wavetableA[(uint32_t)stepWavetable1[chan]] * balance +
//                                 wavetableB[(uint32_t)stepWavetable2[chan]] * (1 - balance);

//             renderDest[i + chan] = convert(audioSample);

//             float step = fastNoteLin2Log_f32((float)chan * layerA.test.aFreq.valueMapped);
//             stepWavetable1[chan] += step;
//             stepWavetable2[chan] += step;
//         }
//     }
// }

#endif