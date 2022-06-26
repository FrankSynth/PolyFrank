#ifdef POLYRENDER

#include "renderSub.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateShape(const Sub &sub) {
    return clamp(sub.iShape + sub.aShape, sub.aShape.min, sub.aShape.max);
}
// inline vec<VOICESPERCHIP> accumulateBitcrusher(const Sub &sub) {
//     return clamp(sub.iBitcrusher + sub.aBitcrusher, sub.aBitcrusher.min, sub.aBitcrusher.max);
// }
// inline vec<VOICESPERCHIP> accumulateSamplecrusher(const Sub &sub) {
//     return clamp(sub.iSamplecrusher + sub.aSamplecrusher, sub.aSamplecrusher.min, sub.aSamplecrusher.max);
// }

void renderSub(Sub &sub) {
    sub.shape = accumulateShape(sub);
    sub.phaseLength = 0.5f * !layerA.sub.dOctaveSwitch + 0.25f * layerA.sub.dOctaveSwitch;

    sub.subWavetable =
        clamp(round((sub.oscANote - (sub.dOctaveSwitch + 1)) * ((float)SUBWAVETABLES / 10.0f)), 0.0f,
              (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor
}

#endif