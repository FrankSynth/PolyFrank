#ifdef POLYRENDER

#include "renderNoise.hpp"
#include "rng.h"

inline float accumulateLevel(Noise &noise, uint16_t voice) {
    return testFloat(noise.iLevel.currentSample[voice] + noise.aLevel.valueMapped, noise.aLevel.min, noise.aLevel.max);
}
inline float accumulateBitcrusher(Noise &noise, uint16_t voice) {
    return testFloat(noise.iBitcrusher.currentSample[voice] + noise.aBitcrusher.valueMapped, noise.aBitcrusher.min,
                     noise.aBitcrusher.max);
}

void renderNoise(Noise &noise) {
    float *outLevelSteiner = noise.levelSteiner.nextSample;
    float *outLevelLadder = noise.levelLadder.nextSample;
    float *outBitcrusher = noise.bitcrusher.nextSample;
    int32_t &filterSwitch = noise.dVcfDestSwitch.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        float level = accumulateLevel(noise, voice);
        outBitcrusher[voice] = accumulateBitcrusher(noise, voice);

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