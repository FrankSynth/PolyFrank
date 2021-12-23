#ifdef POLYRENDER

#include "renderFeel.hpp"

extern Layer layerA;

inline float calcRandom() {
    uint32_t randomNumber;

    randomNumber = std::rand();
    randomNumber = randomNumber & 0x00FFFFFF;

    // map to -1, 1
    return ((float)randomNumber / 8388607.0f) - 1.0f;
}

inline float accumulateGlide(Feel feel, uint16_t voice) {
    return testFloat(feel.iGlide.currentSample[voice] + feel.aGlide.valueMapped, feel.aGlide.min, feel.aGlide.max);
}

inline float accumulateDetune(Feel feel, uint16_t voice) {
    return testFloat(feel.iDetune.currentSample[voice] + feel.aDetune.valueMapped, feel.aDetune.min, feel.aDetune.max);
}

void renderFeel(Feel feel) {

    // static float spread[VOICESPERCHIP] = {calcRandom(), calcRandom(), calcRandom(), calcRandom()}; // temp
    // static float drift[VOICESPERCHIP] = {calcRandom(), calcRandom(), calcRandom(), calcRandom()};  // temp

    float *SpreadNextSample = feel.oSpread.nextSample;
    // float *DriftNextSample = feel.oDrift.nextSample;

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

    // drift
    // for (uint16_t i = 0; i < VOICESPERCHIP; i++)
    //     DriftNextSample[i] =
    //         1 * layerA.feel.aDrift.valueMapped; // temporary , vll ein sehr langsamer sinus (30minuten oder so?)
}

#endif