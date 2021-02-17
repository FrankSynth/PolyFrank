#ifdef POLYRENDER

#include "renderSub.hpp"

extern Layer layerA;

inline float accumulateLevel(Sub &sub, uint16_t voice) {
    return testFloat(sub.iLevel.currentSample[voice] + sub.aLevel.valueMapped, sub.aLevel.min, sub.aLevel.max);
}
inline float accumulateShape(Sub &sub, uint16_t voice) {
    return testFloat(sub.iShape.currentSample[voice] + sub.aShape.valueMapped, sub.aShape.min, sub.aShape.max);
}
inline float accumulateBitcrusher(Sub &sub, uint16_t voice) {
    return testFloat(sub.iBitcrusher.currentSample[voice] + sub.aBitcrusher.valueMapped, sub.aBitcrusher.min,
                     sub.aBitcrusher.max);
}

void renderSub(Sub &sub) {

    static float *shapeOut = sub.shape.nextSample;
    static float *outLevelSteiner = sub.levelSteiner.nextSample;
    static float *outLevelLadder = sub.levelLadder.nextSample;
    static float *outBitcrusher = sub.bitcrusher.nextSample;
    static int32_t &filterSwitch = sub.dVcfDestSwitch.valueMapped;

    float level[VOICESPERCHIP];

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        shapeOut[voice] = accumulateShape(sub, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(sub, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        level[voice] = accumulateLevel(sub, voice);

    switch (filterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = level[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = level[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = level[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = level[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = 0;
            break;
    }
}

#endif