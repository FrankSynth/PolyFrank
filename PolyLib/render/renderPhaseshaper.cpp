#include "math/polyMath.hpp"
#include "renderWaveshaper.hpp"

#ifdef POLYRENDER

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateValue(const Input &input, const Analog &knob) {
    return clamp(input + knob, knob.min, knob.max);
}
inline vec<VOICESPERCHIP> accumulateX2(const Phaseshaper &phaseshaper) {
    return clamp(phaseshaper.iPoint2X + phaseshaper.aPoint2X, phaseshaper.aPoint2X.min, phaseshaper.aPoint3X);
}
inline vec<VOICESPERCHIP> accumulateX3(const Phaseshaper &phaseshaper) {
    return clamp(phaseshaper.iPoint3X + phaseshaper.aPoint3X, phaseshaper.aPoint2X, phaseshaper.aPoint3X.max);
}

void renderPhaseshaper(Phaseshaper &phaseshaper) {
    phaseshaper.Point1Y = accumulateValue(phaseshaper.iPoint1Y, phaseshaper.aPoint1Y);
    phaseshaper.Point2X = accumulateX2(phaseshaper);
    phaseshaper.Point2Y = accumulateValue(phaseshaper.iPoint2Y, phaseshaper.aPoint2Y);
    phaseshaper.Point3X = accumulateX3(phaseshaper);
    phaseshaper.Point3Y = accumulateValue(phaseshaper.iPoint3Y, phaseshaper.aPoint3Y);
    phaseshaper.Point4Y = accumulateValue(phaseshaper.iPoint4Y, phaseshaper.aPoint4Y);
    phaseshaper.DryWet = accumulateValue(phaseshaper.iDryWet, phaseshaper.aDryWet);
}

vec<VOICESPERCHIP> renderPhaseshaperSample(const vec<VOICESPERCHIP> &input) {

    vec<VOICESPERCHIP> lowerBoundX;
    vec<VOICESPERCHIP> lowerBoundY;
    vec<VOICESPERCHIP> upperBoundX;
    vec<VOICESPERCHIP> upperBoundY;
    vec<VOICESPERCHIP> lerpAmount;

    for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
        if (input[voice] < layerA.phaseshaper.Point2X[voice]) {
            lowerBoundX[voice] = 0.0f;
            lowerBoundY[voice] = layerA.phaseshaper.Point1Y[voice];
            upperBoundX[voice] = layerA.phaseshaper.Point2X[voice];
            upperBoundY[voice] = layerA.phaseshaper.Point2Y[voice];
            lerpAmount[voice] = input[voice] / layerA.phaseshaper.Point2X[voice];
        }
        else if (input[voice] < layerA.phaseshaper.Point3X[voice]) {
            lowerBoundX[voice] = layerA.phaseshaper.Point2X[voice];
            lowerBoundY[voice] = layerA.phaseshaper.Point2Y[voice];
            upperBoundX[voice] = layerA.phaseshaper.Point3X[voice];
            upperBoundY[voice] = layerA.phaseshaper.Point3Y[voice];
            lerpAmount[voice] =
                (input[voice] - lowerBoundX[voice]) / (layerA.phaseshaper.Point3X[voice] - lowerBoundX[voice]);
        }
        else {
            lowerBoundX[voice] = layerA.phaseshaper.Point3X[voice];
            lowerBoundY[voice] = layerA.phaseshaper.Point3Y[voice];
            upperBoundX[voice] = 1.0f;
            upperBoundY[voice] = layerA.phaseshaper.Point4Y[voice];
            lerpAmount[voice] = (input[voice] - lowerBoundX[voice]) / (1.0f - lowerBoundX[voice]);
        }
    }

    vec<VOICESPERCHIP> sample = fast_lerp_f32(lowerBoundY, upperBoundY, lerpAmount);

    return (sample * layerA.phaseshaper.aDryWet) + (input * (1.0f - layerA.phaseshaper.aDryWet));
}
#endif

float renderPhaseshaperSample(float input, const Layer &layer) {
    float lowerBoundY;
    float upperBoundY;
    float lerpAmount;

    if (input < layer.phaseshaper.Point2X[0]) {
        lowerBoundY = layer.phaseshaper.Point1Y[0];
        upperBoundY = layer.phaseshaper.Point2Y[0];
        lerpAmount = input / layer.phaseshaper.Point2X[0];
    }
    else if (input < layer.phaseshaper.Point3X[0]) {
        lowerBoundY = layer.phaseshaper.Point2Y[0];
        upperBoundY = layer.phaseshaper.Point3Y[0];
        lerpAmount =
            (input - layer.phaseshaper.Point2X[0]) / (layer.phaseshaper.Point3X[0] - layer.phaseshaper.Point2X[0]);
    }
    else {
        lowerBoundY = layer.phaseshaper.Point3Y[0];
        upperBoundY = layer.phaseshaper.Point4Y[0];
        lerpAmount = (input - layer.phaseshaper.Point3X[0]) / (1.0f - layer.phaseshaper.Point3X[0]);
    }

    float sample = fast_lerp_f32(lowerBoundY, upperBoundY, lerpAmount);

    return (sample * layer.phaseshaper.aDryWet) + (input * (1.0f - layer.phaseshaper.aDryWet));
}
