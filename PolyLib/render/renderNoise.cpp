#ifdef POLYRENDER

#include "renderNoise.hpp"
#include "rng.h"

// inline float accumulateLevel(Noise &noise, uint16_t voice) {
//     return testFloat(noise.iLevel.currentSample[voice] + noise.aLevel.valueMapped, noise.aLevel.min,
//     noise.aLevel.max);
// }
// inline float accumulateBitcrusher(Noise &noise, uint16_t voice) {
//     return testFloat(noise.iBitcrusher.currentSample[voice] + noise.aBitcrusher.valueMapped, noise.aBitcrusher.min,
//                      noise.aBitcrusher.max);
// }
inline float accumulateSamplecrusher(Noise &noise, uint16_t voice) {
    return testFloat(noise.iSamplecrusher.currentSample[voice] + noise.aSamplecrusher.valueMapped,
                     noise.aSamplecrusher.min, noise.aSamplecrusher.max);
}

void renderNoise(Noise &noise) {
    // float *outLevelSteiner = noise.levelSteiner.nextSample;
    // float *outLevelLadder = noise.levelLadder.nextSample;
    // float *outBitcrusher = noise.bitcrusher.nextSample;
    float *outSamplecrusher = noise.samplecrusher.nextSample;
    // int32_t &filterSwitch = noise.dVcfDestSwitch.valueMapped;

    // float level[VOICESPERCHIP];

    // for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //     level[voice] = accumulateLevel(noise, voice);
    // for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //     outBitcrusher[voice] = accumulateBitcrusher(noise, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(noise, voice);

    // switch (filterSwitch) {
    //     case 0:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelSteiner[voice] = level[voice];
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelLadder[voice] = 0;
    //         break;
    //     case 1:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelSteiner[voice] = 0;
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelLadder[voice] = level[voice];
    //         break;
    //     case 2:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelSteiner[voice] = level[voice];
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelLadder[voice] = level[voice];
    //         break;
    //     case 3:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelSteiner[voice] = 0;
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             outLevelLadder[voice] = 0;
    //         break;
    // }
}

#endif