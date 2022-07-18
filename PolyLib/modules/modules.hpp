#pragma once

#include "datacore/datacore.hpp"
#include "math/spline.hpp"
#include "render/renderAudioDef.h"
#include "wavetables/wavetables.hpp"
#include <vector>

// NameLists for switches
extern const std::vector<const char *> nlOnOff;
extern const std::vector<const char *> nlVCFDest;
extern const std::vector<const char *> nlSteinerModes;
extern const std::vector<const char *> nlLadderSlopes;
extern const std::vector<const char *> nlADSRShapes;
extern const std::vector<const char *> nlClockSteps;
extern const std::vector<const char *> nlSubOctaves;

typedef enum {
    MODULE_NOTDEFINED,
    MODULE_OSC,
    MODULE_LFO,
    MODULE_ADSR,
    MODULE_MIX,
    MODULE_SUB,
    MODULE_OUT,
    MODULE_MIDI,
    MODULE_FEEL,
    MODULE_NOISE,
    MODULE_VCF,
    MODULE_PHASE,
    MODULE_WAVESHAPER

} ModuleType;

// Basemodule
class BaseModule {
  public:
    BaseModule(const char *name, const char *shortName) {
        this->name = name;
        this->shortName = shortName;
    }

    inline const char *getName() { return name; }
    inline const char *getShortName() { return shortName; }

    inline std::vector<Output *> &getOutputs() { return outputs; }
    inline std::vector<Input *> &getInputs() { return inputs; }
    inline std::vector<Analog *> &getPotis() { return knobs; }
    inline std::vector<Digital *> &getSwitches() { return switches; }
    inline std::vector<Setting *> &getSettings() { return settings; }
    inline std::vector<RenderBuffer *> &getRenderBuffer() { return renderBuffer; }

    uint8_t id;
    uint8_t layerId;
    uint8_t displayVis = 1;

    const char *name;
    const char *shortName;

    std::vector<Output *> outputs;
    std::vector<Input *> inputs;
    std::vector<Analog *> knobs;
    std::vector<Digital *> switches;
    std::vector<Setting *> settings;
    std::vector<RenderBuffer *> renderBuffer;

    ModuleType moduleType = MODULE_NOTDEFINED;

    inline virtual void resetPhase(uint32_t voice) {}
    inline virtual void retrigger(uint32_t voice) {}
};

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// MODULES //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class Midi : public BaseModule {
  public:
    Midi(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&oMod);
        outputs.push_back(&oAftertouch);
        outputs.push_back(&oPitchbend);
        outputs.push_back(&oVelocity);
        outputs.push_back(&oGate);
        outputs.push_back(&oNote);

        knobs.push_back(&aMod);
        knobs.push_back(&aAftertouch);
        knobs.push_back(&aPitchbend);

        moduleType = MODULE_MIDI;

        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++)
            rawNote[voice] = 21;
    }

    Output oMod = Output("MOD", "MOD");
    Output oAftertouch = Output("AFTERTOUCH", "AT");
    Output oPitchbend = Output("PITCHBEND", "PB");
    Output oVelocity = Output("VELOCITY", "VEL");
    Output oGate = Output("GATE", "GATE");
    Output oNote = Output("NOTE", "NOTE");

    Analog aMod = Analog("MOD", 0, 1, 0, 127, 0, true, linMap);
    Analog aAftertouch = Analog("AFTERTOUCH", 0, 1, 0, 127, 0, true, linMap);
    Analog aPitchbend = Analog("PITCHBEND", -1, 1, -8191, 8191, 0, true, linMap);

    vec<VOICESPERCHIP, uint8_t> rawNote;
    vec<VOICESPERCHIP, uint8_t> rawVelocity;
    vec<VOICESPERCHIP, uint8_t> rawGate;

    inline void gateOn(uint16_t voice) { rawGate[voice] = 1; }
    inline void gateOff(uint16_t voice) { rawGate[voice] = 0; }
    inline void setNote(uint16_t voice, uint8_t note, uint8_t velocity) {
        rawNote[voice] = note;
        rawVelocity[voice] = velocity;
    }
    inline void resetAllNotes() {
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
            setNote(voice, 21, 64);
        }
    }
};

