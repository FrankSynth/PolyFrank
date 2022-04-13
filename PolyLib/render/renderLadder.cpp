#ifdef POLYRENDER

#include "renderLadder.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateLevel(Ladder &ladder) {
    return clamp(ladder.iLevel + ladder.aLevel, 0.0f, 1.0f);
}
inline vec<VOICESPERCHIP> accumulateCutoff(Ladder &ladder) {
    return clamp(ladder.iCutoff + ladder.aCutoff + layerA.envF.out * ladder.aADSR, ladder.aCutoff.min,
                 ladder.aCutoff.max);
}
inline vec<VOICESPERCHIP> accumulateResonance(Ladder &ladder) {
    return clamp(ladder.iResonance + ladder.aResonance, 0.0f, 1.0f);
}

void renderLadder(Ladder &ladder) {
    ladder.level = accumulateLevel(ladder);
    ladder.resonance = accumulateResonance(ladder);
    ladder.cutoff = accumulateCutoff(ladder);
}

#endif