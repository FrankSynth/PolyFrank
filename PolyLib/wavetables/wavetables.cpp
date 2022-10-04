#include "wavetables.hpp"
std::vector<const WaveTable *> wavetables;

uint32_t WaveTable::subSize[SUBWAVETABLES] = {2048, 2048, 2048, 2048, 1448, 936, 608, 392,
                                              256,  160,  128,  128,  128,  128, 128, 128};

uint32_t WaveTable::size = MAXWAVETABLELENGTH;

std::vector<const char *> nlWavetable;

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
}
