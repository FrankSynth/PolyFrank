#ifdef POLYRENDER

#include "renderOSC.hpp"
#include "math/polyMath.hpp"
#include "renderCVDef.h"
#include "wavetables/wavetables.hpp"

extern Layer layerA;

inline vec<VOICESPERCHIP> accumulateBitcrusher(const OSC_A &osc_a) {
    return clamp(osc_a.iBitcrusher + osc_a.aBitcrusher, osc_a.aBitcrusher.min, osc_a.aBitcrusher.max);
}
inline vec<VOICESPERCHIP> accumulateSamplecrusher(const OSC_A &osc_a) {
    return clamp(osc_a.iSamplecrusher + osc_a.aSamplecrusher, osc_a.aSamplecrusher.min, osc_a.aSamplecrusher.max);
}
inline vec<VOICESPERCHIP> accumulateMorph(const OSC_A &osc_a) {
    return clamp(osc_a.iMorph + osc_a.aMorph, osc_a.aMorph.min, osc_a.aMorph.max);
}

inline vec<VOICESPERCHIP> accumulateOctave(const OSC_A &osc_a) {
    return clamp(round(osc_a.iOctave + (float)osc_a.dOctave), osc_a.dOctave.min, osc_a.dOctave.max);
}

vec<VOICESPERCHIP, float> noteConverted;

