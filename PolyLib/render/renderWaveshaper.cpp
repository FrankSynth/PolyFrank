
#include "renderWaveshaper.hpp"
#include "math/polyMath.hpp"

#ifdef POLYRENDER

inline vec<VOICESPERCHIP> accumulateValue(const Input &input, const Analog &knob) {
    return clamp(input + knob, knob.min, knob.max);
}
inline vec<VOICESPERCHIP> accumulateX1(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint1X + waveshaper.aPoint1X, WAVESHAPERDISTANCE,
                 waveshaper.aPoint2X - WAVESHAPERDISTANCE);
}
inline vec<VOICESPERCHIP> accumulateX2(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint2X + waveshaper.aPoint2X, waveshaper.Point1X.nextSample + WAVESHAPERDISTANCE,
                 waveshaper.aPoint3X - WAVESHAPERDISTANCE);
}
inline vec<VOICESPERCHIP> accumulateX3(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint3X + waveshaper.aPoint3X, waveshaper.Point2X.nextSample + WAVESHAPERDISTANCE,
                 1.0f - WAVESHAPERDISTANCE);
}

void renderWaveshaper(Waveshaper &waveshaper, uint8_t voice) {
    waveshaper.Point1X = accumulateX1(waveshaper);
    waveshaper.Point1Y = accumulateValue(waveshaper.iPoint1Y, waveshaper.aPoint1Y);
    waveshaper.Point2X = accumulateX2(waveshaper);
    waveshaper.Point2Y = accumulateValue(waveshaper.iPoint2Y, waveshaper.aPoint2Y);
    waveshaper.Point3X = accumulateX3(waveshaper);
    waveshaper.Point3Y = accumulateValue(waveshaper.iPoint3Y, waveshaper.aPoint3Y);
    waveshaper.Point4Y = accumulateValue(waveshaper.iPoint4Y, waveshaper.aPoint4Y);

    waveshaper.DryWet = accumulateValue(waveshaper.iDryWet, waveshaper.aDryWet);

    waveshaper.splineX[voice].clear();
    waveshaper.splineY[voice].clear();

    waveshaper.splineX[voice].push_back(0.0f);
    waveshaper.splineX[voice].push_back(waveshaper.Point1X.nextSample[voice]);
    waveshaper.splineX[voice].push_back(waveshaper.Point2X.nextSample[voice]);
    waveshaper.splineX[voice].push_back(waveshaper.Point3X.nextSample[voice]);
    waveshaper.splineX[voice].push_back(1.0f);

    waveshaper.splineY[voice].push_back(0.0f);
    waveshaper.splineY[voice].push_back(waveshaper.Point1Y.nextSample[voice]);
    waveshaper.splineY[voice].push_back(waveshaper.Point2Y.nextSample[voice]);
    waveshaper.splineY[voice].push_back(waveshaper.Point3Y.nextSample[voice]);
    waveshaper.splineY[voice].push_back(waveshaper.Point4Y.nextSample[voice]);
    waveshaper.wavespline[voice].set_points(waveshaper.splineX[voice], waveshaper.splineY[voice]);
}

#endif

float renderWaveshaperSample(float input, const Waveshaper &waveshaper) {
    float sign = getSign(input);
    float inputAbs = input * sign;

    float sample = waveshaper.wavespline[0](inputAbs);
    sample *= sign;

    return (sample * waveshaper.aDryWet) + (input * (1.0f - waveshaper.aDryWet));
}
