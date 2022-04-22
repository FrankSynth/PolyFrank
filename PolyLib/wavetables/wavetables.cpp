#include "wavetables.hpp"
#ifdef POLYRENDER
std::vector<const WaveTable *> wavetables;
#endif
std::vector<std::string> nlWavetable;

inline void pushWavetable(const WaveTable &wavetable) {
#ifdef POLYRENDER
    wavetables.push_back(&wavetable);
#endif
    nlWavetable.push_back(wavetable.name);
}

void initWavetables() {
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