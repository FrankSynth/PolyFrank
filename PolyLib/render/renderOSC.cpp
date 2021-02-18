#ifdef POLYRENDER

#include "renderOSC.hpp"
#include "math/polyMath.hpp"

extern Layer layerA;

inline float accumulateLevel(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iLevel.currentSample[voice] + osc_a.aLevel.valueMapped, osc_a.aLevel.min, osc_a.aLevel.max);
}
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

static float noteConverted[VOICESPERCHIP];

inline void accumulateNote(OSC_A &osc_a, float *note) {
    // TODO glide missing
    // TODO detune missing
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        noteConverted[i] = (float)(layerA.midi.rawNote[i] - 21) / (float)12.0f;

    // TODO settings pitchbend range missing
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        noteConverted[i] += layerA.midi.oPitchbend.currentSample[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = noteConverted[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] += accumulateOctave(osc_a, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] += osc_a.iFM.currentSample[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = fastNoteLin2Log_f32(note[i]);
}

void renderOSC_A(OSC_A &osc_A) {
    float *outNote = osc_A.note.nextSample;
    float *outMorph = osc_A.morph.nextSample;
    float *outBitcrusher = osc_A.bitcrusher.nextSample;
    float *outSamplecrusher = osc_A.samplecrusher.nextSample;
    float *outLevelSteiner = osc_A.levelSteiner.nextSample;
    float *outLevelLadder = osc_A.levelLadder.nextSample;
    static int32_t &filterSwitch = osc_A.dVcfDestSwitch.valueMapped;
    float level[VOICESPERCHIP];

    accumulateNote(osc_A, outNote);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outMorph[voice] = accumulateMorph(osc_A, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(osc_A, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(osc_A, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        level[voice] = accumulateLevel(osc_A, voice);

    switch (filterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = level[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = level[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = level[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = level[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = 0;
            break;
    }
}

///////////////////////////// OSC B /////////////////////////////////

inline float accumulateLevel(OSC_B &osc_b, uint16_t voice) {
    return testFloat(osc_b.iLevel.currentSample[voice] + osc_b.aLevel.valueMapped, osc_b.aLevel.min, osc_b.aLevel.max);
}
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

inline void accumulateNote(OSC_B &osc_b, float *note) {

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = noteConverted[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] += accumulateOctave(osc_b, i);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] += osc_b.iFM.currentSample[i];

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = fastNoteLin2Log_f32(note[i]);
}

void renderOSC_B(OSC_B &osc_B) {

    float *outNote = osc_B.note.nextSample;
    float *outMorph = osc_B.morph.nextSample;
    float *outBitcrusher = osc_B.bitcrusher.nextSample;
    float *outSamplecrusher = osc_B.samplecrusher.nextSample;
    float *outLevelSteiner = osc_B.levelSteiner.nextSample;
    float *outLevelLadder = osc_B.levelLadder.nextSample;
    static int32_t &filterSwitch = osc_B.dVcfDestSwitch.valueMapped;
    float level[VOICESPERCHIP];

    accumulateNote(osc_B, outNote);

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outMorph[voice] = accumulateMorph(osc_B, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outBitcrusher[voice] = accumulateBitcrusher(osc_B, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        outSamplecrusher[voice] = accumulateSamplecrusher(osc_B, voice);
    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
        level[voice] = accumulateLevel(osc_B, voice);

    switch (filterSwitch) {
        case 0:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = level[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = 0;
            break;
        case 1:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = level[voice];
            break;
        case 2:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = level[voice];
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = level[voice];
            break;
        case 3:
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelSteiner[voice] = 0;
            for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++)
                outLevelLadder[voice] = 0;
            break;
    }
}

#endif