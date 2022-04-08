#ifdef POLYRENDER

#include "renderOutput.hpp"

extern Layer layerA;

LogCurve panningAntiLog(32, 0.94);

// TODO check this pan spread mapping
inline vec<VOICESPERCHIP> accumulatePan(Out &out) {
    vec<VOICESPERCHIP> pan = out.iPan + out.aPan;

    vec<VOICESPERCHIP> sign = getSign(layerA.spreadValues);

    pan = fast_lerp_f32(pan, sign, sign * out.aPanSpread.valueMapped * layerA.spreadValues);

    return pan;
}

inline vec<VOICESPERCHIP> accumulateVCA(Out &out) {
    return clamp(out.iVCA + out.aVCA + layerA.envA.out * out.aADSR, out.aVCA.min, out.aVCA.max);
}

inline vec<VOICESPERCHIP> accumulateDistort(Out &out) {
    return clamp(out.iDistort + out.aDistort, 0.0f, 1.0f);
}

void renderOut(Out &out) {

    vec<VOICESPERCHIP> vca;
    vec<VOICESPERCHIP> pan;
    vec<VOICESPERCHIP> panRight;
    vec<VOICESPERCHIP> panLeft;
    vec<VOICESPERCHIP> left;
    vec<VOICESPERCHIP> right;

    // const float &spread = out.aPanSpread.valueMapped;

    out.distort = accumulateDistort(out);
    vca = accumulateVCA(out);
    pan = accumulatePan(out);

    panRight = (pan + 1.0f) * 0.5f;
    panLeft = (-1.0f * pan + 1.0f) * 0.5f;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        left[i] = panningAntiLog.mapValue(vca[i] * panLeft[i] * out.aMaster.valueMapped);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        right[i] = panningAntiLog.mapValue(vca[i] * panRight[i] * out.aMaster.valueMapped);

    out.left = clamp(left, 0.0f, 1.0f);
    out.right = clamp(right, 0.0f, 1.0f);
}

#endif