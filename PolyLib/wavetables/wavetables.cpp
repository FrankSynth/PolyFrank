#include "wavetables.hpp"
std::vector<const WaveTable *> wavetables;

uint32_t WaveTable::subSize[SUBWAVETABLES] = {2048, 2048, 1720, 1116, 724, 468, 304, 196,
                                              128,  80,   64,   64,   64,  64,  64,  64};

uint32_t WaveTable::size = MAXWAVETABLELENGTH;

std::vector<const char *> nlWavetable;

const WaveTable *waveTableSets[][4] = {
    {&wavetable_Sine, &wavetable_Triangle, &wavetable_Saw, &wavetable_Square},
    {&wavetable_am1, &wavetable_am2, &wavetable_am3, &wavetable_am4},
    {&wavetable_fm1, &wavetable_fm2, &wavetable_fm3, &wavetable_fm4},
    {&wavetable_pm1, &wavetable_pm2, &wavetable_pm3, &wavetable_pm4},
    {&wavetable_FeltPianoLow, &wavetable_GuitarHigh, &wavetable_GuitarLow, &wavetable_Square},
    {&wavetable_Sine, &wavetable_Triangle, &wavetable_Saw, &wavetable_Square},
    {&wavetable_MiniSaw, &wavetable_MiniSquare, &wavetable_MiniTriangle, &wavetable_Saw},

};

inline void pushWavetable(const WaveTable &wavetable) {
    wavetables.push_back(&wavetable);
    nlWavetable.push_back(wavetable.name);
}

void initWavetables() {

    nlWavetable.reserve(WAVETABLESAMOUNT);
    // #ifdef POLYRENDER
    wavetables.reserve(WAVETABLESAMOUNT);
    // #endif

    pushWavetable(wavetable_Sine);
    pushWavetable(wavetable_Triangle);
    pushWavetable(wavetable_Saw);
    pushWavetable(wavetable_Square);
    pushWavetable(wavetable_am1);
    pushWavetable(wavetable_am2);
    pushWavetable(wavetable_am3);
    pushWavetable(wavetable_am4);
    pushWavetable(wavetable_fm1);
    pushWavetable(wavetable_fm2);
    pushWavetable(wavetable_fm3);
    pushWavetable(wavetable_fm4);
    pushWavetable(wavetable_pm1);
    pushWavetable(wavetable_pm2);
    pushWavetable(wavetable_pm3);
    pushWavetable(wavetable_pm4);
    pushWavetable(wavetable_FeltPianoLow);
    pushWavetable(wavetable_GuitarHigh);
    pushWavetable(wavetable_GuitarLow);
    pushWavetable(wavetable_MiniSaw);
    pushWavetable(wavetable_MiniSquare);
    pushWavetable(wavetable_MiniTriangle);
}
