#include "wavetables.hpp"
std::vector<const WaveTable *> wavetables;

uint32_t WaveTable::subSize[SUBWAVETABLES] = {2048, 2048, 2048, 2048, 1448, 936, 608, 392,
                                              256,  160,  128,  128,  128,  128, 128, 128};

uint32_t WaveTable::size = MAXWAVETABLELENGTH;

std::vector<const char *> nlWavetable;

inline void pushWavetable(const WaveTable &wavetable) {
#ifdef POLYRENDER
    wavetables.push_back(&wavetable);
#endif
    nlWavetable.push_back(wavetable.name);
}

void initWavetables() {

    nlWavetable.reserve(WAVETABLESAMOUNT);
#ifdef POLYRENDER
    wavetables.reserve(WAVETABLESAMOUNT);
#endif

    pushWavetable(wavetable_FeltPianoLow);
    pushWavetable(wavetable_GuitarHigh);
    pushWavetable(wavetable_GuitarLow);
    pushWavetable(wavetable_Saw);
    pushWavetable(wavetable_Sine);
    pushWavetable(wavetable_Square);
    pushWavetable(wavetable_SSMMix01);
    pushWavetable(wavetable_SSMMix02);
    pushWavetable(wavetable_SSMMix03);
    pushWavetable(wavetable_SSMMix04);
    pushWavetable(wavetable_SSMMix05);
    pushWavetable(wavetable_SSMMix06);
    pushWavetable(wavetable_SSMMix07);
    pushWavetable(wavetable_SSMMix08);
    pushWavetable(wavetable_SSMSaw);
    pushWavetable(wavetable_SSMSquare);
    pushWavetable(wavetable_SSMTriangle);
    pushWavetable(wavetable_Triangle);
}