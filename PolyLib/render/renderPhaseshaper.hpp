#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

void renderPhaseshaper(Phaseshaper &phaseshaper, const RenderBuffer &effectAmt);

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
    sample = max(sample, 0.9999999f); // for safety agains rounding error

    return faster_lerp_f32(input, sample, phaseshaper.DryWet);
}

/**
 * @brief returns single sample
 *
 * @param input phase between 0 and 1
 * @param layer current layer
 * @return float
 */

inline float renderPhaseshaperSample(const float &input, const Phaseshaper &phaseshaper) {
    float sample;

    if (input < phaseshaper.Point2X[0]) {
        sample = phaseshaper.Point1Y[0] + phaseshaper.slope[0][0] * input;
    }
    else if (input < phaseshaper.Point3X[0]) {
        sample = phaseshaper.Point2Y[0] + phaseshaper.slope[1][0] * (input - phaseshaper.Point2X[0]);
    }
    else {
        sample = phaseshaper.Point3Y[0] + phaseshaper.slope[2][0] * (input - phaseshaper.Point3X[0]);
    }

    sample = fminf(sample, 0.9999999f); // for safety agains rounding error

    return faster_lerp_f32(input, sample, phaseshaper.DryWet[0]);
}

#endif
