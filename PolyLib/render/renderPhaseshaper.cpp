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
