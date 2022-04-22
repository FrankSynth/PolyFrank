
#include "renderWaveshaper.hpp"
#include "math/polyMath.hpp"

#ifdef POLYRENDER

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateValue(const Input &input, const Analog &knob) {
    return clamp(input + knob, knob.min, knob.max);
}
inline vec<VOICESPERCHIP> accumulateX1(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint1X + waveshaper.aPoint1X, waveshaper.aPoint1X.min, waveshaper.aPoint2X - 0.001f);
}
inline vec<VOICESPERCHIP> accumulateX2(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint2X + waveshaper.aPoint2X, waveshaper.aPoint1X + 0.001f, waveshaper.aPoint3X - 0.001f);
}
inline vec<VOICESPERCHIP> accumulateX3(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint3X + waveshaper.aPoint3X, waveshaper.aPoint2X + 0.001f, waveshaper.aPoint3X.max);
}

void renderWaveshaper(Waveshaper &waveshaper) {
    waveshaper.shape1 = accumulateValue(waveshaper.iShape1, waveshaper.aShape1);
    waveshaper.shape2 = accumulateValue(waveshaper.iShape2, waveshaper.aShape2);
    waveshaper.shape3 = accumulateValue(waveshaper.iShape3, waveshaper.aShape3);
    waveshaper.shape4 = accumulateValue(waveshaper.iShape4, waveshaper.aShape4);
    waveshaper.Point1X = accumulateX1(waveshaper);
    waveshaper.Point1Y = accumulateValue(waveshaper.iPoint1Y, waveshaper.aPoint1Y);
    waveshaper.Point2X = accumulateX2(waveshaper);
    waveshaper.Point2Y = accumulateValue(waveshaper.iPoint2Y, waveshaper.aPoint2Y);
    waveshaper.Point3X = accumulateX3(waveshaper);
    waveshaper.Point3Y = accumulateValue(waveshaper.iPoint3Y, waveshaper.aPoint3Y);
    waveshaper.DryWet = accumulateValue(waveshaper.iDryWet, waveshaper.aDryWet);
}

vec<VOICESPERCHIP> renderWaveshaperSample(const vec<VOICESPERCHIP> &input) {
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

float renderWaveshaperSample(float input, const Layer &layer) {
    float sign = getSign(input);
    float inputAbs = input * sign;
    float lowerBoundX;
    float lowerBoundY;
    float upperBoundX;
    float upperBoundY;
    float shape;

    if (inputAbs < layer.waveshaper.Point1X[0]) {
        lowerBoundX = 0.0f;
        lowerBoundY = 0.0f;
        upperBoundX = layer.waveshaper.Point1X[0];
        upperBoundY = layer.waveshaper.Point1Y[0];
        shape = layer.waveshaper.shape1[0];
    }
    else if (inputAbs < layer.waveshaper.Point2X[0]) {
        lowerBoundX = layer.waveshaper.Point1X[0];
        lowerBoundY = layer.waveshaper.Point1Y[0];
        upperBoundX = layer.waveshaper.Point2X[0];
        upperBoundY = layer.waveshaper.Point2Y[0];
        shape = layer.waveshaper.shape2[0];
    }
    else if (inputAbs < layer.waveshaper.Point3X[0]) {
        lowerBoundX = layer.waveshaper.Point2X[0];
        lowerBoundY = layer.waveshaper.Point2Y[0];
        upperBoundX = layer.waveshaper.Point3X[0];
        upperBoundY = layer.waveshaper.Point3Y[0];
        shape = layer.waveshaper.shape3[0];
    }
    else {
        lowerBoundX = layer.waveshaper.Point3X[0];
        lowerBoundY = layer.waveshaper.Point3Y[0];
        upperBoundX = 1.0f;
        upperBoundY = 1.0f;
        shape = layer.waveshaper.shape4[0];
    }

    float sample = fastMap(inputAbs * sign, lowerBoundX, upperBoundX, 0.0f, 1.0f);
    sample = calcSquircleSimplified(sample, shape);
    sample = fastMap(sample, 0.0f, 1.0f, lowerBoundY, upperBoundY) * sign;

    return (sample * layer.waveshaper.aDryWet) + (input * (1.0f - layer.waveshaper.aDryWet));
}
