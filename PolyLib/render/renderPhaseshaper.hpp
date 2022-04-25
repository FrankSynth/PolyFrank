#pragma once

#include "layer/layer.hpp"

#ifdef POLYRENDER

void renderPhaseshaper(Phaseshaper &phaseshaper);

/**
 * @brief calculates vec<VOICESPERCHIP> at once
 *
 * @param input phase between 0 and 1
 * @return vec<VOICESPERCHIP>
 */
vec<VOICESPERCHIP> renderPhaseshaperSample(const vec<VOICESPERCHIP> &input);

#endif

/**
 * @brief returns single sample
 *
 * @param input phase between 0 and 1
 * @param layer current layer
 * @return float
 */
float renderPhaseshaperSample(float input, const Layer &layer);