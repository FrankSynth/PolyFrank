#ifdef POLYRENDER

#include "renderLadder.hpp"

extern Layer layerA;

inline float accumulateLevel(Ladder &ladder, uint16_t voice) {
    return std::clamp(ladder.iLevel.currentSample[voice] + ladder.aLevel.valueMapped, 0.0f, 1.0f);
}
inline float accumulateCutoff(Ladder &ladder, uint16_t voice) {
    return std::clamp(ladder.iCutoff.currentSample[voice] + ladder.aCutoff.valueMapped +
                          layerA.envF.out.currentSample[voice] * ladder.aADSR.valueMapped,
                      ladder.aCutoff.min, ladder.aCutoff.max);
}
inline float accumulateResonance(Ladder &ladder, uint16_t voice) {
    return std::clamp(ladder.iResonance.currentSample[voice] + ladder.aResonance.valueMapped, 0.0f, 1.0f);
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