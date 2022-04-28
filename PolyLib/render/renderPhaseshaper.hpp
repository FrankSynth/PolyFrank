#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

void renderPhaseshaper(Phaseshaper &phaseshaper);

extern Layer layerA;

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input phase between 0 and 1
 * @return vec<VOICESPERCHIP>
 */
inline vec<VOICESPERCHIP> renderPhaseshaperSample(const vec<VOICESPERCHIP> &input) {

    vec<VOICESPERCHIP> lowerBoundY;
    vec<VOICESPERCHIP> upperBoundY;
    vec<VOICESPERCHIP> lerpAmount;

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (input[voice] < layerA.phaseshaper.Point2X[voice]) {
            lowerBoundY[voice] = layerA.phaseshaper.Point1Y[voice];
            upperBoundY[voice] = layerA.phaseshaper.Point2Y[voice];
            lerpAmount[voice] = input[voice] / layerA.phaseshaper.Point2X[voice];
        }
        else if (input[voice] < layerA.phaseshaper.Point3X[voice]) {
            lowerBoundY[voice] = layerA.phaseshaper.Point2Y[voice];
            upperBoundY[voice] = layerA.phaseshaper.Point3Y[voice];
            lerpAmount[voice] = (input[voice] - layerA.phaseshaper.Point2X[voice]) /
                                (layerA.phaseshaper.Point3X[voice] - layerA.phaseshaper.Point2X[voice]);
        }
        else {
            lowerBoundY[voice] = layerA.phaseshaper.Point3Y[voice];
            upperBoundY[voice] = layerA.phaseshaper.Point4Y[voice];
            lerpAmount[voice] =
                (input[voice] - layerA.phaseshaper.Point3X[voice]) / (1.0f - layerA.phaseshaper.Point3X[voice]);
        }
    }

    vec<VOICESPERCHIP> sample = fast_lerp_f32(lowerBoundY, upperBoundY, lerpAmount);

    return (sample * layerA.phaseshaper.aDryWet) + (input * (1.0f - layerA.phaseshaper.aDryWet));
}

#endif

/**
 * @brief returns single sample
 *
 * @param input phase between 0 and 1
 * @param layer current layer
 * @return float
 */
float renderPhaseshaperSample(float input, const Layer &layer);