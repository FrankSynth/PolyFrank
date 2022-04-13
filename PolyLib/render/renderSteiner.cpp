#ifdef POLYRENDER

#include "renderSteiner.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateLevel(Steiner &steiner) {
    return clamp(steiner.iLevel + steiner.aLevel, 0.0f, 1.0f);
}
inline vec<VOICESPERCHIP> accumulateCutoff(Steiner &steiner) {
    return clamp(steiner.iCutoff + steiner.aCutoff + layerA.envF.out * steiner.aADSR, steiner.aCutoff.min,
                 steiner.aCutoff.max);
}
inline vec<VOICESPERCHIP> accumulateResonance(Steiner &steiner) {
    return clamp(steiner.iResonance + steiner.aResonance, 0.0f, 1.0f);
}

void renderSteiner(Steiner &steiner) {

    steiner.level = accumulateLevel(steiner);
    steiner.resonance = accumulateResonance(steiner);
    steiner.cutoff = accumulateCutoff(steiner);
    steiner.toLadder = steiner.aParSer;
}

#endif