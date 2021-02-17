#ifdef POLYRENDER

#include "renderLadder.hpp"

#define INPUTWEIGHTING 1.0f

inline float accumulateLevel(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iLevel.currentSample[voice] + ladder.aLevel.valueMapped, 0, 1);
}
inline float accumulateCutoff(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iCutoff.currentSample[voice] * ladder.aCutoff.valueMapped * INPUTWEIGHTING +
                         ladder.aCutoff.valueMapped,
                     ladder.aCutoff.min, ladder.aCutoff.max);
}
inline float accumulateResonance(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iResonance.currentSample[voice] * ladder.aResonance.valueMapped * INPUTWEIGHTING +
                         ladder.aResonance.valueMapped,
                     0, 1);
}

void renderLadder(Ladder &ladder) {
    static float *outLevel = ladder.level.nextSample;
    static float *outResonance = ladder.resonance.nextSample;
    static float *outCutoff = ladder.cutoff.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outLevel[voice] = accumulateLevel(ladder, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outResonance[voice] = accumulateResonance(ladder, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outCutoff[voice] = accumulateCutoff(ladder, voice) / 20000.0f;
}

#endif