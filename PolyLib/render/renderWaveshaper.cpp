
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

void renderWaveshaper(Waveshaper &waveshaper) {
    // waveshaper.shape1 = accumulateValue(waveshaper.iShape1, waveshaper.aShape1);.nextSample
    // waveshaper.shape2 = accumulateValue(waveshaper.iShape2, waveshaper.aShape2);
    // waveshaper.shape3 = accumulateValue(waveshaper.iShape3, waveshaper.aShape3);
    // waveshaper.shape4 = accumulateValue(waveshaper.iShape4, waveshaper.aShape4);
    waveshaper.Point1X = accumulateX1(waveshaper);
    waveshaper.Point1Y = accumulateValue(waveshaper.iPoint1Y, waveshaper.aPoint1Y);
    waveshaper.Point2X = accumulateX2(waveshaper);
    waveshaper.Point2Y = accumulateValue(waveshaper.iPoint2Y, waveshaper.aPoint2Y);
    waveshaper.Point3X = accumulateX3(waveshaper);
    waveshaper.Point3Y = accumulateValue(waveshaper.iPoint3Y, waveshaper.aPoint3Y);
    waveshaper.Point4Y = accumulateValue(waveshaper.iPoint4Y, waveshaper.aPoint4Y);

    waveshaper.DryWet = accumulateValue(waveshaper.iDryWet, waveshaper.aDryWet);

    static uint32_t counter = 0;

    waveshaper.splineX[counter].clear();
    waveshaper.splineY[counter].clear();

    waveshaper.splineX[counter].push_back(0.0f);
    waveshaper.splineX[counter].push_back(waveshaper.Point1X.nextSample[counter]);
    waveshaper.splineX[counter].push_back(waveshaper.Point2X.nextSample[counter]);
    waveshaper.splineX[counter].push_back(waveshaper.Point3X.nextSample[counter]);
    waveshaper.splineX[counter].push_back(1.0f);

    waveshaper.splineY[counter].push_back(0.0f);
    waveshaper.splineY[counter].push_back(waveshaper.Point1Y.nextSample[counter]);
    waveshaper.splineY[counter].push_back(waveshaper.Point2Y.nextSample[counter]);
    waveshaper.splineY[counter].push_back(waveshaper.Point3Y.nextSample[counter]);
    waveshaper.splineY[counter].push_back(waveshaper.Point4Y.nextSample[counter]);
    waveshaper.wavespline[counter].set_points(waveshaper.splineX[counter], waveshaper.splineY[counter]);

    counter++;
    if (counter >= VOICESPERCHIP)
        counter = 0;
}

#endif

float renderWaveshaperSample(float input, const Waveshaper &waveshaper) {
    float sign = getSign(input);
    float inputAbs = input * sign;

    float sample = waveshaper.wavespline[0](inputAbs);
    sample *= sign;

    return (sample * waveshaper.aDryWet) + (input * (1.0f - waveshaper.aDryWet));
}
