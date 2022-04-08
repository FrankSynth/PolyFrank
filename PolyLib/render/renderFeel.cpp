#ifdef POLYRENDER

#include "renderFeel.hpp"

extern Layer layerA;

inline float accumulateGlide(Feel &feel, uint16_t voice) {
    return std::clamp(feel.iGlide.currentSample[voice] + feel.aGlide.valueMapped, feel.aGlide.min, feel.aGlide.max);
}

inline float accumulateDetune(Feel &feel, uint16_t voice) {
    return std::clamp(feel.iDetune.currentSample[voice] + feel.aDetune.valueMapped, feel.aDetune.min, feel.aDetune.max);
}

void renderFeel(Feel &feel) {

    float *SpreadNextSample = feel.oSpread.nextSample;

    float *glideNextSample = feel.glide.nextSample;
    float *detuneNextSample = feel.detune.nextSample;

    float accumulatedDetune[VOICESPERCHIP];

    // glide
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        glideNextSample[i] = accumulateGlide(feel, i);

    // detune
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        accumulatedDetune[i] = accumulateDetune(feel, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        detuneNextSample[i] = accumulatedDetune[i] * layerA.spreadValues[i];

    // spread
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        SpreadNextSample[i] = layerA.spreadValues[i] * layerA.feel.aSpread.valueMapped;
}

#endif