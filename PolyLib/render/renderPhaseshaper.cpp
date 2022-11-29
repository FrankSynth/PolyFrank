#include "renderPhaseshaper.hpp"
#include "math/polyMath.hpp"

#ifdef POLYRENDER
inline vec<VOICESPERCHIP> accumulateValue(const Input &input, const Analog &knob) {
    return clamp(input + knob, knob.min, knob.max);
}
inline vec<VOICESPERCHIP> accumulateX2(const Phaseshaper &phaseshaper) {
    return clamp(phaseshaper.iPoint2X * (phaseshaper.aPoint3X - phaseshaper.aPoint2X.min) + phaseshaper.aPoint2X,
                 phaseshaper.aPoint2X.min, phaseshaper.aPoint3X);
}
inline vec<VOICESPERCHIP> accumulateX3(const Phaseshaper &phaseshaper) {
    return clamp(phaseshaper.iPoint3X * (phaseshaper.aPoint3X.max - phaseshaper.aPoint2X) + phaseshaper.aPoint3X,
                 phaseshaper.aPoint2X, phaseshaper.aPoint3X.max);
}

void renderPhaseshaper(Phaseshaper &phaseshaper, const RenderBuffer &effectAmt) {

    phaseshaper.Point1Y = accumulateValue(phaseshaper.iPoint1Y, phaseshaper.aPoint1Y);
    phaseshaper.Point2X = accumulateX2(phaseshaper);
    phaseshaper.Point2Y = accumulateValue(phaseshaper.iPoint2Y, phaseshaper.aPoint2Y);
    phaseshaper.Point3X = accumulateX3(phaseshaper);
    phaseshaper.Point3Y = accumulateValue(phaseshaper.iPoint3Y, phaseshaper.aPoint3Y);
    phaseshaper.Point4Y = accumulateValue(phaseshaper.iPoint4Y, phaseshaper.aPoint4Y);

    phaseshaper.slope[0] = (phaseshaper.Point2Y - phaseshaper.Point1Y) / phaseshaper.Point2X;
    phaseshaper.slope[1] = (phaseshaper.Point3Y - phaseshaper.Point2Y) / (phaseshaper.Point3X - phaseshaper.Point2X);
    phaseshaper.slope[2] =
        (phaseshaper.Point4Y - phaseshaper.Point3Y) / ((vec<VOICESPERCHIP>)1.0f - phaseshaper.Point3X);

    phaseshaper.DryWet = clamp((phaseshaper.iDryWet + phaseshaper.aDryWet) * effectAmt, phaseshaper.aDryWet.min,
                               phaseshaper.aDryWet.max);
}

#endif
