#ifdef POLYRENDER

#include "renderMixer.hpp"

inline float accumulateLevelOscA(Mixer &mixer, uint16_t voice) {
    return testFloat(mixer.iOSCALevel.currentSample[voice] + mixer.aOSCALevel.valueMapped, mixer.aOSCALevel.min,
                     mixer.aOSCALevel.max);
}
inline float accumulateLevelOscB(Mixer &mixer, uint16_t voice) {
    return testFloat(mixer.iOSCBLevel.currentSample[voice] + mixer.aOSCBLevel.valueMapped, mixer.aOSCBLevel.min,
                     mixer.aOSCBLevel.max);
}
inline float accumulateLevelSub(Mixer &mixer, uint16_t voice) {
    return testFloat(mixer.iSUBLevel.currentSample[voice] + mixer.aSUBLevel.valueMapped, mixer.aSUBLevel.min,
                     mixer.aSUBLevel.max);
}
inline float accumulateLevelNoise(Mixer &mixer, uint16_t voice) {
    return testFloat(mixer.iNOISELevel.currentSample[voice] + mixer.aNOISELevel.valueMapped, mixer.aNOISELevel.min,
                     mixer.aNOISELevel.max);
}

void renderMixer(Mixer &mixer) {
    float *oscALevelSteiner = mixer.oscALevelSteiner.nextSample;
    float *oscALevelLadder = mixer.oscALevelLadder.nextSample;
    float *oscBLevelSteiner = mixer.oscBLevelSteiner.nextSample;
    float *oscBLevelLadder = mixer.oscBLevelLadder.nextSample;
    float *subLevelSteiner = mixer.subLevelSteiner.nextSample;
    float *subLevelLadder = mixer.subLevelLadder.nextSample;
    float *noiseLevelSteiner = mixer.noiseLevelSteiner.nextSample;
    float *noiseLevelLadder = mixer.noiseLevelLadder.nextSample;

    int32_t &oscAFilterSwitch = mixer.dOSCADestSwitch.valueMapped;
    int32_t &oscBFilterSwitch = mixer.dOSCBDestSwitch.valueMapped;
    int32_t &SubFilterSwitch = mixer.dSUBDestSwitch.valueMapped;
    int32_t &noiseFilterSwitch = mixer.dNOISEDestSwitch.valueMapped;

    float oscALevel[VOICESPERCHIP];
    float oscBLevel[VOICESPERCHIP];
    float subLevel[VOICESPERCHIP];
    float noiseLevel[VOICESPERCHIP];

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        oscALevel[voice] = accumulateLevelOscA(mixer, voice);

    switch (oscAFilterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelSteiner[voice] = oscALevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelLadder[voice] = oscALevel[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelSteiner[voice] = oscALevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelLadder[voice] = oscALevel[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscALevelLadder[voice] = 0;
            break;
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        oscBLevel[voice] = accumulateLevelOscB(mixer, voice);

    switch (oscBFilterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelSteiner[voice] = oscBLevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelLadder[voice] = oscBLevel[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelSteiner[voice] = oscBLevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelLadder[voice] = oscBLevel[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                oscBLevelLadder[voice] = 0;
            break;
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        subLevel[voice] = accumulateLevelSub(mixer, voice);

    switch (SubFilterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelSteiner[voice] = subLevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelLadder[voice] = subLevel[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelSteiner[voice] = subLevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelLadder[voice] = subLevel[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                subLevelLadder[voice] = 0;
            break;
    }

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        noiseLevel[voice] = accumulateLevelNoise(mixer, voice);

    switch (noiseFilterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelSteiner[voice] = noiseLevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelLadder[voice] = noiseLevel[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelSteiner[voice] = noiseLevel[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelLadder[voice] = noiseLevel[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                noiseLevelLadder[voice] = 0;
            break;
    }
}

#endif