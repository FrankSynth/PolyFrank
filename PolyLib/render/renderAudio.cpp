#include "renderAudio.hpp"
#include "math/polyMath.hpp"

void renderAudio(uint32_t *renderDest, uint32_t samples) {
    static uint32_t step = 0;
    for (uint32_t i = 0; i < samples; i++) {
        renderDest[i] = fast_sin_f32((float)step * 0.0045833333f); // freq / 96khz - 440 / 96000
        step++;
    }
}