class OSC_A : public BaseModule {
  public:
    OSC_A(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iFM);
        inputs.push_back(&iMorph);
        inputs.push_back(&iOctave);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iSamplecrusher);

        knobs.push_back(&aMasterTune);
        knobs.push_back(&aMorph);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);

        switches.push_back(&dSample0);
        switches.push_back(&dSample1);
        switches.push_back(&dSample2);
        switches.push_back(&dSample3);
        switches.push_back(&dOctave);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&fm);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&morphRAW);
        renderBuffer.push_back(&morphFract);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&samplecrusher);

        moduleType = MODULE_OSC;
    }

    Output out = Output("OUT");

    Input iFM = Input("FM", "FM", &fm);                // TODO different to OSC B?????
    Input iMorph = Input("MORPH", "MORPH", &morphRAW); // TODO morph raw?
    Input iBitcrusher = Input("BITCRUSH", "BCRUSH", &bitcrusher);
    Input iOctave = Input("OCTAVE", "OCTAVE");
    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);

    Analog aMasterTune = Analog("MASTERTUNE", -1, 1, 0, true, linMap, &iFM);
    Analog aMorph = Analog("MORPH", 0, 1, 0, true, linMap, &iMorph);
    Analog aBitcrusher = Analog("BITCRUSH", 1.0f / 8388607.0f, 1, 1.0f / 8388607.0f, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 1, 0, true, linMap, &iSamplecrusher);

    Digital dSample0 = Digital("WAVE 1", 0, WAVETABLESAMOUNT - 1, 0, true, &nlWavetable);
    Digital dSample1 = Digital("WAVE 2", 0, WAVETABLESAMOUNT - 1, 1, true, &nlWavetable);
    Digital dSample2 = Digital("WAVE 3", 0, WAVETABLESAMOUNT - 1, 2, true, &nlWavetable);
    Digital dSample3 = Digital("WAVE 4", 0, WAVETABLESAMOUNT - 1, 3, true, &nlWavetable);

    Digital dOctave = Digital("OCTAVE", -4, 4, 0, true, nullptr);

    RenderBuffer note;
    RenderBuffer fm;
    RenderBuffer morph;
    RenderBuffer morphRAW;
    RenderBuffer morphFract;
    RenderBuffer bitcrusher;
    RenderBuffer bitcrusherInv;
    RenderBuffer samplecrusher;

    vec<VOICESPERCHIP, uint32_t> subWavetable;
    vec<VOICESPERCHIP, uint32_t> waveTableSelectionLower;
    vec<VOICESPERCHIP, uint32_t> waveTableSelectionUpper;
    bool newPhase[VOICESPERCHIP] = {false};
    vec<VOICESPERCHIP> phase;
};

class OSC_B : public BaseModule {
  public:
    OSC_B(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iFM);
        inputs.push_back(&iMorph);
        inputs.push_back(&iOctave);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iSamplecrusher);
        inputs.push_back(&iPhaseOffset);

        knobs.push_back(&aTuning);
        knobs.push_back(&aMorph);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);
        knobs.push_back(&aPhaseoffset);

        switches.push_back(&dSample0);
        switches.push_back(&dSample1);
        switches.push_back(&dSample2);
        switches.push_back(&dSample3);
        switches.push_back(&dOctave);
        switches.push_back(&dSync);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&fm);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&morphRAW);
        renderBuffer.push_back(&morphFract);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&samplecrusher);
        renderBuffer.push_back(&phaseoffset);

        moduleType = MODULE_OSC;
    }

    Output out = Output("OUT");

    Input iFM = Input("FM", "FM", &fm); // TODO ?? different to OSC A
    Input iMorph = Input("MORPH", "MORPH", &morphRAW);
    Input iBitcrusher = Input("BITCRUSH", "BCRUSH", &bitcrusher);
    Input iOctave = Input("OCTAVE", "OCT");
    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);
    Input iPhaseOffset = Input("PHASE", "PH OFF+", &phaseoffset);

    Analog aMorph = Analog("MORPH", 0, 1, 0, true, linMap, &iMorph);
    Analog aTuning = Analog("TUNING", -0.5, 0.5, 0, true, linMap, &iFM);
    Analog aBitcrusher = Analog("BITCRUSH", 1.0f / 8388607.0f, 1, 1.0f / 8388607.0f, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 1, 0, true, linMap, &iSamplecrusher);
    Analog aPhaseoffset = Analog("PHASE OFFSET", -1, 1, 0, true, linMap, &iPhaseOffset);

    Digital dOctave = Digital("OCT", -4, 4, 0, true, nullptr, &iOctave);
    Digital dSync = Digital("SYNC", 0, 1, 0, true, &nlOnOff);

    Digital dSample0 = Digital("WAVE 1", 0, WAVETABLESAMOUNT - 1, 0, true, &nlWavetable);
    Digital dSample1 = Digital("WAVE 2", 0, WAVETABLESAMOUNT - 1, 1, true, &nlWavetable);
    Digital dSample2 = Digital("WAVE 3", 0, WAVETABLESAMOUNT - 1, 2, true, &nlWavetable);
    Digital dSample3 = Digital("WAVE 4", 0, WAVETABLESAMOUNT - 1, 3, true, &nlWavetable);

    RenderBuffer note;
    RenderBuffer fm;
    RenderBuffer morph;
    RenderBuffer morphRAW;
    RenderBuffer morphFract;
    RenderBuffer bitcrusher;
    RenderBuffer bitcrusherInv;
    RenderBuffer samplecrusher;
    RenderBuffer phaseoffset;

    vec<VOICESPERCHIP, uint32_t> subWavetable;
    vec<VOICESPERCHIP, uint32_t> waveTableSelectionLower;
    vec<VOICESPERCHIP, uint32_t> waveTableSelectionUpper;
    bool newPhase[VOICESPERCHIP] = {false};
};

