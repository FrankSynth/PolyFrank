#ifdef POLYRENDER

#include "renderOSC.hpp"
#include "math/polyMath.hpp"
#include "renderCVDef.h"
#include "wavetables/wavetables.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateBitcrusher() {
    return clamp(layerA.oscA.iBitcrusher + layerA.oscA.aBitcrusher, layerA.oscA.aBitcrusher.min,
                 layerA.oscA.aBitcrusher.max);
}
inline vec<VOICESPERCHIP> accumulateSamplecrusher() {
    return clamp(layerA.oscA.iSamplecrusher + layerA.oscA.aSamplecrusher, layerA.oscA.aSamplecrusher.min,
                 layerA.oscA.aSamplecrusher.max);
}
inline vec<VOICESPERCHIP> accumulateMorph() {
    return clamp(layerA.oscA.iMorph + layerA.oscA.aMorph, layerA.oscA.aMorph.min, layerA.oscA.aMorph.max);
}

inline vec<VOICESPERCHIP> accumulateOctave() {
    return clamp(round(layerA.oscA.iOctave + (float)layerA.oscA.dOctave), layerA.oscA.dOctave.min,
                 layerA.oscA.dOctave.max);
}

vec<VOICESPERCHIP, float> noteConverted;

inline vec<VOICESPERCHIP> accumulateNote() {

    noteConverted = (((vec<VOICESPERCHIP>)layerA.midi.rawNote) - 21.0f) / 12.0f;
    noteConverted += layerA.oscA.aMasterTune;

    static vec<VOICESPERCHIP> currentNote;
    static vec<VOICESPERCHIP> desiredNote;

    desiredNote = noteConverted;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[0][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    desiredNote += layerA.feel.detune;
    desiredNote += accumulateOctave();

    // // TODO check glide, plus glide settings, i.e. CT & CR
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] = std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
        else
            currentNote[i] = std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);

    // currentNote = currentNote +
    //               (1.0f / (layerA.feel.glide.currentSample * SECONDSPERCVRENDER * 10.0f)) * (desiredNote -
    //               currentNote);

    vec<VOICESPERCHIP> note = currentNote + (layerA.oscA.iFM * 0.25f);

    note += layerA.midi.oPitchbend * layerA.layersettings.dPitchbendRange;

    // static uint32_t cacheSubTable = 100;

    layerA.oscA.subWavetable = (vec<VOICESPERCHIP, uint32_t>)clamp(
        (note * ((float)SUBWAVETABLES / 10.0f) + 1.0f), 0.0f,
        (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor

    // if (cacheSubTable != layerA.oscA.subWavetable[0]) {
    //     cacheSubTable = layerA.oscA.subWavetable[0];
    //     println(layerA.oscA.subWavetable[0]); // TODO remove print once done
    // }

    layerA.sub.oscANote = note; // to calc subtable

    vec<VOICESPERCHIP> logNote;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        logNote[i] = fastNoteLin2Log_f32(note[i]);

    return logNote;
}

void renderOSC_A() {
    layerA.oscA.note = accumulateNote();
    layerA.oscA.fm = layerA.oscA.iFM;
    layerA.oscA.morphRAW = accumulateMorph();
    layerA.oscA.morph = layerA.oscA.morphRAW * (float)(WAVETABLESPERVOICE - 1);
    layerA.oscA.bitcrusher = accumulateBitcrusher();
    layerA.oscA.bitcrusherInv = 1.0f / layerA.oscA.bitcrusher.currentSample;
    layerA.oscA.samplecrusher = accumulateSamplecrusher();

    layerA.oscA.waveTableSelectionLower = (vec<VOICESPERCHIP>)layerA.oscA.morph;
    layerA.oscA.waveTableSelectionUpper = ceil((vec<VOICESPERCHIP>)layerA.oscA.morph);

    layerA.oscA.morphFract = layerA.oscA.morph - layerA.oscA.waveTableSelectionLower;
}

///////////////////////////// OSC B /////////////////////////////////

inline vec<VOICESPERCHIP> accumulateBitcrusherOscB() {
    return clamp(layerA.oscB.iBitcrusher + layerA.oscB.aBitcrusher, layerA.oscB.aBitcrusher.min,
                 layerA.oscB.aBitcrusher.max);
}
inline vec<VOICESPERCHIP> accumulateSamplecrusherOscB() {
    return clamp(layerA.oscB.iSamplecrusher + layerA.oscB.aSamplecrusher, layerA.oscB.aSamplecrusher.min,
                 layerA.oscB.aSamplecrusher.max);
}
inline vec<VOICESPERCHIP> accumulateMorphOscB() {
    return clamp(layerA.oscB.iMorph + layerA.oscB.aMorph, layerA.oscB.aMorph.min, layerA.oscB.aMorph.max);
}

inline vec<VOICESPERCHIP> accumulateOctaveOscB() {
    return clamp(round(layerA.oscB.iOctave + (float)layerA.oscB.dOctave), layerA.oscB.dOctave.min,
                 layerA.oscB.dOctave.max);
}
inline vec<VOICESPERCHIP> accumulatePhaseoffsetOscB() {
    return layerA.oscB.iPhaseOffset + layerA.oscB.aPhaseoffset;
}

inline vec<VOICESPERCHIP> accumulateNoteOscB() {

    static vec<VOICESPERCHIP> currentNote;
    static vec<VOICESPERCHIP> desiredNote;

    desiredNote = noteConverted; // from oscA

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[1][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    desiredNote += layerA.oscB.aTuning;
    desiredNote += layerA.feel.detune;
    desiredNote += accumulateOctaveOscB();

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] = std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
        else
            currentNote[i] = std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
    // currentNote = currentNote +
    //               (1.0f / (layerA.feel.glide.currentSample * SECONDSPERCVRENDER * 10.0f)) * (desiredNote -
    //               currentNote);

    vec<VOICESPERCHIP> note = currentNote + (layerA.oscB.fm * 0.25f);

    note += layerA.midi.oPitchbend * layerA.layersettings.dPitchbendRange;

    layerA.oscB.subWavetable = (vec<VOICESPERCHIP, uint32_t>)clamp(
        (note * ((float)SUBWAVETABLES / 10.0f) + 1.0f), 0.0f,
        (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor

    vec<VOICESPERCHIP> logNote;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        logNote[i] = fastNoteLin2Log_f32(note[i]);

    return logNote;
}

void renderOSC_B() {
    layerA.oscB.morphRAW = accumulateMorphOscB();
    layerA.oscB.morph = layerA.oscB.morphRAW * (float)(WAVETABLESPERVOICE - 1);
    layerA.oscB.fm = layerA.oscB.iFM + layerA.oscA.iFM;
    layerA.oscB.bitcrusher = accumulateBitcrusherOscB();
    layerA.oscB.bitcrusherInv = 1.0f / layerA.oscB.bitcrusher.currentSample;
    layerA.oscB.samplecrusher = accumulateSamplecrusherOscB();
    layerA.oscB.phaseoffset = accumulatePhaseoffsetOscB();
    layerA.oscB.note = accumulateNoteOscB();

    layerA.oscB.waveTableSelectionLower = (vec<VOICESPERCHIP>)layerA.oscB.morph;
    layerA.oscB.waveTableSelectionUpper = ceil((vec<VOICESPERCHIP>)layerA.oscB.morph);
    layerA.oscB.morphFract = layerA.oscB.morph - layerA.oscB.waveTableSelectionLower;
}

#endif