#include "renderAudio.hpp"
#include "debughelper/debughelper.hpp"
#include "layer/layer.hpp"
#include "math/polyMath.hpp"
#include "wavetables/wavetables.hpp"

#define SAMPLERATE 96000
#define FULLSCALE 0xFFFFFFFF

extern Layer layerA;

static inline int32_t convert(float f) {
    double t = (double)f * 8388607.0;

    return (int32_t)(t);
}

void renderAudio(int32_t *renderDest, uint32_t samples, uint32_t channels) {

    static const wavetable *currentWavetable = &wavetable_strings01;

    static float step[8] = {0};
    for (uint32_t i = 0; i < samples * channels; i += channels) {

        for (uint32_t chan = 0; chan < channels; chan++) {
            renderDest[i + chan] = convert(currentWavetable->data[(uint32_t)step[chan]]); // step * freq / 96khz
            step[chan] += fastNoteLin2Log_f32((float)chan);
            if (step[chan] >= (float)currentWavetable->size)
                step[chan] -= (float)currentWavetable->size;
        }
    }
}