class Sub : public BaseModule {
  public:
    Sub(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iShape);
        // inputs.push_back(&iBitcrusher);
        // inputs.push_back(&iSamplecrusher);

        knobs.push_back(&aShape);
        // knobs.push_back(&aBitcrusher);
        // knobs.push_back(&aSamplecrusher);

        switches.push_back(&dOctaveSwitch);

        renderBuffer.push_back(&shape);
        // renderBuffer.push_back(&bitcrusher);
        // renderBuffer.push_back(&samplecrusher);
        moduleType = MODULE_SUB;
    }

    Output out = Output("OUT");

    Input iShape = Input("SHAPE", "SHAPE", &shape);
    // Input iBitcrusher = Input("BITCRUSH", "BCRUSH", &bitcrusher);
    // Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);

    Analog aShape = Analog("SHAPE", 0.01f, 1, 0.01f, true, linMap, &iShape);
    // Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    // Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 1, 0, true, linMap, &iSamplecrusher);

    Digital dOctaveSwitch = Digital("OscA", 0, 1, 0, true, &nlSubOctaves);

    RenderBuffer shape;
    vec<VOICESPERCHIP, uint32_t> subWavetable;
    float phaseLength;
    vec<VOICESPERCHIP> oscANote;

    // RenderBuffer bitcrusher;
    // RenderBuffer samplecrusher;
};

class Noise : public BaseModule {
  public:
    Noise(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iSamplecrusher);

        knobs.push_back(&aSamplecrusher);

        renderBuffer.push_back(&samplecrusher);

        moduleType = MODULE_NOISE;
    }

    Output out = Output("OUT");

    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);

    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 1, 0, true, linMap, &iSamplecrusher);

    RenderBuffer samplecrusher;
};

class Mixer : public BaseModule {
  public:
    Mixer(const char *name, const char *shortName) : BaseModule(name, shortName) {

        inputs.push_back(&iOSCALevel);
        inputs.push_back(&iOSCBLevel);
        inputs.push_back(&iSUBLevel);
        inputs.push_back(&iNOISELevel);

        knobs.push_back(&aOSCALevel);
        knobs.push_back(&aOSCBLevel);
        knobs.push_back(&aSUBLevel);
        knobs.push_back(&aNOISELevel);

        switches.push_back(&dOSCADestSwitch);
        switches.push_back(&dOSCBDestSwitch);
        switches.push_back(&dSUBDestSwitch);
        switches.push_back(&dNOISEDestSwitch);

        renderBuffer.push_back(&oscALevelSteiner);
        renderBuffer.push_back(&oscALevelLadder);
        renderBuffer.push_back(&oscBLevelSteiner);
        renderBuffer.push_back(&oscBLevelLadder);
        renderBuffer.push_back(&subLevelSteiner);
        renderBuffer.push_back(&subLevelLadder);
        renderBuffer.push_back(&noiseLevelSteiner);
        renderBuffer.push_back(&noiseLevelLadder);

        renderBuffer.push_back(&oscALevel);
        renderBuffer.push_back(&oscBLevel);
        renderBuffer.push_back(&subLevel);
        renderBuffer.push_back(&noiseLevel);

        moduleType = MODULE_MIX;
    }

    Input iOSCALevel = Input("OSC A", "OSC A", &oscALevel);
    Input iOSCBLevel = Input("OSC B", "OSC B", &oscBLevel);
    Input iSUBLevel = Input("SUB", "SUB", &subLevel);
    Input iNOISELevel = Input("NOISE", "NOISE", &noiseLevel);

