#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

extern Layer layerA;

void renderWaveshaper(Waveshaper &waveshaper);

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return vec<VOICESPERCHIP>
 */
inline vec<VOICESPERCHIP> renderWaveshaperSample(const vec<VOICESPERCHIP> &input) {
    vec<VOICESPERCHIP> sign = getSign(input);
    vec<VOICESPERCHIP> inputAbs = input * sign;

    vec<VOICESPERCHIP> lowerBoundX;
    vec<VOICESPERCHIP> lowerBoundY;
    vec<VOICESPERCHIP> upperBoundX;
    vec<VOICESPERCHIP> upperBoundY;
    vec<VOICESPERCHIP> shape;

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (inputAbs[voice] < layerA.waveshaper.Point1X[voice]) {
            lowerBoundX[voice] = 0.0f;
            lowerBoundY[voice] = 0.0f;
            upperBoundX[voice] = layerA.waveshaper.Point1X[voice];
            upperBoundY[voice] = layerA.waveshaper.Point1Y[voice];
            shape[voice] = layerA.waveshaper.shape1[voice];
        }
        else if (inputAbs[voice] < layerA.waveshaper.Point2X[voice]) {
            lowerBoundX[voice] = layerA.waveshaper.Point1X[voice];
            lowerBoundY[voice] = layerA.waveshaper.Point1Y[voice];
            upperBoundX[voice] = layerA.waveshaper.Point2X[voice];
            upperBoundY[voice] = layerA.waveshaper.Point2Y[voice];
            shape[voice] = layerA.waveshaper.shape2[voice];
        }
        else if (inputAbs[voice] < layerA.waveshaper.Point3X[voice]) {
            lowerBoundX[voice] = layerA.waveshaper.Point2X[voice];
            lowerBoundY[voice] = layerA.waveshaper.Point2Y[voice];
            upperBoundX[voice] = layerA.waveshaper.Point3X[voice];
            upperBoundY[voice] = layerA.waveshaper.Point3Y[voice];
            shape[voice] = layerA.waveshaper.shape3[voice];
        }
        else {
            lowerBoundX[voice] = layerA.waveshaper.Point3X[voice];
            lowerBoundY[voice] = layerA.waveshaper.Point3Y[voice];
            upperBoundX[voice] = 1.0f;
            upperBoundY[voice] = 1.0f;
            shape[voice] = layerA.waveshaper.shape4[voice];
        }
    }

    vec<VOICESPERCHIP> sample = fastMap(inputAbs, lowerBoundX, upperBoundX, 0.0f, 1.0f);
    sample = calcSquircleSimplified(sample, shape);
    sample = fastMap(sample, 0.0f, 1.0f, lowerBoundY, upperBoundY) * sign;

    return (sample * layerA.waveshaper.aDryWet) + (input * (1.0f - layerA.waveshaper.aDryWet));
}

#endif

/**
 * @brief returns single sample
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return float
 */
float renderWaveshaperSample(float input, const Layer &layer);