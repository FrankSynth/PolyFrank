#include "wavetables.hpp"

const WaveTable *wavetables[] = {
    &wavetable_FeltPianoLow, &wavetable_GuitarHigh,  &wavetable_GuitarLow, &wavetable_Saw,      &wavetable_Sine,
    &wavetable_Square,       &wavetable_SSMMix01,    &wavetable_SSMMix02,  &wavetable_SSMMix03, &wavetable_SSMMix04,
    &wavetable_SSMMix05,     &wavetable_SSMMix06,    &wavetable_SSMMix07,  &wavetable_SSMMix08, &wavetable_SSMSaw,
    &wavetable_SSMSquare,    &wavetable_SSMTriangle, &wavetable_Triangle};

const std::vector<std::string> nlWavetable = {
    wavetable_FeltPianoLow, wavetable_GuitarHigh,  wavetable_GuitarLow, wavetable_Saw,      wavetable_Sine,
    wavetable_Square,       wavetable_SSMMix01,    wavetable_SSMMix02,  wavetable_SSMMix03, wavetable_SSMMix04,
    wavetable_SSMMix05,     wavetable_SSMMix06,    wavetable_SSMMix07,  wavetable_SSMMix08, wavetable_SSMSaw,
    wavetable_SSMSquare,    wavetable_SSMTriangle, wavetable_Triangle};
