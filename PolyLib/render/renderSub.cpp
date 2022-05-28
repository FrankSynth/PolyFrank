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
    // sub.bitcrusher = accumulateBitcrusher(sub);
    // sub.samplecrusher = accumulateSamplecrusher(sub);
}

#endif