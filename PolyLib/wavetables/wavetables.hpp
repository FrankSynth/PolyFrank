#pragma once

#include <stdint.h>

#define MINWAVETABLELENGTH 3491
#define MAXWAVETABLELENGTH 13964

/**
 * @brief wave table size and data pointer
 *
 */
typedef struct {
    /**
     * @brief wave table size should be 3491, 6982, 10472, 13964
     *
     */
    const uint32_t size;
    const float *data;
} wavetable;

extern const wavetable wavetable_strings01;
extern const wavetable wavetable_nylonGuitar01;