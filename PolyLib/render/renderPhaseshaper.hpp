#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

void renderPhaseshaper(Phaseshaper &phaseshaper, RenderBuffer &effectAmt);

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input phase between 0 and 1
 * @return vec<VOICESPERCHIP>
 */
inline vec<VOICESPERCHIP> renderPhaseshaperSample(const vec<VOICESPERCHIP> &input, const Phaseshaper &phaseshaper) {
    vec<VOICESPERCHIP> sample;

    for (int32_t voice = 0; voice < VOICESPERCHIP; voice++) {

        if (input[voice] < phaseshaper.Point2X[voice]) {
            sample[voice] = phaseshaper.Point1Y[voice] + phaseshaper.slope[0][voice] * input[voice];
        }
        else if (input[voice] < phaseshaper.Point3X[voice]) {
            sample[voice] =
                phaseshaper.Point2Y[voice] + phaseshaper.slope[1][voice] * (input[voice] - phaseshaper.Point2X[voice]);
        }
        else {
            sample[voice] =
                phaseshaper.Point3Y[voice] + phaseshaper.slope[2][voice] * (input[voice] - phaseshaper.Point3X[voice]);
        }
    }

    return faster_lerp_f32(input, sample, phaseshaper.DryWet);
}
#endif

/**
 * @brief returns single sample
 *
 * @param input phase between 0 and 1
 * @param layer current layer
 * @return float
 */
float renderPhaseshaperSample(float input, const Phaseshaper &phaseshaper);