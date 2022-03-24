#ifdef POLYRENDER

#include "renderOutput.hpp"

extern Layer layerA;

LogCurve panningAntiLog(32, 0.94);

// TODO check this pan spread mapping
inline float accumulatePan(Out &out, uint16_t voice) {
    float pan = out.iPan.currentSample[voice] + out.aPan.valueMapped;
    if (layerA.spreadValues[voice] < 0)
        pan = fast_lerp_f32(pan, -1, out.aPanSpread.valueMapped * -layerA.spreadValues[voice]);
    else
        pan = fast_lerp_f32(pan, 1, out.aPanSpread.valueMapped * layerA.spreadValues[voice]);

    return pan;
}

inline float accumulateVCA(Out &out, uint16_t voice) {
    return testFloat(out.iVCA.currentSample[voice] + out.aVCA.valueMapped +
                         out.aADSR.valueMapped * layerA.envA.out.currentSample[voice],
                     out.aVCA.min, out.aVCA.max);
}

inline float accumulateDistort(Out &out, uint16_t voice) {
    return testFloat(out.iDistort.currentSample[voice] + out.aDistort.valueMapped, 0, 1);
}

void renderOut(Out &out) {

    float *outDistort = out.distort.nextSample;
    float *outLeft = out.left.nextSample;
    float *outRight = out.right.nextSample;
    float vca[VOICESPERCHIP];
    float pan[VOICESPERCHIP];
    float panRight[VOICESPERCHIP];
    float panLeft[VOICESPERCHIP];
    float left[VOICESPERCHIP];
    float right[VOICESPERCHIP];

    // const float &spread = out.aPanSpread.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outDistort[voice] = accumulateDistort(out, voice);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        vca[i] = accumulateVCA(out, i);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        pan[i] = accumulatePan(out, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panRight[i] = (pan[i] + 1) * 0.5f;
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panLeft[i] = (-1 * pan[i] + 1) * 0.5f;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panLeft[i] = panLeft[i];
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panRight[i] = panRight[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        left[i] = panningAntiLog.mapValue(vca[i] * panLeft[i] * out.aMaster.valueMapped);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        right[i] = panningAntiLog.mapValue(vca[i] * panRight[i] * out.aMaster.valueMapped);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        outLeft[i] = testFloat(left[i], 0, 1);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        outRight[i] = testFloat(right[i], 0, 1);
}

#endif