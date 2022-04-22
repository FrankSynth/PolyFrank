
#include "renderWaveshaper.hpp"
#include "math/polyMath.hpp"

#ifdef POLYRENDER

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateValue(const Input &input, const Analog &knob) {
    return clamp(input + knob, knob.min, knob.max);
}

void renderWaveshaper(Waveshaper &waveshaper) {
    waveshaper.LowerShape = accumulateValue(waveshaper.iLowerShape, waveshaper.aLowerShape);
    waveshaper.UpperShape = accumulateValue(waveshaper.iUpperShape, waveshaper.aUpperShape);
    waveshaper.Point1X = accumulateValue(waveshaper.iPoint1X, waveshaper.aPoint1X);
    waveshaper.Point1Y = accumulateValue(waveshaper.iPoint1Y, waveshaper.aPoint1Y);
    waveshaper.DryWet = accumulateValue(waveshaper.iDryWet, waveshaper.aDryWet);
}

vec<VOICESPERCHIP> renderWaveshaperSample(const vec<VOICESPERCHIP> &input) {
    vec<VOICESPERCHIP> sign = getSign(input);
    vec<VOICESPERCHIP> inputAbs = input * sign;
    vec<VOICESPERCHIP> abovePoint1 = (inputAbs > layerA.waveshaper.Point1X);
    vec<VOICESPERCHIP> lowerBoundX = layerA.waveshaper.Point1X * abovePoint1;
    vec<VOICESPERCHIP> lowerBoundY = layerA.waveshaper.Point1Y * abovePoint1;
    vec<VOICESPERCHIP> upperBoundX = max(layerA.waveshaper.Point1X, abovePoint1);
    vec<VOICESPERCHIP> upperBoundY = max(layerA.waveshaper.Point1Y, abovePoint1);
    vec<VOICESPERCHIP> shape =
        layerA.waveshaper.aLowerShape * !abovePoint1 + layerA.waveshaper.aUpperShape * abovePoint1;

    vec<VOICESPERCHIP> sample = fastMap(inputAbs, lowerBoundX, upperBoundX, 0.0f, 1.0f);
    sample = calcSquircleSimplified(sample, shape);
    sample = fastMap(sample, 0.0f, 1.0f, lowerBoundY, upperBoundY) * sign;

    return (sample * layerA.waveshaper.aDryWet) + (input * (1.0f - layerA.waveshaper.aDryWet));
}
#endif

float renderWaveshaperSample(float input, const Layer &layer) {
    float sign = getSign(input);
    float inputAbs = input * sign;
    float abovePoint1 = (inputAbs > layer.waveshaper.Point1X[0]);
    float lowerBoundX = layer.waveshaper.Point1X[0] * abovePoint1;
    float lowerBoundY = layer.waveshaper.Point1Y[0] * abovePoint1;
    float upperBoundX = std::max(layer.waveshaper.Point1X[0], abovePoint1);
    float upperBoundY = std::max(layer.waveshaper.Point1Y[0], abovePoint1);
    float shape = layer.waveshaper.aLowerShape * !abovePoint1 + layer.waveshaper.aUpperShape * abovePoint1;

    float sample = fastMap(inputAbs * sign, lowerBoundX, upperBoundX, 0.0f, 1.0f);
    sample = calcSquircleSimplified(sample, shape);
    sample = fastMap(sample, 0.0f, 1.0f, lowerBoundY, upperBoundY) * sign;

    return (sample * layer.waveshaper.aDryWet) + (input * (1.0f - layer.waveshaper.aDryWet));
}
