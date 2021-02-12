#ifdef POLYRENDER

#include "renderOSC.hpp"
#include "math/polyMath.hpp"

extern Layer layerA;

#define INPUTWEIGHTING 1

inline float accumulateLevel(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iLevel.currentSample[voice] + osc_a.aLevel.valueMapped, osc_a.aLevel.min, osc_a.aLevel.max);
}
inline float accumulateBitcrusher(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iBitcrusher.currentSample[voice] + osc_a.aBitcrusher.valueMapped, osc_a.aBitcrusher.min,
                     osc_a.aBitcrusher.max);
}
inline float accumulateMorph(OSC_A &osc_a, uint16_t voice) {
    return testFloat(osc_a.iMorph.currentSample[voice] + osc_a.aMorph.valueMapped, osc_a.aMorph.min, osc_a.aMorph.max);
}
inline int32_t accumulateOctave(OSC_A &osc_a, uint16_t voice) {
    return testInt(std::roundf(osc_a.iOctave.currentSample[voice] + (float)osc_a.dOctave.valueMapped),
                   osc_a.dOctave.min, osc_a.dOctave.max);
}

inline float accumulateNote(OSC_A &osc_a, uint16_t voice) {
    // TODO glide missing
    // TODO detune missing
    float note;

    // A0 = 0, A1 = 1, ...
    note = (float)(layerA.midi.rawNote[voice] - 21) / (float)12.0f;

    // TODO settings pitchbend range missing
    note += layerA.midi.oPitchbend.currentSample[voice];

    note += accumulateOctave(osc_a, voice);

    note += osc_a.iFM.currentSample[voice];

    return fastNoteLin2Log_f32(note);
}

void renderOSC_A(OSC_A &osc_A) {
    float *outNote = osc_A.note.nextSample;
    float *outMorph = osc_A.morph.nextSample;
    float *outBitcrusher = osc_A.bitcrusher.nextSample;
    float *outLevelSteiner = osc_A.levelSteiner.nextSample;
    float *outLevelLadder = osc_A.levelLadder.nextSample;
    int32_t &filterSwitch = osc_A.dVcfDestSwitch.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        outNote[voice] = accumulateNote(osc_A, voice);
        outMorph[voice] = accumulateMorph(osc_A, voice);
        outBitcrusher[voice] = accumulateBitcrusher(osc_A, voice);

        float level = accumulateLevel(osc_A, voice);
        switch (filterSwitch) {
            case 0:
                outLevelSteiner[voice] = level;
                outLevelLadder[voice] = 0;
                break;
            case 1:
                outLevelSteiner[voice] = 0;
                outLevelLadder[voice] = level;
                break;
            case 2:
                outLevelSteiner[voice] = level;
                outLevelLadder[voice] = level;
                break;
            case 3:
                outLevelSteiner[voice] = 0;
                outLevelLadder[voice] = 0;
                break;
        }
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
inline float accumulateMorph(OSC_B &osc_b, uint16_t voice) {
    return testFloat(osc_b.iMorph.currentSample[voice] + osc_b.aMorph.valueMapped, osc_b.aMorph.min, osc_b.aMorph.max);
}

inline int32_t accumulateOctave(OSC_B &osc_b, uint16_t voice) {
    return testInt(std::roundf(osc_b.iOctave.currentSample[voice] + (float)osc_b.dOctave.valueMapped),
                   osc_b.dOctave.min, osc_b.dOctave.max);
}

inline float accumulateNote(OSC_B &osc_b, uint16_t voice) {
    float note;

    // A0 = 0, A1 = 1, ...
    // TODO can be done for both osc at once
    note = (float)(layerA.midi.rawNote[voice] - 21) / (float)12.0f;

    // TODO settings pitchbend range missing
    note += layerA.midi.oPitchbend.currentSample[voice];

    note += accumulateOctave(osc_b, voice);

    note += osc_b.iFM.currentSample[voice];

    return fastNoteLin2Log_f32(note);
}

void renderOSC_B(OSC_B &osc_B) {

    // TODO sine wave out for testing

    float *outNote = osc_B.note.nextSample;
    float *outMorph = osc_B.morph.nextSample;
    float *outBitcrusher = osc_B.bitcrusher.nextSample;
    float *outLevelSteiner = osc_B.levelSteiner.nextSample;
    float *outLevelLadder = osc_B.levelLadder.nextSample;
    int32_t &filterSwitch = osc_B.dVcfDestSwitch.valueMapped;

    for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {

        outNote[voice] = accumulateNote(osc_B, voice);
        outMorph[voice] = accumulateMorph(osc_B, voice);
        outBitcrusher[voice] = accumulateBitcrusher(osc_B, voice);

        float level = accumulateLevel(osc_B, voice);
        switch (filterSwitch) {
            case 0:
                outLevelSteiner[voice] = level;
                outLevelLadder[voice] = 0;
                break;
            case 1:
                outLevelSteiner[voice] = 0;
                outLevelLadder[voice] = level;
                break;
            case 2:
                outLevelSteiner[voice] = level;
                outLevelLadder[voice] = level;
                break;
            case 3:
                outLevelSteiner[voice] = 0;
                outLevelLadder[voice] = 0;
                break;
        }
    }
}

#endif