    Analog aOSCALevel = Analog("OSC A", 0, 1, 0.8, true, linMap, &iOSCALevel);
    Analog aOSCBLevel = Analog("OSC B", 0, 1, 0, true, linMap, &iOSCBLevel);
    Analog aSUBLevel = Analog("SUB", 0, 1, 0, true, linMap, &iSUBLevel);
    Analog aNOISELevel = Analog("NOISE", 0, 1, 0, true, linMap, &iNOISELevel);

    Digital dOSCADestSwitch = Digital("OSC A", 0, 3, 1, true, &nlVCFDest);
    Digital dOSCBDestSwitch = Digital("OSC B", 0, 3, 1, true, &nlVCFDest);
    Digital dSUBDestSwitch = Digital("SUB", 0, 3, 1, true, &nlVCFDest);
    Digital dNOISEDestSwitch = Digital("NOISE", 0, 3, 1, true, &nlVCFDest);

    RenderBuffer oscALevelSteiner = RenderBuffer(false);
    RenderBuffer oscALevelLadder = RenderBuffer(false);
    RenderBuffer oscBLevelSteiner = RenderBuffer(false);
    RenderBuffer oscBLevelLadder = RenderBuffer(false);
    RenderBuffer subLevelSteiner = RenderBuffer(false);
    RenderBuffer subLevelLadder = RenderBuffer(false);
    RenderBuffer noiseLevelSteiner = RenderBuffer(false);
    RenderBuffer noiseLevelLadder = RenderBuffer(false);

    RenderBuffer oscALevel;
    RenderBuffer oscBLevel;
    RenderBuffer subLevel;
    RenderBuffer noiseLevel;
};

class Steiner : public BaseModule {

  public:
    Steiner(const char *name, const char *shortName) : BaseModule(name, shortName) {
        inputs.push_back(&iCutoff);
        inputs.push_back(&iResonance);
        inputs.push_back(&iLevel);

        knobs.push_back(&aCutoff);
        knobs.push_back(&aResonance);
        knobs.push_back(&aLevel);
        knobs.push_back(&aParSer);

        switches.push_back(&dMode);

        renderBuffer.push_back(&resonance);
        renderBuffer.push_back(&level);
        renderBuffer.push_back(&levelRAW);
        renderBuffer.push_back(&cutoff);
        renderBuffer.push_back(&toLadder);

        moduleType = MODULE_VCF;
    }
    Input iCutoff = Input("CUTOFF", "CUT", &cutoff);
    Input iResonance = Input("RESONANCE", "RES", &resonance);
    Input iLevel = Input("LEVEL", "LEVEL", &levelRAW);

    Analog aCutoff = Analog("CUTOFF", 0, 1, 1, true, linMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, linMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, linMap, &iLevel);
    Analog aParSer = Analog("PAR/SER", 0, 1, 0, true, linMap);

    Digital dMode = Digital("MODE", 0, 3, 0, true, &nlSteinerModes);

    RenderBuffer resonance;
    RenderBuffer level;
    RenderBuffer levelRAW;

    RenderBuffer cutoff;
    RenderBuffer toLadder;
};

class Ladder : public BaseModule {
  public:
    Ladder(const char *name, const char *shortName) : BaseModule(name, shortName) {
        inputs.push_back(&iCutoff);
        inputs.push_back(&iResonance);
        inputs.push_back(&iLevel);

        knobs.push_back(&aCutoff);
        knobs.push_back(&aResonance);
        // knobs.push_back(&aADSR);
        knobs.push_back(&aLevel);

        switches.push_back(&dSlope);

        renderBuffer.push_back(&resonance);
        renderBuffer.push_back(&level);
        renderBuffer.push_back(&levelRAW);
        renderBuffer.push_back(&cutoff);

        moduleType = MODULE_VCF;
    }

    Input iCutoff = Input("CUTOFF", "CUT", &cutoff);
    Input iResonance = Input("RESONANCE", "RES", &resonance);
    Input iLevel = Input("LEVEL", "LEVEL", &levelRAW);

    Analog aCutoff = Analog("CUTOFF", 0, 1, 1, true, linMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, linMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, linMap, &iLevel);

    Digital dSlope = Digital("SLOPE", 0, 3, 3, true, &nlLadderSlopes);

