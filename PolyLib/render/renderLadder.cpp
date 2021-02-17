#ifdef POLYRENDER

#include "renderLadder.hpp"

#define INPUTWEIGHTING 1.0f

inline float accumulateLevel(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iLevel.currentSample[voice] + ladder.aLevel.valueMapped, 0, 1);
}
inline float accumulateCutoff(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iCutoff.currentSample[voice] * INPUTWEIGHTING + ladder.aCutoff.valueMapped,
                     ladder.aCutoff.min, ladder.aCutoff.max);
}
inline float accumulateResonance(Ladder &ladder, uint16_t voice) {
    return testFloat(ladder.iResonance.currentSample[voice] * INPUTWEIGHTING + ladder.aResonance.valueMapped, 0, 1);
}

void renderLadder(Ladder &ladder) {
    float *outLevel = ladder.level.nextSample;
    float *outResonance = ladder.resonance.nextSample;
    float *outCutoff = ladder.cutoff.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outLevel[voice] = accumulateLevel(ladder, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outResonance[voice] = accumulateResonance(ladder, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outCutoff[voice] = accumulateCutoff(ladder, voice);
}

#endif