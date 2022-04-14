#ifdef POLYRENDER

#include "renderNoise.hpp"
#include "rng.h"

inline vec<VOICESPERCHIP> accumulateSamplecrusher(const Noise &noise) {
    return clamp(noise.iSamplecrusher + noise.aSamplecrusher, noise.aSamplecrusher.min, noise.aSamplecrusher.max);
}

void renderNoise(Noise &noise) {
    noise.samplecrusher = accumulateSamplecrusher(noise);
}

#endif