    RenderBuffer resonance;
    RenderBuffer levelRAW;
    RenderBuffer level;
    RenderBuffer cutoff;
};

class LFO : public BaseModule {
  public:
    LFO(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iFreq);
        inputs.push_back(&iShape);
        inputs.push_back(&iAmount);

        knobs.push_back(&aFreq);
        knobs.push_back(&aShape);
        knobs.push_back(&aAmount);

        switches.push_back(&dFreq);
        switches.push_back(&dFreqSnap);
        switches.push_back(&dGateTrigger);
        switches.push_back(&dClockTrigger);
        switches.push_back(&dClockStep);
        switches.push_back(&dAlignLFOs);

        renderBuffer.push_back(&speed);
        renderBuffer.push_back(&speedRAW);
        renderBuffer.push_back(&shape);
        renderBuffer.push_back(&shapeRAW);
        renderBuffer.push_back(&amount);

        moduleType = MODULE_LFO;
    }
    Output out = Output("OUT");

    Input iFreq = Input("FM", "FM", &speedRAW);
    Input iShape = Input("SHAPE", "SHAPE", &shapeRAW);
    Input iAmount = Input("AMOUNT", "AMOUNT", &amount);

    Analog aFreq = Analog("FREQ", 0, 1, 0.6, true, linMap, &iFreq);
    Analog aShape = Analog("SHAPE", 0, 1, 0, true, linMap, &iShape);
    Analog aAmount = Analog("AMOUNT", 0, 1, 1, true, linMap, &iAmount);

    // TODO Switch  ywischen den beiden freq einstellungen, wie im UI?
    Digital dFreq = Digital("FREQ", 0, 22, 0, false, &nlClockSteps);
    Digital dFreqSnap = Digital("SNAP", 0, 1, 0, true, &nlOnOff);
    Digital dGateTrigger = Digital("SYNC G", 0, 1, 0, true, &nlOnOff);
    Digital dClockTrigger = Digital("SYNC C", 0, 1, 0, false, &nlOnOff);
    Digital dClockStep = Digital("CLOCK", 0, 22, 0, false, &nlClockSteps);
    Digital dAlignLFOs = Digital("ALIGN", 0, 1, 0, true, &nlOnOff);

    RenderBuffer speed;
    RenderBuffer speedRAW;
    RenderBuffer shape;
    RenderBuffer shapeRAW;
    RenderBuffer amount;

    vec<VOICESPERCHIP> currentTime;
    bool newPhase[VOICESPERCHIP] = {false};
    vec<VOICESPERCHIP> currentRandom;

    bool alignedRandom = false;
    uint32_t randSeed = 1;

    inline void resetPhase(uint32_t voice) {
        currentTime[voice] = 1;
        newPhase[voice] = true;
    }

    inline void retrigger(uint32_t voice) { resetPhase(voice); }

    inline void resetAllPhases() {
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetPhase(voice);
        }
    }

    inline void gateOn(uint32_t voice) {
        if (dGateTrigger.valueMapped == 1 && dAlignLFOs.valueMapped == 0) {
            resetPhase(voice);
        }
    }
};

class ADSR : public BaseModule {
  public:
    ADSR(const char *name, const char *shortName) : BaseModule(name, shortName) {

        outputs.push_back(&out);

        inputs.push_back(&iAmount);

        knobs.push_back(&aDelay);
        knobs.push_back(&aAttack);
        knobs.push_back(&aDecay);
        knobs.push_back(&aSustain);
        knobs.push_back(&aRelease);
        knobs.push_back(&aAmount);

        knobs.push_back(&aKeytrack);
        knobs.push_back(&aVelocity);
        knobs.push_back(&aShape);

        switches.push_back(&dLoop);
        switches.push_back(&dLatch);
        switches.push_back(&dReset);
        switches.push_back(&dGateTrigger);
        switches.push_back(&dClockTrigger);
        switches.push_back(&dClockStep);

        renderBuffer.push_back(&amount);

        moduleType = MODULE_ADSR;
    }

    Output out = Output("OUT");

    Input iAmount = Input("AMOUNT", "AMOUNT", &amount);

    Analog aDelay = Analog("DELAY", 0, 5, 0, true, logMap);
    Analog aAttack = Analog("ATTACK", 0.0005, 20, 0.5, true, logMap);
    Analog aDecay = Analog("DECAY", 0.0005, 30, 0.5, true, logMap);
    Analog aSustain = Analog("SUSTAIN", 0, 1, 0.6, true, linMap);
    Analog aRelease = Analog("RELEASE", 0.0005, 30, 0.5, true, logMap);
    Analog aAmount = Analog("AMOUNT", -1, 1, 0.5, true, linMap, &iAmount);

