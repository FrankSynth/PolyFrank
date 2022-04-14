#ifdef POLYRENDER

#include "renderMixer.hpp"

inline vec<VOICESPERCHIP> accumulateLevelOscA(const Mixer &mixer) {
    return clamp(mixer.iOSCALevel + mixer.aOSCALevel, mixer.aOSCALevel.min, mixer.aOSCALevel.max);
}
inline vec<VOICESPERCHIP> accumulateLevelOscB(const Mixer &mixer) {
    return clamp(mixer.iOSCBLevel + mixer.aOSCBLevel, mixer.aOSCBLevel.min, mixer.aOSCBLevel.max);
}
inline vec<VOICESPERCHIP> accumulateLevelSub(const Mixer &mixer) {
    return clamp(mixer.iSUBLevel + mixer.aSUBLevel, mixer.aSUBLevel.min, mixer.aSUBLevel.max);
}
inline vec<VOICESPERCHIP> accumulateLevelNoise(const Mixer &mixer) {
    return clamp(mixer.iNOISELevel + mixer.aNOISELevel, mixer.aNOISELevel.min, mixer.aNOISELevel.max);
}

void renderMixer(Mixer &mixer) {

    vec<VOICESPERCHIP> oscALevel = accumulateLevelOscA(mixer);
    mixer.oscALevelSteiner = oscALevel * (!(mixer.dOSCADestSwitch & 0b1));
    mixer.oscALevelLadder = oscALevel * (mixer.dOSCADestSwitch == 1 || mixer.dOSCADestSwitch == 2);

    vec<VOICESPERCHIP> oscBLevel = accumulateLevelOscB(mixer);
    mixer.oscBLevelSteiner = oscBLevel * (!(mixer.dOSCBDestSwitch & 0b1));
    mixer.oscBLevelLadder = oscBLevel * (mixer.dOSCBDestSwitch == 1 || mixer.dOSCBDestSwitch == 2);

    vec<VOICESPERCHIP> subLevel = accumulateLevelSub(mixer);
    mixer.subLevelSteiner = subLevel * (!(mixer.dSUBDestSwitch & 0b1));
    mixer.subLevelLadder = subLevel * (mixer.dSUBDestSwitch == 1 || mixer.dSUBDestSwitch == 2);

    vec<VOICESPERCHIP> noiseLevel = accumulateLevelNoise(mixer);
    mixer.noiseLevelSteiner = noiseLevel * (!(mixer.dNOISEDestSwitch & 0b1));
    mixer.noiseLevelLadder = noiseLevel * (mixer.dNOISEDestSwitch == 1 || mixer.dNOISEDestSwitch == 2);

    // switch (oscAFilterSwitch) {
    //     case 0:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelSteiner[voice] = oscALevel[voice];
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelLadder[voice] = 0;
    //         break;
    //     case 1:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelSteiner[voice] = 0;
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelLadder[voice] = oscALevel[voice];
    //         break;
    //     case 2:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelSteiner[voice] = oscALevel[voice];
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelLadder[voice] = oscALevel[voice];
    //         break;
    //     case 3:
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelSteiner[voice] = 0;
    //         for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
    //             oscALevelLadder[voice] = 0;
    //         break;
    // }
}

#endif