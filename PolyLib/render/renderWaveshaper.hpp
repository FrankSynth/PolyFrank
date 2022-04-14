#pragma once

#include "layer/layer.hpp"
#include "math/polyMath.hpp"

#ifdef POLYRENDER

void renderWaveshaper(Waveshaper &waveshaper);

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return vec<VOICESPERCHIP>
 */
vec<VOICESPERCHIP> renderWaveshaperSample(const vec<VOICESPERCHIP> &input);

#endif

/**
 * @brief returns single sample
 *
 * @param input sample between -1 and 1, though it's symmetrical anyway
 * @param layer current layer
 * @return float
 */
float renderWaveshaperSample(float input, const Layer &layer);