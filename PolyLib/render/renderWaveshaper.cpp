
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
    return clamp(waveshaper.iPoint2X + waveshaper.aPoint2X, waveshaper.Point1X + WAVESHAPERDISTANCE,
                 waveshaper.aPoint3X - WAVESHAPERDISTANCE);
}
inline vec<VOICESPERCHIP> accumulateX3(const Waveshaper &waveshaper) {
    return clamp(waveshaper.iPoint3X + waveshaper.aPoint3X, waveshaper.Point2X + WAVESHAPERDISTANCE,
                 1.0f - WAVESHAPERDISTANCE);
}

void renderWaveshaper(Waveshaper &waveshaper, RenderBuffer &effectAmt, uint8_t voice) {

    waveshaper.Point1X = accumulateX1(waveshaper);
    waveshaper.Point1Y = accumulateValue(waveshaper.iPoint1Y, waveshaper.aPoint1Y);
    waveshaper.Point2X = accumulateX2(waveshaper);
    waveshaper.Point2Y = accumulateValue(waveshaper.iPoint2Y, waveshaper.aPoint2Y);
    waveshaper.Point3X = accumulateX3(waveshaper);
    waveshaper.Point3Y = accumulateValue(waveshaper.iPoint3Y, waveshaper.aPoint3Y);
    waveshaper.Point4Y = accumulateValue(waveshaper.iPoint4Y, waveshaper.aPoint4Y);

    waveshaper.DryWet =
        clamp((waveshaper.iDryWet + waveshaper.aDryWet) * effectAmt, waveshaper.aDryWet.min, waveshaper.aDryWet.max);

    // waveshaper.splineX[counter][0] = 0.0f;
    waveshaper.splineX[voice][1] = waveshaper.Point1X[voice];
    waveshaper.splineX[voice][2] = waveshaper.Point2X[voice];
    waveshaper.splineX[voice][3] = waveshaper.Point3X[voice];
    // waveshaper.splineX[voice][4] = 1.0f;

    // waveshaper.splineY[voice][0] = 0.0f;
    waveshaper.splineY[voice][1] = waveshaper.Point1Y[voice];
    waveshaper.splineY[voice][2] = waveshaper.Point2Y[voice];
    waveshaper.splineY[voice][3] = waveshaper.Point3Y[voice];
    waveshaper.splineY[voice][4] = waveshaper.Point4Y[voice];

    waveshaper.wavespline[voice].set_points_polyfrank(waveshaper.splineX[voice], waveshaper.splineY[voice]);
}

#endif

float renderWaveshaperSample(float input, const Waveshaper &waveshaper) {
    float inputAbs = fabs(input);

    float sample = waveshaper.wavespline[0].getValueWithoutExtrapolation(inputAbs);
    sample *= getSign(input);

    return (sample * waveshaper.aDryWet) + (input * (1.0f - waveshaper.aDryWet));
}
