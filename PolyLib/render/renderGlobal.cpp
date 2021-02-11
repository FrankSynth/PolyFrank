#ifdef POLYRENDER

#include "renderGlobal.hpp"

inline float accumulatePan(GlobalModule globalModule, uint16_t voice) {
    return testFloat(globalModule.iPan.currentSample[voice] + globalModule.aPan.valueMapped, globalModule.aPan.min,
                     globalModule.aPan.max);
}
inline float accumulateVCA(GlobalModule globalModule, uint16_t voice) {
    return testFloat(globalModule.iVCA.currentSample[voice] + globalModule.aVCA.valueMapped, globalModule.aVCA.min,
                     globalModule.aVCA.max);
}

void renderGlobalModule(GlobalModule globalModule) {

    float *outLeft = globalModule.left.nextSample;
    float *outRight = globalModule.right.nextSample;
    // TODO spread

    const float &spread = globalModule.aSpread.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        float vca = accumulateVCA(globalModule, voice);
        float pan = accumulatePan(globalModule, voice);

        float panRight = (pan + 1) * 0.5f;
        float panLeft = (-1 * pan + 1) * 0.5f;

        float left = vca * panLeft;
        float right = vca * panRight;

        outLeft[voice] = testFloat(left, 0, 1);
        outRight[voice] = testFloat(right, 0, 1);
    }
}

#endif