#ifdef POLYRENDER

#include "renderGlobal.hpp"

LogCurve panningAntiLog(16, 0.9);

inline float accumulatePan(GlobalModule globalModule, uint16_t voice) {
    return testFloat(globalModule.iPan.currentSample[voice] + globalModule.aPan.valueMapped, globalModule.aPan.min,
                     globalModule.aPan.max);
}
inline float accumulateVCA(GlobalModule globalModule, uint16_t voice) {
    return testFloat(globalModule.iVCA.currentSample[voice] + globalModule.aVCA.valueMapped, globalModule.aVCA.min,
                     globalModule.aVCA.max);
}

void renderGlobalModule(GlobalModule globalModule) {

    static float *outLeft = globalModule.left.nextSample;
    static float *outRight = globalModule.right.nextSample;
    float vca[VOICESPERCHIP];
    float pan[VOICESPERCHIP];
    float panRight[VOICESPERCHIP];
    float panLeft[VOICESPERCHIP];
    float left[VOICESPERCHIP];
    float right[VOICESPERCHIP];

    // TODO spread
    static const float &spread = globalModule.aSpread.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        vca[i] = accumulateVCA(globalModule, i);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        pan[i] = accumulatePan(globalModule, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panRight[i] = (pan[i] + 1) * 0.5f;
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panLeft[i] = (-1 * pan[i] + 1) * 0.5f;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panLeft[i] = panningAntiLog.mapValue(panLeft[i]);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        panRight[i] = panningAntiLog.mapValue(panRight[i]);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        left[i] = vca[i] * panLeft[i];
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        right[i] = vca[i] * panRight[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        outLeft[i] = testFloat(left[i], 0, 1);
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        outRight[i] = testFloat(right[i], 0, 1);
}

#endif