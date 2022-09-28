#pragma once

#include <main.h>
// #include <stdint.h>
#include "datacore/datalocation.hpp"
#include <string>
#include <vector>

#define MAXWAVETABLELENGTH 12760
#define WAVETABLESPERVOICE 4
#define MAXWAVETABLESPERVOICE 4
#define WAVETABLESAMOUNT 18
#define SUBWAVETABLES 16

class WaveTable;

// #ifdef POLYRENDER
extern std::vector<const WaveTable *> wavetables;
// #endif
extern std::vector<const char *> nlWavetable;

class WaveTable {
  public:
    static uint32_t size;
    static uint32_t subSize[SUBWAVETABLES];

    const float *subData[SUBWAVETABLES];
    const float *data;
    const char *name;

    WaveTable(const float *data, const char *name = "noName") {
        this->data = data;
        this->name = name;
        subData[0] = data;
        for (int i = 1; i < SUBWAVETABLES; i++) {
            subData[i] = &(subData[i - 1][subSize[i - 1]]);
        }
    }
    ~WaveTable() {}
};

void initWavetables();

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