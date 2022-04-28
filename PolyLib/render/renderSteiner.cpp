#ifdef POLYRENDER

#include "renderSteiner.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateLevel(const Steiner &steiner) {
    return clamp(steiner.iLevel + steiner.aLevel, 0.0f, 1.0f);
}
inline vec<VOICESPERCHIP> accumulateCutoff(const Steiner &steiner) {
    return clamp(steiner.iCutoff + steiner.aCutoff, steiner.aCutoff.min, steiner.aCutoff.max);
}
inline vec<VOICESPERCHIP> accumulateResonance(const Steiner &steiner) {
    return clamp(steiner.iResonance + steiner.aResonance, 0.0f, 1.0f);
}

void renderSteiner(Steiner &steiner) {

    // TODO log fuer blend noeting, wie im panning?

    vec<VOICESPERCHIP> level = accumulateLevel(steiner);

    steiner.level = level * (1.0f - steiner.aParSer);
    steiner.resonance = accumulateResonance(steiner);
    steiner.cutoff = accumulateCutoff(steiner);
    steiner.toLadder = level * steiner.aParSer;
}

#endif