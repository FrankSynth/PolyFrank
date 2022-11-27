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
inline void renderWaveshaperSample(vec<VOICESPERCHIP> &input, const Waveshaper &waveshaper) {
    vec<VOICESPERCHIP> inputAbs = fabs(input);

    vec<VOICESPERCHIP> sample;

    for (int32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        sample[voice] = waveshaper.wavespline[voice].getValueWithoutExtrapolation(inputAbs[voice]);
    }
    sample *= getSign(input);

    input = (sample * waveshaper.DryWet) + (input * (((vec<VOICESPERCHIP>)waveshaper.DryWet * (-1.0f)) + 1.0f));
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