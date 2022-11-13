#ifdef POLYRENDER

#include "renderOutput.hpp"

extern Layer layerA;

LogCurve panningAntiLog(64, 0.970); //-3db bei 0.5
LogCurve volumeAntiLog(64, 0.970);  //-3db bei 0.5

// TODO check this pan spread mapping
inline vec<VOICESPERCHIP> accumulatePan(const Out &out) {
    vec<VOICESPERCHIP> pan = out.iPan + out.aPan;
    vec<VOICESPERCHIP> sign = getSign(layerA.spreadValues);
    pan = fast_lerp_f32(pan, sign, sign * out.aPanSpread * layerA.spreadValues);
    return pan;
}

inline vec<VOICESPERCHIP> accumulateVCA(const Out &out) {
    return clamp(out.iVCA + out.aVCA, out.aVCA.min, out.aVCA.max);
}

inline vec<VOICESPERCHIP> accumulateDistort(const Out &out) {
    return clamp(out.iDistort + out.aDistort, 0.0f, 1.0f);
}

void renderOut(Out &out) {

    vec<VOICESPERCHIP> pan;
    vec<VOICESPERCHIP> panRight;
    vec<VOICESPERCHIP> panLeft;
    vec<VOICESPERCHIP> left;
    vec<VOICESPERCHIP> right;
    vec<VOICESPERCHIP> volume;

    out.distort = accumulateDistort(out);
    out.vca = accumulateVCA(out);
    out.pan = accumulatePan(out);

    panRight = (out.pan + 1.0f) * 0.5f;
    panLeft = (out.pan * -1.0f + 1.0f) * 0.5f;

    volume = 1 - volumeAntiLog.mapValue(out.vca * out.aMaster);

    left = panningAntiLog.mapValue(panLeft) - volume;
    right = panningAntiLog.mapValue(panRight) - volume;

    out.left = clamp(left, 0.0f, 1.0f);
    out.right = clamp(right, 0.0f, 1.0f);
}

#endif