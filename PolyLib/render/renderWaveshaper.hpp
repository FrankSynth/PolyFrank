#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

void renderWaveshaper(Waveshaper &waveshaper, uint8_t voice);

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return vec<VOICESPERCHIP>
 */
inline vec<VOICESPERCHIP> renderWaveshaperSample(const vec<VOICESPERCHIP> &input, const Waveshaper &waveshaper) {
    vec<VOICESPERCHIP> sign = getSign(input);
    vec<VOICESPERCHIP> inputAbs = input * sign;

    vec<VOICESPERCHIP> sample;

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        sample[voice] = waveshaper.wavespline[0](inputAbs[voice]);
    }
    sample *= sign;

    return (sample * waveshaper.aDryWet) + (input * (1.0f - waveshaper.aDryWet));
}

#endif

/**
 * @brief returns single sample
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return float
 */
float renderWaveshaperSample(float input, const Waveshaper &waveshaper);