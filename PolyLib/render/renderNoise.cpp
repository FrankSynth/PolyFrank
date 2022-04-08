#ifdef POLYRENDER

#include "renderNoise.hpp"
#include "rng.h"

inline float accumulateSamplecrusher(Noise &noise, uint16_t voice) {
    return std::clamp(noise.iSamplecrusher.currentSample[voice] + noise.aSamplecrusher.valueMapped,
                      noise.aSamplecrusher.min, noise.aSamplecrusher.max);
}

void renderNoise(Noise &noise) {

    float *outSamplecrusher = noise.samplecrusher.nextSample;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(noise, voice);
}

#endif