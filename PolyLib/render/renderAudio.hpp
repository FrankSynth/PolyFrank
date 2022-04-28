#pragma once
#include "datacore/datalocation.hpp"
#include "debughelper/debughelper.hpp"
#include "layer/layer.hpp"
#include "math/polyMath.hpp"
#include "wavetables/wavetables.hpp"
#include <stdint.h>

void loadInitialWavetables();
void renderAudio(volatile int32_t *renderDest);
void renderAudioUI(int8_t *renderDest);
void switchOscAWavetable(uint32_t position, const WaveTable *wavetable);
void switchOscBWavetable(uint32_t position, const WaveTable *wavetable);