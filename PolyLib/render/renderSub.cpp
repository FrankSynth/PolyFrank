#ifdef POLYRENDER

#include "renderSub.hpp"

extern Layer layerA;

inline float accumulateShape(Sub &sub, uint16_t voice) {
    return std::clamp(sub.iShape.currentSample[voice] + sub.aShape.valueMapped, sub.aShape.min, sub.aShape.max);
}
inline float accumulateBitcrusher(Sub &sub, uint16_t voice) {
    return std::clamp(sub.iBitcrusher.currentSample[voice] + sub.aBitcrusher.valueMapped, sub.aBitcrusher.min,
                      sub.aBitcrusher.max);
}
inline float accumulateSamplecrusher(Sub &sub, uint16_t voice) {
    return std::clamp(sub.iSamplecrusher.currentSample[voice] + sub.aSamplecrusher.valueMapped, sub.aSamplecrusher.min,
                      sub.aSamplecrusher.max);
}

void renderSub(Sub &sub) {

    float *shapeOut = sub.shape.nextSample;
    float *outBitcrusher = sub.bitcrusher.nextSample;
    float *outSamplecrusher = sub.samplecrusher.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        shapeOut[voice] = accumulateShape(sub, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(sub, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(sub, voice);
}

#endif