#include "renderAudio.hpp"
#include "debughelper/debughelper.hpp"
#include "layer/layer.hpp"
#include "math/polyMath.hpp"

#define SAMPLERATE 96000
#define FULLSCALE 0xFFFFFFFF

extern Layer layerA;

static inline int32_t convert(float f) {
    double t = (double)f * 8388607.0;

    return (int32_t)(t);
}

void renderAudio(int32_t *renderDest, uint32_t samples, uint32_t channels) {
    static uint32_t step = 0;
    static volatile int32_t check0 = 0;
    static volatile int32_t check1 = 0;
    for (uint32_t i = 0; i < samples * channels; i += channels) {

        for (uint32_t chan = 0; chan < channels; chan++) {
            renderDest[i + chan] = convert(
                fast_sin_f32((float)step / (float)SAMPLERATE * layerA.adsrA.aDelay.valueMapped)); // step * freq / 96khz
        }
        step++;
    }

    // for (uint32_t i = 0; i < samples; i++) {
    //     renderDest[i * channels + 0] =
    //         convert(fast_sin_f32((float)step / (float)SAMPLERATE * 20.0f)); // freq / 96khz - 220 / 96000
    //     renderDest[i * channels + 1] = check0;

    //     check0 = renderDest[i * channels + 0];
    //     check1 = renderDest[i * channels + 1];

    //     renderDest[i * channels + 2] = check0;
    //     renderDest[i * channels + 3] = check0;
    //     renderDest[i * channels + 4] = check0;
    //     renderDest[i * channels + 5] = check0;
    //     renderDest[i * channels + 6] = check0;
    //     renderDest[i * channels + 7] = check0;
    //     // renderDest[i * channels + 0];
    //     // println(renderDest[i * channels + 0]);
    // }
}
