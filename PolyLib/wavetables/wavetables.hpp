#pragma once

#include <main.h>
#include <stdint.h>
#include <string>
#include <vector>

// 48k wavetable length
#define MINWAVETABLELENGTH 1746
#define MAXWAVETABLELENGTH 13968
#define WAVETABLESPERVOICE 4
#define MAXWAVETABLESPERVOICE 4
#define WAVETABLESAMOUNT 17

/**
 * @brief wave table size, cycles, sizePerCycle, name and data pointer
 *
 */

class WaveTable {
    /**
     * @brief wave table size must be smaller than 13968
     *
     */
  public:
    uint32_t size;
    uint32_t cycles;
    float sizePerCycle;
    const float *data;
    const char *name;

    operator std::string() const { return name; }
};

extern const WaveTable wavetable_FeltPianoLow;
extern const WaveTable wavetable_GuitarHigh;
extern const WaveTable wavetable_GuitarLow;
extern const WaveTable wavetable_Saw;
extern const WaveTable wavetable_Sine;
extern const WaveTable wavetable_Square;
extern const WaveTable wavetable_SSMMix01;
extern const WaveTable wavetable_SSMMix02;
extern const WaveTable wavetable_SSMMix03;
extern const WaveTable wavetable_SSMMix04;
extern const WaveTable wavetable_SSMMix05;
extern const WaveTable wavetable_SSMMix06;
extern const WaveTable wavetable_SSMMix07;
extern const WaveTable wavetable_SSMMix08;
extern const WaveTable wavetable_SSMSaw;
extern const WaveTable wavetable_SSMSquare;
extern const WaveTable wavetable_SSMTriangle;
extern const WaveTable wavetable_Triangle;

extern const WaveTable *wavetables[];
extern const std::vector<std::string> nlWavetable;