    Analog aKeytrack = Analog("KEYTRACK", 0, 1, 0, true, linMap);
    Analog aVelocity = Analog("VELOCITY", 0, 1, 0, true, linMap);
    Analog aShape = Analog("SHAPE", 0, 1, 0, true, linMap);

    Digital dLoop = Digital("LOOP", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dLatch = Digital("LATCH", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dReset = Digital("RESET", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dGateTrigger = Digital("GATE", 0, 1, 1, true, &nlOnOff, nullptr);
    Digital dClockTrigger = Digital("CLOCK", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dClockStep = Digital("CLOCK", 0, 22, 0, false, &nlClockSteps);

    RenderBuffer amount;

    inline void resetPhase(uint32_t voice) {
        if (dReset)
            resetADSR(voice);
        else
            setStatusDelay(voice);
    }

    inline void retrigger(uint32_t voice) {
        if (dClockTrigger == 0)
            return;
        resetPhase(voice);
        retriggered[voice] = 1;
    }

    inline void resetAllPhases() {
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetPhase(voice);
        }
    }
    inline void resetADSR(uint32_t voice) {
        level[voice] = 0;
        currentTime[voice] = 0;
        setStatusOff(voice);
        retriggered[voice] = 0;
    }

    inline void resetAllADSRs() {
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetADSR(voice);
        }
    }

    inline void gateOn(uint32_t voice) {
        if (dGateTrigger == 0)
            return;

        gate[voice] = 1;
        if (dReset) {
            resetADSR(voice);
        }
    }

    inline void gateOffAll() {
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++)
            gateOff(voice);
    }

    inline void gateOff(uint32_t voice) { gate[voice] = 0; }

    inline void gateOnAll() {
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
            gateOn(voice);
        }
    }

    inline void setStatusOff(uint32_t voice) { currentState[voice] = OFF; }
    inline void setStatusDelay(uint32_t voice) {
        currentState[voice] = DELAY;
        currentTime[voice] = 0;
    }
    inline void setStatusAttack(uint32_t voice) { currentState[voice] = ATTACK; }
    inline void setStatusDecay(uint32_t voice) { currentState[voice] = DECAY; }
    inline void setStatusSustain(uint32_t voice) { currentState[voice] = SUSTAIN; }
    inline void setStatusRelease(uint32_t voice) { currentState[voice] = RELEASE; }

    enum ADSR_State { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

    vec<VOICESPERCHIP, bool> gate;
    vec<VOICESPERCHIP, bool> retriggered;
    vec<VOICESPERCHIP> level;
    vec<VOICESPERCHIP> currentTime;
    // vec<VOICESPERCHIP> sustain;
    ADSR_State currentState[VOICESPERCHIP] = {OFF};

    float attackRate;
    float decayRate;
    float releaseRate;
    float attackCoef;
    float decayCoef;
    float releaseCoef;
    float attackBase;
    float decayBase;
    float releaseBase;
};

class Feel : public BaseModule {
  public:
    Feel(const char *name, const char *shortName) : BaseModule(name, shortName) {

        inputs.push_back(&iGlide);
        inputs.push_back(&iDetune);

        outputs.push_back(&oSpread);

        knobs.push_back(&aGlide);
        knobs.push_back(&aDetune);
        knobs.push_back(&aSpread);
        knobs.push_back(&aImperfection);

        renderBuffer.push_back(&glide);
        renderBuffer.push_back(&detune);

        moduleType = MODULE_FEEL;
    }

    Input iGlide = Input("GLIDE", "GLIDE", &glide);
    Input iDetune = Input("DETUNE", "DETUNE", &detune);

    Output oSpread = Output("SPREAD");

    Analog aGlide = Analog("GLIDE", 0.0001, 10, 0, true, linMap, &iGlide);
    Analog aDetune = Analog("DETUNE", 0, 1, .03, true, linMap, &iDetune);
    Analog aSpread = Analog("SPREAD", 0, 1, 0, true, linMap);
    Analog aImperfection = Analog("HUMANIZE", 0, 1, 0.1, true, linMap);

    RenderBuffer glide;
    RenderBuffer detune;
};

class Out : public BaseModule {

