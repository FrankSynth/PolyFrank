#ifdef POLYRENDER

#include "renderSteiner.hpp"

extern Layer layerA;
LogCurve SteinerlinAntiLog(64, 0.99);

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

    steiner.levelRAW = level;

    steiner.level = SteinerlinAntiLog.mapValue(level * (1.0f - steiner.aParSer));
    steiner.resonance = accumulateResonance(steiner);
    steiner.cutoff = accumulateCutoff(steiner);
    steiner.toLadder = SteinerlinAntiLog.mapValue(level * steiner.aParSer);
}

#endif