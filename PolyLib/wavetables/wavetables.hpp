#pragma once

#include <main.h>
// #include <stdint.h>
#include "datacore/datalocation.hpp"
#include <string>
#include <vector>

#define MAXWAVETABLELENGTH 12760
#define WAVETABLESPERVOICE 4
#define MAXWAVETABLESPERVOICE 4
#define WAVETABLESAMOUNT 19
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

extern const WaveTable wavetable_Sine;
extern const WaveTable wavetable_Triangle;
extern const WaveTable wavetable_Saw;
extern const WaveTable wavetable_Square;
extern const WaveTable wavetable_am1;
extern const WaveTable wavetable_am2;
extern const WaveTable wavetable_am3;
extern const WaveTable wavetable_am4;
extern const WaveTable wavetable_fm1;
extern const WaveTable wavetable_fm2;
extern const WaveTable wavetable_fm3;
extern const WaveTable wavetable_fm4;
extern const WaveTable wavetable_pm1;
extern const WaveTable wavetable_pm2;
extern const WaveTable wavetable_pm3;
extern const WaveTable wavetable_pm4;
extern const WaveTable wavetable_FeltPianoLow;
extern const WaveTable wavetable_GuitarHigh;
extern const WaveTable wavetable_GuitarLow;