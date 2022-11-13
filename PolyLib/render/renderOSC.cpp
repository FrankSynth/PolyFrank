#ifdef POLYRENDER

#include "renderOSC.hpp"
#include "math/polyMath.hpp"
#include "renderCVDef.h"
#include "wavetables/wavetables.hpp"

extern Layer layerA;

// TODO Accumulate ohne input raus
inline vec<VOICESPERCHIP> accumulateBitcrusher() {
    return clamp(layerA.oscA.effect * (layerA.oscA.iBitcrusher + layerA.oscA.aBitcrusher), layerA.oscA.aBitcrusher.min,
                 layerA.oscA.aBitcrusher.max);
}
inline vec<VOICESPERCHIP> accumulateSamplecrusher() {
    return clamp(layerA.oscA.effect * (layerA.oscA.iSamplecrusher + layerA.oscA.aSamplecrusher),
                 layerA.oscA.aSamplecrusher.min, layerA.oscA.aSamplecrusher.max);
}

inline vec<VOICESPERCHIP> accumulateEffect() {
    return clamp(layerA.oscA.iEffect + layerA.oscA.aEffect, layerA.oscA.aEffect.min, layerA.oscA.aEffect.max);
}

inline vec<VOICESPERCHIP> accumulateMorph() {
    return layerA.oscA.iMorph + layerA.oscA.aMorph;
}

inline vec<VOICESPERCHIP> accumulateOctave() {
    return clamp((vec<VOICESPERCHIP>)(float)layerA.oscA.dOctave, layerA.oscA.dOctave.min, layerA.oscA.dOctave.max);
}

vec<VOICESPERCHIP, float> noteConverted;

inline vec<VOICESPERCHIP> accumulateNote() {

    noteConverted = (((vec<VOICESPERCHIP>)layerA.midi.rawNote) - 21.0f) / 12.0f;

    static vec<VOICESPERCHIP> currentNote;
    static vec<VOICESPERCHIP> desiredNote;

    desiredNote = noteConverted;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[0][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    desiredNote += accumulateOctave() + layerA.oscA.fm;

    // // TODO check glide, plus glide settings, i.e. CT & CR
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] = std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
        else
            currentNote[i] = std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);

    vec<VOICESPERCHIP> note = currentNote + (layerA.oscB.iFM * 0.25f);

    // note += layerA.midi.oPitchbend * layerA.layersettings.dPitchbendRange;

    layerA.oscA.subWavetable = (vec<VOICESPERCHIP, uint32_t>)clamp((note * ((float)SUBWAVETABLES / 10.0f) + 1.0f), 0.0f,
                                                                   (float)(SUBWAVETABLES - 1));

    layerA.oscA.oscNote = note; // to calc subtable

    vec<VOICESPERCHIP> logNote;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        logNote[i] = fastNoteLin2Log_f32(note[i]);

    return logNote;
}

inline vec<VOICESPERCHIP> accumulateShapeSub() {
    return clamp(layerA.oscA.iShapeSub + layerA.oscA.aShapeSub, layerA.oscA.aShapeSub.min, layerA.oscA.aShapeSub.max);
}

