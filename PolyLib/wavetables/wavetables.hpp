#pragma once

#include <stdint.h>

#define MINWAVETABLELENGTH 1746
#define MAXWAVETABLELENGTH 13968
// #define MINWAVETABLELENGTH 3491
// #define MAXWAVETABLELENGTH 13964

// TODO new wavetables for new samplerate

/**
 * @brief wave table size, cycles and data pointer
 *
 */
typedef struct {
    /**
     * @brief wave table size should be 1746, 3492, 5238, 6984, 13968
     *
     */
    const uint32_t size;
    const uint32_t cycles;
    const float *data;
} wavetable;

extern const wavetable wavetable_strings01;
extern const wavetable wavetable_nylonGuitar01;
extern const wavetable wavetable_sinus01;
extern const wavetable wavetable_wurli02;