  public:
    Out(const char *name, const char *shortName) : BaseModule(name, shortName) {

        inputs.push_back(&iDistort);
        inputs.push_back(&iVCA);
        inputs.push_back(&iPan);

        knobs.push_back(&aDistort);
        knobs.push_back(&aVCA);
        knobs.push_back(&aPan);
        knobs.push_back(&aPanSpread);
        knobs.push_back(&aMaster);

        renderBuffer.push_back(&distort);
        renderBuffer.push_back(&left);
        renderBuffer.push_back(&right);
        renderBuffer.push_back(&vca);
        renderBuffer.push_back(&pan);

        moduleType = MODULE_OUT;
    }

    Input iDistort = Input("DRIVE", "DRIVE", &distort);
    Input iVCA = Input("VCA", "VCA", &vca);
    Input iPan = Input("PAN", "PAN", &pan);

    Analog aDistort = Analog("DRIVE", 0, 1, 0, true, linMap, &iDistort);
    Analog aVCA = Analog("VCA", 0, 1, 0, true, linMap, &iVCA);
    Analog aPan = Analog("PAN", -1, 1, 0, true, linMap, &iPan);
    Analog aPanSpread = Analog("PANSPREAD", 0, 1, 0, true);
    Analog aMaster = Analog("MASTER", 0, 1, 0.8, true, linMap);

    RenderBuffer left = RenderBuffer(false);
    RenderBuffer right = RenderBuffer(false);
    RenderBuffer vca;
    RenderBuffer pan;
    RenderBuffer distort;
};

class Waveshaper : public BaseModule {
  public:
    Waveshaper(const char *name, const char *shortName) : BaseModule(name, shortName) {

        inputs.push_back(&iPoint1X);
        inputs.push_back(&iPoint1Y);
        inputs.push_back(&iPoint2X);
        inputs.push_back(&iPoint2Y);
        inputs.push_back(&iPoint3X);
        inputs.push_back(&iPoint3Y);
        inputs.push_back(&iPoint4Y);
        inputs.push_back(&iDryWet);

        knobs.push_back(&aPoint1X);
        knobs.push_back(&aPoint1Y);
        knobs.push_back(&aPoint2X);
        knobs.push_back(&aPoint2Y);
        knobs.push_back(&aPoint3X);
        knobs.push_back(&aPoint3Y);
        knobs.push_back(&aPoint4Y);
        knobs.push_back(&aDryWet);

        renderBuffer.push_back(&Point1X);
        renderBuffer.push_back(&Point1Y);
        renderBuffer.push_back(&Point2X);
        renderBuffer.push_back(&Point2Y);
        renderBuffer.push_back(&Point3X);
        renderBuffer.push_back(&Point3Y);
        renderBuffer.push_back(&Point4Y);
        renderBuffer.push_back(&DryWet);

        moduleType = MODULE_WAVESHAPER;

        for (int i = 0; i < VOICESPERCHIP; i++) {
            splineX[i].resize(5);
            splineY[i].resize(5);

            splineX[i][0] = 0.0f;
            splineX[i][1] = 0.25f;
            splineX[i][2] = 0.5f;
            splineX[i][3] = 0.75f;
            splineX[i][4] = 1.0f;

            splineY[i][0] = 0.0f;
            splineY[i][1] = 0.25f;
            splineY[i][2] = 0.5f;
            splineY[i][3] = 0.75f;
            splineY[i][4] = 1.0f;

            wavespline[i] = tk::spline(splineX[i], splineY[i], tk::spline::cspline_hermite, false,
                                       tk::spline::second_deriv, 0.0f, tk::spline::second_deriv, 0.0f);
        }
    }

    Input iPoint1X = Input("P1 X", "P1 X", &Point1X);
    Input iPoint1Y = Input("P1 Y", "P1 Y", &Point1Y);
    Input iPoint2X = Input("P2 X", "P2 X", &Point2X);
    Input iPoint2Y = Input("P2 Y", "P2 Y", &Point2Y);
    Input iPoint3X = Input("P3 X", "P3 X", &Point3X);
    Input iPoint3Y = Input("P3 Y", "P3 Y", &Point3Y);
    Input iPoint4Y = Input("P4 Y", "P4 Y", &Point4Y);
    Input iDryWet = Input("Dry/Wet", "Dry/Wet", &DryWet);

