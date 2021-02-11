#ifdef POLYRENDER

#include "renderLadder.hpp"

#define INPUTWEIGHTING 1

inline float accumulateLevel(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iLevel.currentSample[voice] + ladder.aLevel.valueMapped, ladder.aLevel.min,
                     ladder.aLevel.max);
}
inline float accumulateCutoff(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iCutoff.currentSample[voice] * ladder.aCutoff.valueMapped * INPUTWEIGHTING +
                         ladder.aCutoff.valueMapped,
                     ladder.aCutoff.min, ladder.aCutoff.max);
}
inline float accumulateResonance(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iResonance.currentSample[voice] * ladder.aResonance.valueMapped * INPUTWEIGHTING +
                         ladder.aResonance.valueMapped,
                     ladder.aResonance.min, ladder.aResonance.max);
}

void renderLadder(Ladder &ladder) {
    float *outLevel = ladder.level.nextSample;
    float *outResonance = ladder.resonance.nextSample;
    float *outCutoff = ladder.cutoff.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        outLevel[voice] = accumulateLevel(ladder, voice);
        outResonance[voice] = accumulateResonance(ladder, voice);
        outCutoff[voice] = accumulateCutoff(ladder, voice);
    }
}

#endif