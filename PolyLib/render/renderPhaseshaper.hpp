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

    vec<VOICESPERCHIP> lowerBoundY;
    vec<VOICESPERCHIP> upperBoundY;
    vec<VOICESPERCHIP> lerpAmount;

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (input[voice] < phaseshaper.Point2X[voice]) {
            lowerBoundY[voice] = phaseshaper.Point1Y[voice];
            upperBoundY[voice] = phaseshaper.Point2Y[voice];
            lerpAmount[voice] = input[voice] / phaseshaper.Point2X[voice];
        }
        else if (input[voice] < phaseshaper.Point3X[voice]) {
            lowerBoundY[voice] = phaseshaper.Point2Y[voice];
            upperBoundY[voice] = phaseshaper.Point3Y[voice];
            lerpAmount[voice] =
                (input[voice] - phaseshaper.Point2X[voice]) / (phaseshaper.Point3X[voice] - phaseshaper.Point2X[voice]);
        }
        else {
            lowerBoundY[voice] = phaseshaper.Point3Y[voice];
            upperBoundY[voice] = phaseshaper.Point4Y[voice];
            lerpAmount[voice] = (input[voice] - phaseshaper.Point3X[voice]) / (1.0f - phaseshaper.Point3X[voice]);
        }
    }

    vec<VOICESPERCHIP> sample = faster_lerp_f32(lowerBoundY, upperBoundY, lerpAmount);

    return (sample * phaseshaper.DryWet) + (input * (((vec<VOICESPERCHIP>)phaseshaper.DryWet * (-1.0f)) + 1.0f));
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