    Analog aPoint1X = Analog("P1 X", 0, 1, 0.25, true, linMap, &iPoint1X);
    Analog aPoint1Y = Analog("P1 Y", 0, 1, 0.25, true, linMap, &iPoint1Y);
    Analog aPoint2X = Analog("P2 X", 0, 1, 0.5, true, linMap, &iPoint2X);
    Analog aPoint2Y = Analog("P2 Y", 0, 1, 0.5, true, linMap, &iPoint2Y);
    Analog aPoint3X = Analog("P3 X", 0, 1, 0.75, true, linMap, &iPoint3X);
    Analog aPoint3Y = Analog("P3 Y", 0, 1, 0.75, true, linMap, &iPoint3Y);
    Analog aPoint4Y = Analog("P4 Y", 0, 1, 1, true, linMap, &iPoint4Y);
    Analog aDryWet = Analog("Dry/Wet", 0, 1, 0, true, linMap, &iDryWet);

    RenderBuffer Point1X;
    RenderBuffer Point1Y;
    RenderBuffer Point2X;
    RenderBuffer Point2Y;
    RenderBuffer Point3X;
    RenderBuffer Point3Y;
    RenderBuffer Point4Y;
    RenderBuffer DryWet;

    std::vector<float> splineX[VOICESPERCHIP];
    std::vector<float> splineY[VOICESPERCHIP];

    tk::spline wavespline[VOICESPERCHIP];
};

class Phaseshaper : public BaseModule {
  public:
    Phaseshaper(const char *name, const char *shortName) : BaseModule(name, shortName) {

        inputs.push_back(&iPoint1Y);
        inputs.push_back(&iPoint2X);
        inputs.push_back(&iPoint2Y);
        inputs.push_back(&iPoint3X);
        inputs.push_back(&iPoint3Y);
        inputs.push_back(&iPoint4Y);
        inputs.push_back(&iDryWet);

        knobs.push_back(&aPoint1Y);
        knobs.push_back(&aPoint2X);
        knobs.push_back(&aPoint2Y);
        knobs.push_back(&aPoint3X);
        knobs.push_back(&aPoint3Y);
        knobs.push_back(&aPoint4Y);
        knobs.push_back(&aDryWet);

        renderBuffer.push_back(&Point1Y);
        renderBuffer.push_back(&Point2X);
        renderBuffer.push_back(&Point2Y);
        renderBuffer.push_back(&Point3X);
        renderBuffer.push_back(&Point3Y);
        renderBuffer.push_back(&Point4Y);
        renderBuffer.push_back(&DryWet);

        moduleType = MODULE_PHASE;
    }

    Input iPoint1Y = Input("P1 Y", "P1 Y", &Point1Y);
    Input iPoint2X = Input("P2 X", "P2 X", &Point2X);
    Input iPoint2Y = Input("P2 Y", "P2 Y", &Point2Y);
    Input iPoint3X = Input("P3 X", "P3 X", &Point3X);
    Input iPoint3Y = Input("P3 Y", "P3 Y", &Point3Y);
    Input iPoint4Y = Input("P4 Y", "P4 Y", &Point4Y);
    Input iDryWet = Input("Dry/Wet", "Dry/Wet", &DryWet);

    Analog aPoint1Y = Analog("P1 Y", 0, 0.99999f, 0, true, linMap, &iPoint1Y); // TODO does this work, with 0.9999?
    Analog aPoint2X = Analog("P2 X", 0, 0.99999f, 1.0f / 3.0f, true, linMap, &iPoint2X);
    Analog aPoint2Y = Analog("P2 Y", 0, 0.99999f, 1.0f / 3.0f, true, linMap, &iPoint2Y);
    Analog aPoint3X = Analog("P3 X", 0, 0.99999f, 2.0f / 3.0f, true, linMap, &iPoint3X);
    Analog aPoint3Y = Analog("P3 Y", 0, 0.99999f, 2.0f / 3.0f, true, linMap, &iPoint3Y);
    Analog aPoint4Y = Analog("P4 Y", 0, 0.99999f, 0.99999f, true, linMap, &iPoint4Y);
    Analog aDryWet = Analog("Dry/Wet", 0, 1, 0, true, linMap, &iDryWet);

    RenderBuffer Point1Y;
    RenderBuffer Point2X;
    RenderBuffer Point2Y;
    RenderBuffer Point3X;
    RenderBuffer Point3Y;
    RenderBuffer Point4Y;
    RenderBuffer DryWet;
};

class LayerSetting : public BaseModule {

  public:
    LayerSetting(const char *name, const char *shortName) : BaseModule(name, shortName) {
        switches.push_back(&dPitchbendRange);
    }

    Digital dPitchbendRange = Digital("PITCHBEND RANGE", 1, 24, 1, true, nullptr, nullptr);
};
