#ifdef POLYRENDER

#include "renderSub.hpp"

extern Layer layerA;

// inline float accumulateLevel(Sub &sub, uint16_t voice) {
//     return testFloat(sub.iLevel.currentSample[voice] + sub.aLevel.valueMapped, sub.aLevel.min, sub.aLevel.max);
// }
inline float accumulateShape(Sub &sub, uint16_t voice) {
    return testFloat(sub.iShape.currentSample[voice] + sub.aShape.valueMapped, sub.aShape.min, sub.aShape.max);
}
inline float accumulateBitcrusher(Sub &sub, uint16_t voice) {
    return testFloat(sub.iBitcrusher.currentSample[voice] + sub.aBitcrusher.valueMapped, sub.aBitcrusher.min,
                     sub.aBitcrusher.max);
}
inline float accumulateSamplecrusher(Sub &sub, uint16_t voice) {
    return testFloat(sub.iSamplecrusher.currentSample[voice] + sub.aSamplecrusher.valueMapped, sub.aSamplecrusher.min,
                     sub.aSamplecrusher.max);
}

void renderSub(Sub &sub) {

    float *shapeOut = sub.shape.nextSample;
    // float *outLevelSteiner = sub.levelSteiner.nextSample;
    // float *outLevelLadder = sub.levelLadder.nextSample;
    float *outBitcrusher = sub.bitcrusher.nextSample;
    float *outSamplecrusher = sub.samplecrusher.nextSample;
    //  int32_t &filterSwitch = sub.dVcfDestSwitch.valueMapped;

    // float level[VOICESPERCHIP];

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        shapeOut[voice] = accumulateShape(sub, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(sub, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(sub, voice);
    // for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //     level[voice] = accumulateLevel(sub, voice);

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