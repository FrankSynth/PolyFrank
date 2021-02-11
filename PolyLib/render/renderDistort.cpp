#ifdef POLYRENDER

#include "renderDistort.hpp"

#define INPUTWEIGHTING 1

inline float accumulateDistort(Distortion &distort, uint16_t voice) {
    return testFloat(distort.iDistort.currentSample[voice] + distort.aDistort.valueMapped, distort.aDistort.min,
                     distort.aDistort.max);
}

void renderDistort(Distortion &distort) {
    float *outDistort = distort.distort.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
        outDistort[voice] = accumulateDistort(distort, voice);
    }
}

#endif