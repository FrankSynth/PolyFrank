#ifdef POLYRENDER

#include "renderMixer.hpp"

LogCurve MixlinlogMapping(64, 0.01);

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
    mixer.oscALevel = oscALevel;
    MixlinlogMapping.mapValue(oscALevel);
    mixer.oscALevelSteiner = oscALevel * (!(mixer.dOSCADestSwitch & 0b1));
    mixer.oscALevelLadder = oscALevel * (mixer.dOSCADestSwitch == 1 || mixer.dOSCADestSwitch == 2);

    vec<VOICESPERCHIP> oscBLevel = accumulateLevelOscB(mixer);
    mixer.oscBLevel = oscBLevel;
    MixlinlogMapping.mapValue(oscBLevel);
    mixer.oscBLevelSteiner = oscBLevel * (!(mixer.dOSCBDestSwitch & 0b1));
    mixer.oscBLevelLadder = oscBLevel * (mixer.dOSCBDestSwitch == 1 || mixer.dOSCBDestSwitch == 2);

    vec<VOICESPERCHIP> subLevel = accumulateLevelSub(mixer);
    mixer.subLevel = subLevel;
    MixlinlogMapping.mapValue(subLevel);
    mixer.subLevelSteiner = subLevel * (!(mixer.dSUBDestSwitch & 0b1));
    mixer.subLevelLadder = subLevel * (mixer.dSUBDestSwitch == 1 || mixer.dSUBDestSwitch == 2);

    vec<VOICESPERCHIP> noiseLevel = accumulateLevelNoise(mixer);
    mixer.noiseLevel = noiseLevel;
    MixlinlogMapping.mapValue(noiseLevel);
    mixer.noiseLevelSteiner = noiseLevel * (!(mixer.dNOISEDestSwitch & 0b1));
    mixer.noiseLevelLadder = noiseLevel * (mixer.dNOISEDestSwitch == 1 || mixer.dNOISEDestSwitch == 2);
}

#endif