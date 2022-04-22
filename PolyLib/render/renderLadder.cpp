#ifdef POLYRENDER

#include "renderLadder.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateLevel(const Ladder &ladder) {
    return clamp(ladder.iLevel + ladder.aLevel, 0.0f, 1.0f);
}
inline vec<VOICESPERCHIP> accumulateCutoff(const Ladder &ladder) {
    return clamp(ladder.iCutoff + ladder.aCutoff, ladder.aCutoff.min, ladder.aCutoff.max);
}
inline vec<VOICESPERCHIP> accumulateResonance(const Ladder &ladder) {
    return clamp(ladder.iResonance + ladder.aResonance, 0.0f, 1.0f);
}

void renderLadder(Ladder &ladder) {
    ladder.level = accumulateLevel(ladder);
    ladder.resonance = accumulateResonance(ladder);
    ladder.cutoff = accumulateCutoff(ladder);
}

#endif