void renderOSC_A() {
    layerA.oscA.fm = layerA.oscA.iFM + layerA.feel.detune + layerA.oscA.aMasterTune;

    layerA.oscA.note = accumulateNote();
    layerA.oscA.morphRAW = accumulateMorph();
    layerA.oscA.morphRAW = layerA.oscA.morphRAW - floor((vec<VOICESPERCHIP>)layerA.oscA.morphRAW);

    layerA.oscA.morph = ((layerA.oscA.morphRAW) * (float)(WAVETABLESPERVOICE));

    layerA.oscA.effect = accumulateEffect();

    layerA.oscA.bitcrusher = accumulateBitcrusher();
    layerA.oscA.bitcrusherInv = 1.0f / layerA.oscA.bitcrusher.currentSample;
    layerA.oscA.samplecrusher = accumulateSamplecrusher();

    layerA.oscA.waveTableSelectionLower = ((vec<VOICESPERCHIP, uint32_t>)((vec<VOICESPERCHIP>)layerA.oscA.morph));

    layerA.oscA.waveTableSelectionUpper = (layerA.oscA.waveTableSelectionLower + 1u) & 0b11;

    layerA.oscA.morphFract = layerA.oscA.morph - floor((vec<VOICESPERCHIP>)layerA.oscA.morph);

    ////// SUB
    layerA.oscA.shapeSub = accumulateShapeSub();
    layerA.oscA.phaseLengthSub = 0.5f * !layerA.oscA.dOctaveSwitchSub + 0.25f * layerA.oscA.dOctaveSwitchSub;

    layerA.oscA.subWavetable =
        clamp(round((layerA.oscA.oscNote - (layerA.oscA.dOctaveSwitchSub + 1)) * ((float)SUBWAVETABLES / 10.0f)), 0.0f,
              (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor
}

///////////////////////////// OSC B /////////////////////////////////

inline vec<VOICESPERCHIP> accumulateBitcrusherOscB() {
    return clamp(layerA.oscB.effect * (layerA.oscB.iBitcrusher + layerA.oscB.aBitcrusher), layerA.oscB.aBitcrusher.min,
                 layerA.oscB.aBitcrusher.max);
}
inline vec<VOICESPERCHIP> accumulateSamplecrusherOscB() {
    return clamp(layerA.oscB.effect * (layerA.oscB.iSamplecrusher + layerA.oscB.aSamplecrusher),
                 layerA.oscB.aSamplecrusher.min, layerA.oscB.aSamplecrusher.max);
}
inline vec<VOICESPERCHIP> accumulateMorphOscB() {
    return layerA.oscB.iMorph + layerA.oscB.aMorph;
}

inline vec<VOICESPERCHIP> accumulateEffectOscB() {
    return clamp(layerA.oscB.iEffect + layerA.oscB.aEffect, layerA.oscB.aEffect.min, layerA.oscB.aEffect.max);
}

inline vec<VOICESPERCHIP> accumulateOctaveOscB() {
    return clamp((vec<VOICESPERCHIP>)round((float)layerA.oscB.dOctave), layerA.oscB.dOctave.min,
                 layerA.oscB.dOctave.max);
}
inline vec<VOICESPERCHIP> accumulatePhaseoffsetOscB() {
    return layerA.oscB.iPhaseOffset + layerA.oscB.aPhaseoffset;
}

inline vec<VOICESPERCHIP> accumulateNoteOscB() {

    static vec<VOICESPERCHIP> currentNote;
    static vec<VOICESPERCHIP> desiredNote;

    desiredNote = noteConverted; // from oscA

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[1][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    desiredNote += layerA.oscA.fm + layerA.oscB.fm;
    desiredNote += accumulateOctaveOscB();

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] = std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
        else
            currentNote[i] = std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);

    vec<VOICESPERCHIP> note = currentNote;

    layerA.oscB.subWavetable = (vec<VOICESPERCHIP, uint32_t>)clamp((note * ((float)SUBWAVETABLES / 10.0f) + 1.0f), 0.0f,
                                                                   (float)(SUBWAVETABLES - 1));

    vec<VOICESPERCHIP> logNote;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        logNote[i] = fastNoteLin2Log_f32(note[i]);

    return logNote;
}

void renderOSC_B() {
    layerA.oscB.morphRAW = accumulateMorphOscB();
    layerA.oscB.effect = accumulateEffectOscB();
    layerA.oscB.morphRAW = accumulateMorphOscB();
    layerA.oscB.morphRAW = layerA.oscB.morphRAW - floor((vec<VOICESPERCHIP>)layerA.oscB.morphRAW);

    layerA.oscB.morph = ((layerA.oscB.morphRAW) * (float)(WAVETABLESPERVOICE));

    layerA.oscB.fm = layerA.oscB.iFM + layerA.oscB.aTuning;
    layerA.oscB.bitcrusher = accumulateBitcrusherOscB();
    layerA.oscB.bitcrusherInv = 1.0f / layerA.oscB.bitcrusher.currentSample;
    layerA.oscB.samplecrusher = accumulateSamplecrusherOscB();
    layerA.oscB.phaseoffset = accumulatePhaseoffsetOscB();
    layerA.oscB.note = accumulateNoteOscB();

    layerA.oscB.waveTableSelectionLower = ((vec<VOICESPERCHIP, uint32_t>)((vec<VOICESPERCHIP>)layerA.oscB.morph));

    layerA.oscB.waveTableSelectionUpper = (layerA.oscB.waveTableSelectionLower + 1u) & 0b11;

    layerA.oscB.morphFract = layerA.oscB.morph - floor((vec<VOICESPERCHIP>)layerA.oscB.morph);
}

#endif