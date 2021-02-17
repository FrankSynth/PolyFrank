#ifdef POLYRENDER

#include "renderSteiner.hpp"

#define INPUTWEIGHTING 1.0f

inline float accumulateLevel(Steiner &steiner, uint16_t voice) {
    return testFloat(steiner.iLevel.currentSample[voice] + steiner.aLevel.valueMapped, 0, 1);
}
inline float accumulateCutoff(Steiner &steiner, uint16_t voice) {
    return testFloat(steiner.iCutoff.currentSample[voice] * steiner.aCutoff.valueMapped * INPUTWEIGHTING +
                         steiner.aCutoff.valueMapped,
                     steiner.aCutoff.min, steiner.aCutoff.max);
}
inline float accumulateResonance(Steiner &steiner, uint16_t voice) {
    return testFloat(steiner.iResonance.currentSample[voice] * steiner.aResonance.valueMapped * INPUTWEIGHTING +
                         steiner.aResonance.valueMapped,
                     0, 1);
}

void renderSteiner(Steiner &steiner) {
    static float *outLevel = steiner.level.nextSample;
    static float *outResonance = steiner.resonance.nextSample;
    static float *outCutoff = steiner.cutoff.nextSample;
    static float *outToLadder = steiner.toLadder.nextSample;
    // TODO balance with par/ser value

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outLevel[voice] = accumulateLevel(steiner, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outResonance[voice] = accumulateResonance(steiner, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outCutoff[voice] = accumulateCutoff(steiner, voice) / 20000.0f;
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outToLadder[voice] = steiner.aParSer.valueMapped;
}

#endif