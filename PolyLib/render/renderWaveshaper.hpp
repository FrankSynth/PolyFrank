#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

void renderWaveshaper(Waveshaper &waveshaper, RenderBuffer &effectAmt, uint8_t voice);

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return vec<VOICESPERCHIP>
 */
inline vec<VOICESPERCHIP> renderWaveshaperSample(const vec<VOICESPERCHIP> &input, const Waveshaper &waveshaper) {
    vec<VOICESPERCHIP> inputAbs = fabs(input);

    vec<VOICESPERCHIP> sample;

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        sample[voice] = waveshaper.wavespline[voice].getValueWithoutExtrapolation(inputAbs[voice]);
    }
    sample *= getSign(input);

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