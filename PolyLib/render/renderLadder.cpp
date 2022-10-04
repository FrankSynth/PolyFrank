#ifdef POLYRENDER

#include "renderLadder.hpp"

extern Layer layerA;
LogCurve LadderlinAntiLog(64, 0.9);

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
    ladder.levelRAW = accumulateLevel(ladder);
    ladder.level = LadderlinAntiLog.mapValue((vec<VOICESPERCHIP>)ladder.levelRAW);
    ladder.resonance = accumulateResonance(ladder);
    ladder.cutoff = accumulateCutoff(ladder);
}

#endif