inline vec<VOICESPERCHIP> accumulateNote(OSC_A &osc_a) {

    noteConverted = layerA.oscA.aMasterTune;
    noteConverted += (osc_a.iFM * 0.25); // TODO ??
    osc_a.fm = noteConverted;

    noteConverted += (((vec<VOICESPERCHIP>)layerA.midi.rawNote) - 21.0f) / 12.0f;

    static vec<VOICESPERCHIP> currentNote;
    static vec<VOICESPERCHIP> desiredNote;

    desiredNote = noteConverted;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[0][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    desiredNote += accumulateOctave(osc_a);

    desiredNote += (vec<VOICESPERCHIP> &)layerA.feel.detune;

    // // TODO check glide, plus glide settings, i.e. CT & CR
    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] = std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
        else
            currentNote[i] = std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);

    // currentNote = currentNote +
    //               (1.0f / (layerA.feel.glide.currentSample * SECONDSPERCVRENDER * 10.0f)) * (desiredNote -
    //               currentNote);

    vec<VOICESPERCHIP> note = currentNote;

    note += layerA.midi.oPitchbend * layerA.layersettings.dPitchbendRange;

    static uint32_t cacheSubTable = 100;

    osc_a.subWavetable = (vec<VOICESPERCHIP, uint32_t>)clamp(
        (note * ((float)SUBWAVETABLES / 10.0f) + 1.0f), 0.0f,
        (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor

    if (cacheSubTable != osc_a.subWavetable[0]) {
        cacheSubTable = osc_a.subWavetable[0];
        println(osc_a.subWavetable[0]); // TODO remove print once done
    }

    layerA.sub.oscANote = note;

    vec<VOICESPERCHIP> logNote;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        logNote[i] = fastNoteLin2Log_f32(note[i]);

    return logNote;
}

void renderOSC_A(OSC_A &osc_A) {
    osc_A.note = accumulateNote(osc_A);
    osc_A.morphRAW = accumulateMorph(osc_A);
    osc_A.morph = osc_A.morphRAW * (float)(WAVETABLESPERVOICE - 1);
    osc_A.bitcrusher = accumulateBitcrusher(osc_A);
    osc_A.bitcrusherInv = 1.0f / osc_A.bitcrusher.currentSample;
    osc_A.samplecrusher = accumulateSamplecrusher(osc_A);

    osc_A.waveTableSelectionLower = (vec<VOICESPERCHIP>)osc_A.morph;
    osc_A.waveTableSelectionUpper = ceil((vec<VOICESPERCHIP>)osc_A.morph);

    osc_A.morphFract = osc_A.morph - osc_A.waveTableSelectionLower;
}

///////////////////////////// OSC B /////////////////////////////////

inline vec<VOICESPERCHIP> accumulateBitcrusher(const OSC_B &osc_b) {
    return clamp(osc_b.iBitcrusher + osc_b.aBitcrusher, osc_b.aBitcrusher.min, osc_b.aBitcrusher.max);
}
inline vec<VOICESPERCHIP> accumulateSamplecrusher(const OSC_B &osc_b) {
    return clamp(osc_b.iSamplecrusher + osc_b.aSamplecrusher, osc_b.aSamplecrusher.min, osc_b.aSamplecrusher.max);
}
inline vec<VOICESPERCHIP> accumulateMorph(const OSC_B &osc_b) {
    return clamp(osc_b.iMorph + osc_b.aMorph, osc_b.aMorph.min, osc_b.aMorph.max);
}

inline vec<VOICESPERCHIP> accumulateOctave(const OSC_B &osc_b) {
    return clamp(round(osc_b.iOctave + (float)osc_b.dOctave), osc_b.dOctave.min, osc_b.dOctave.max);
}
inline vec<VOICESPERCHIP> accumulatePhaseoffset(const OSC_B &osc_b) {
    return osc_b.iPhaseOffset + osc_b.aPhaseoffset;
}

inline vec<VOICESPERCHIP> accumulateNote(OSC_B &osc_b) {

    static vec<VOICESPERCHIP> currentNote;
    static vec<VOICESPERCHIP> desiredNote;

    desiredNote = noteConverted;

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        desiredNote[i] += layerA.noteImperfection[1][i][layerA.midi.rawNote[i]] * layerA.feel.aImperfection.valueMapped;

    desiredNote += layerA.oscB.aTuning;

    // desiredNote += layerA.oscA.aMasterTune;

    desiredNote += layerA.feel.detune;

    desiredNote += accumulateOctave(osc_b);

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        if (currentNote[i] < desiredNote[i])
            currentNote[i] = std::min(currentNote[i] + SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
        else
            currentNote[i] = std::max(currentNote[i] - SECONDSPERCVRENDER / layerA.feel.glide[i], desiredNote[i]);
    // currentNote = currentNote +
    //               (1.0f / (layerA.feel.glide.currentSample * SECONDSPERCVRENDER * 10.0f)) * (desiredNote -
    //               currentNote);

    vec<VOICESPERCHIP> note = currentNote;

    note += osc_b.iFM;
    note += layerA.midi.oPitchbend * layerA.layersettings.dPitchbendRange;

    osc_b.subWavetable = (vec<VOICESPERCHIP, uint32_t>)clamp(
        (note * ((float)SUBWAVETABLES / 10.0f) + 1.0f), 0.0f,
        (float)(SUBWAVETABLES - 1)); // TODO with round? standardrange is  10, so we divide to get the factor

    for (uint16_t i = 0; i < VOICESPERCHIP; i++)
        note[i] = fastNoteLin2Log_f32(note[i]);

    return note;
}

void renderOSC_B(OSC_B &osc_B) {
    osc_B.note = accumulateNote(osc_B);
    osc_B.morphRAW = accumulateMorph(osc_B);
    osc_B.morph = osc_B.morphRAW * (float)(WAVETABLESPERVOICE - 1);
    osc_B.bitcrusher = accumulateBitcrusher(osc_B);
    osc_B.bitcrusherInv = 1.0f / osc_B.bitcrusher.currentSample;
    osc_B.samplecrusher = accumulateSamplecrusher(osc_B);
    osc_B.phaseoffset = accumulatePhaseoffset(osc_B);

    osc_B.waveTableSelectionLower = (vec<VOICESPERCHIP>)osc_B.morph;
    osc_B.waveTableSelectionUpper = ceil((vec<VOICESPERCHIP>)osc_B.morph);
    osc_B.morphFract = osc_B.morph - osc_B.waveTableSelectionLower;
}

#endif