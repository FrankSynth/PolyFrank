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

    float *shapeOut = sub.shape.nextSample;
    float *outLevelSteiner = sub.levelSteiner.nextSample;
    float *outLevelLadder = sub.levelLadder.nextSample;
    float *outBitcrusher = sub.bitcrusher.nextSample;
    int32_t &filterSwitch = sub.dVcfDestSwitch.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        shapeOut[voice] = accumulateShape(sub, voice);
        outBitcrusher[voice] = accumulateBitcrusher(sub, voice);
        float level = accumulateLevel(sub, voice);

        switch (filterSwitch) {
            case 0:
                outLevelSteiner[voice] = level;
                outLevelLadder[voice] = 0;
                break;
            case 1:
                outLevelSteiner[voice] = 0;
                outLevelLadder[voice] = level;
                break;
            case 2:
                outLevelSteiner[voice] = level;
                outLevelLadder[voice] = level;
                break;
            case 3:
                outLevelSteiner[voice] = 0;
                outLevelLadder[voice] = 0;
                break;
        }
    }
}

#endif