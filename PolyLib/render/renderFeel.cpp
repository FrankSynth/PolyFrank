#ifdef POLYRENDER

#include "renderFeel.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateGlide(const Feel &feel) {
    return clamp(feel.iGlide + feel.aGlide, feel.aGlide.min, feel.aGlide.max);
}

inline vec<VOICESPERCHIP> accumulateDetune(const Feel &feel) {
    return clamp(feel.iDetune + feel.aDetune, feel.aDetune.min, feel.aDetune.max);
}

void renderFeel(Feel &feel) {
    feel.glide = accumulateGlide(feel);
    feel.detune = accumulateDetune(feel) * layerA.spreadValues * 0.1f;
    feel.oSpread = layerA.spreadValues * layerA.feel.aSpread;
}

#endif