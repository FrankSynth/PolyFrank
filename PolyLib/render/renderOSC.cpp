#ifdef POLYRENDER

#include "renderOSC.hpp"
#include "math/polyMath.hpp"
#include "renderCVDef.h"

extern Layer layerA;

inline float accumulateBitcrusher(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iBitcrusher.currentSample[voice] + osc_a.aBitcrusher.valueMapped, osc_a.aBitcrusher.min,
                     osc_a.aBitcrusher.max);
}
inline float accumulateSamplecrusher(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iSamplecrusher.currentSample[voice] + osc_a.aSamplecrusher.valueMapped,
                     osc_a.aSamplecrusher.min, osc_a.aSamplecrusher.max);
}
inline float accumulateMorph(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iMorph.currentSample[voice] + osc_a.aMorph.valueMapped, osc_a.aMorph.min, osc_a.aMorph.max);
}
inline int32_t accumulateOctave(OSC_A &osc_a, uint16_t voice) {
    return testInt(std::roundf(osc_a.iOctave.currentSample[voice] + (float)osc_a.dOctave.valueMapped),
                   osc_a.dOctave.min, osc_a.dOctave.max);
}

float noteConverted[VOICESPERCHIP];

inline void accumulateNote(OSC_A &osc_a, float *note) {
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        noteConverted[i] = (float)(layerA.midi.rawNote[i]) / 12.0f;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        noteConverted[i] += layerA.oscA.aMasterTune.valueMapped;

    static float currentNote[VOICESPERCHIP] = {0};
    static float desiredNote[VOICESPERCHIP] = {0};

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] = noteConverted[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[0][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += accumulateOctave(osc_a, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.feel.detune.currentSample[i];

    // TODO check glide, plus glide settings, i.e. CT & CR
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] =
                std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide.currentSample[i], desiredNote[i]);
        else
            currentNote[i] =
                std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide.currentSample[i], desiredNote[i]);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = currentNote[i] + osc_a.iFM.currentSample[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] += layerA.midi.oPitchbend.currentSample[i] * layerA.layersettings.dPitchbendRange.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = fastNoteLin2Log_f32(note[i]);
}

void renderOSC_A(OSC_A &osc_A) {
    float *outNote = osc_A.note.nextSample;
    float *outMorph = osc_A.morph.nextSample;
    float *outBitcrusher = osc_A.bitcrusher.nextSample;
    float *outSamplecrusher = osc_A.samplecrusher.nextSample;

    accumulateNote(osc_A, outNote);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outMorph[voice] = accumulateMorph(osc_A, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(osc_A, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(osc_A, voice);
}

///////////////////////////// OSC B /////////////////////////////////

inline float accumulateBitcrusher(OSC_B &osc_b, uint16_t voice) {
    return testFloat(osc_b.iBitcrusher.currentSample[voice] + osc_b.aBitcrusher.valueMapped, osc_b.aBitcrusher.min,
                     osc_b.aBitcrusher.max);
}
inline float accumulateSamplecrusher(OSC_B &osc_b, uint16_t voice) {
    return testFloat(osc_b.iSamplecrusher.currentSample[voice] * osc_b.aSamplecrusher.valueMapped +
                         osc_b.aSamplecrusher.valueMapped,
                     osc_b.aSamplecrusher.min, osc_b.aSamplecrusher.max);
}
inline float accumulateMorph(OSC_B &osc_b, uint16_t voice) {
    return testFloat(osc_b.iMorph.currentSample[voice] + osc_b.aMorph.valueMapped, osc_b.aMorph.min, osc_b.aMorph.max);
}

inline int32_t accumulateOctave(OSC_B &osc_b, uint16_t voice) {
    return testInt(std::roundf(osc_b.iOctave.currentSample[voice] + (float)osc_b.dOctave.valueMapped),
                   osc_b.dOctave.min, osc_b.dOctave.max);
}
inline float accumulatePhaseoffset(OSC_B &osc_b, uint16_t voice) {
    return osc_b.iPhaseOffset.currentSample[voice] + osc_b.aPhaseoffset.valueMapped;
}

inline void accumulateNote(OSC_B &osc_b, float *note) {

    static float currentNote[VOICESPERCHIP] = {0};
    static float desiredNote[VOICESPERCHIP] = {0};

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] = noteConverted[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[1][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.oscB.aTuning.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.oscA.aMasterTune.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.feel.detune.currentSample[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += accumulateOctave(osc_b, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] =
                std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide.currentSample[i], desiredNote[i]);
        else
            currentNote[i] =
                std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide.currentSample[i], desiredNote[i]);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = currentNote[i] + osc_b.iFM.currentSample[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] += layerA.midi.oPitchbend.currentSample[i] * layerA.layersettings.dPitchbendRange.valueMapped;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = fastNoteLin2Log_f32(note[i]);
}

void renderOSC_B(OSC_B &osc_B) {

    float *outNote = osc_B.note.nextSample;
    float *outMorph = osc_B.morph.nextSample;
    float *outBitcrusher = osc_B.bitcrusher.nextSample;
    float *outSamplecrusher = osc_B.samplecrusher.nextSample;
    float *phaseoffset = osc_B.phaseoffset.nextSample;

    accumulateNote(osc_B, outNote);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outMorph[voice] = accumulateMorph(osc_B, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(osc_B, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(osc_B, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        phaseoffset[voice] = accumulatePhaseoffset(osc_B, voice);
}

#endif