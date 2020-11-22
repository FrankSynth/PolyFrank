#pragma once

#include <stdint.h>

typedef struct {
    const uint32_t size;
    const float *data;
} wavetable;

extern const wavetable wavetable_strings01;