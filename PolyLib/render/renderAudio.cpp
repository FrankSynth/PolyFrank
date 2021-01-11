#ifdef POLYRENDER

#include "renderAudio.hpp"

#define SAMPLERATE 96000
#define FULLSCALE 0xFFFFFFFF

extern Layer layerA;

RAM1 float wavetableA[MAXWAVETABLELENGTH];
uint32_t wabetableASize;
RAM1 float wavetableB[MAXWAVETABLELENGTH];
uint32_t wabetableBSize;

static inline int32_t convert(float f) {
    double t = (double)f * 8388607.0;

    return (int32_t)(t);
}

// TODO init Audio Rendering
void initAudioRendering() {
    wabetableASize = wavetable_strings01.size;
    fast_copy_f32((uint32_t *)wavetable_strings01.data, (uint32_t *)wavetableA, wabetableASize);
    wabetableBSize = wavetable_nylonGuitar01.size;
    fast_copy_f32((uint32_t *)wavetable_nylonGuitar01.data, (uint32_t *)wavetableB, wabetableBSize);
}

// TODO Audio rendering
void renderAudio(int32_t *renderDest, uint32_t samples, uint32_t channels) {

    // static const wavetable *currentWavetable1 = &wavetable_strings01;
    static float stepWavetable1[8] = {0};
    // static const wavetable *currentWavetable2 = &wavetable_nylonGuitar01;
    static float stepWavetable2[8] = {0};

    float balance = fastMapCached(layerA.adsrA.aAttack.valueMapped, 0, 10, 0, 1);

    for (uint32_t i = 0; i < samples * channels; i += channels) {

        for (uint32_t chan = 0; chan < channels; chan++) {

            while (stepWavetable1[chan] >= (float)wabetableASize)
                stepWavetable1[chan] -= (float)wabetableASize;
            while (stepWavetable2[chan] >= (float)wabetableBSize)
                stepWavetable2[chan] -= (float)wabetableBSize;

            float audioSample = wavetableA[(uint32_t)stepWavetable1[chan]] * balance +
                                wavetableB[(uint32_t)stepWavetable2[chan]] * (1 - balance);

            renderDest[i + chan] = convert(audioSample);

            float step = fastNoteLin2Log_f32((float)chan * layerA.test.aFreq.valueMapped);
            stepWavetable1[chan] += step;
            stepWavetable2[chan] += step;
        }
    }
}

#endif