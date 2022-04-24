#pragma once

#include "datacore/datacore.hpp"
#include "wavetables/wavetables.hpp"
#include <string>
#include <vector>

// NameLists for switches
extern const std::vector<std::string> nlOnOff;
extern const std::vector<std::string> nlVCFDest;
extern const std::vector<std::string> nlSteinerModes;
extern const std::vector<std::string> nlLadderSlopes;
extern const std::vector<std::string> nlADSRShapes;
extern const std::vector<std::string> nlClockSteps;
extern const std::vector<std::string> nlSubOctaves;

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
    MODULE_VCF

} ModuleType;

// Basemodule
class BaseModule {
  public:
    BaseModule(const char *name, const char *shortName) {
        this->name = name;
        this->shortName = shortName;
    }

    inline const std::string &getName() { return name; }
    inline const std::string &getShortName() { return shortName; }

    inline std::vector<Output *> &getOutputs() { return outputs; }
    inline std::vector<Input *> &getInputs() { return inputs; }
    inline std::vector<Analog *> &getPotis() { return knobs; }
    inline std::vector<Digital *> &getSwitches() { return switches; }
    inline std::vector<Setting *> &getSettings() { return settings; }
    inline std::vector<RenderBuffer *> &getRenderBuffer() { return renderBuffer; }

    uint8_t id;
    uint8_t layerId;
    uint8_t displayVis = 1;

    std::string name;
    std::string shortName;

    std::vector<Output *> outputs;
    std::vector<Input *> inputs;
    std::vector<Analog *> knobs;
    std::vector<Digital *> switches;
    std::vector<Setting *> settings;
    std::vector<RenderBuffer *> renderBuffer;

    ModuleType moduleType = MODULE_NOTDEFINED;

    inline virtual void resetPhase(uint16_t voice) {}
    inline virtual void retrigger(uint16_t voice) {}

  protected:
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
        rawNote[voice] = note - 21;
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
        // inputs.push_back(&iSquircle);

        knobs.push_back(&aMasterTune);
        knobs.push_back(&aMorph);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);
        // knobs.push_back(&aSquircle);

        switches.push_back(&dSample0);
        switches.push_back(&dSample1);
        switches.push_back(&dSample2);
        switches.push_back(&dSample3);
        switches.push_back(&dOctave);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&samplecrusher);
        // renderBuffer.push_back(&squircle);

        moduleType = MODULE_OSC;
    }

    Output out = Output("OUT");

    Input iFM = Input("FM", "FM", &note);
    Input iMorph = Input("MORPH", "MORPH", &morph);
    Input iBitcrusher = Input("BITCRUSH", "BCRUSH", &bitcrusher);
    Input iOctave = Input("OCTAVE", "OCTAVE");
    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);
    // Input iSquircle = Input("SQUIRCLE", "SQRCL", &squircle);

    Analog aMasterTune = Analog("MASTERTUNE", -1, 1, 0, true, linMap, &iFM);
    Analog aMorph = Analog("MORPH", 0, WAVETABLESPERVOICE - 1, 0, true, linMap, &iMorph);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);
    // Analog aSquircle = Analog("SQUIRCLE", 0, 1, 0.5, true, linMap, &iSamplecrusher);

    Digital dSample0 = Digital("WAVE 1", 0, WAVETABLESAMOUNT - 1, 0, true, &nlWavetable);
    Digital dSample1 = Digital("WAVE 2", 0, WAVETABLESAMOUNT - 1, 1, true, &nlWavetable);
    Digital dSample2 = Digital("WAVE 3", 0, WAVETABLESAMOUNT - 1, 2, true, &nlWavetable);
    Digital dSample3 = Digital("WAVE 4", 0, WAVETABLESAMOUNT - 1, 3, true, &nlWavetable);

    Digital dOctave = Digital("OCTAVE", -4, 4, 0, true, nullptr);

    RenderBuffer note;
    RenderBuffer morph;
    RenderBuffer bitcrusher;
    RenderBuffer samplecrusher;
    // RenderBuffer squircle;

    bool newPhase[VOICESPERCHIP] = {false};
    vec<VOICESPERCHIP> phase;
    // vec<VOICESPERCHIP> phaseWavetableUpper;
};

class OSC_B : public BaseModule {
  public:
    OSC_B(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iFM);
        inputs.push_back(&iMorph);
        inputs.push_back(&iTuning);
        inputs.push_back(&iOctave);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iSamplecrusher);
        inputs.push_back(&iPhaseOffset);
        // inputs.push_back(&iSquircle);

        knobs.push_back(&aMorph);
        knobs.push_back(&aTuning);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);
        knobs.push_back(&aPhaseoffset);
        // knobs.push_back(&aSquircle);

        switches.push_back(&dSample0);
        switches.push_back(&dSample1);
        switches.push_back(&dSample2);
        switches.push_back(&dSample3);
        switches.push_back(&dOctave);
        switches.push_back(&dSync);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&samplecrusher);
        renderBuffer.push_back(&phaseoffset);
        // renderBuffer.push_back(&squircle);

        moduleType = MODULE_OSC;
    }

    Output out = Output("OUT");

    Input iFM = Input("FM", "FM", &note);
    Input iMorph = Input("MORPH", "MORPH", &morph);
    Input iTuning = Input("TUNING", "TUNING");
    Input iBitcrusher = Input("BITCRUSH", "BCRUSH", &bitcrusher);
    Input iOctave = Input("OCTAVE", "OCT");
    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);
    Input iPhaseOffset = Input("PHASE", "PHASE", &phaseoffset);
    // Input iSquircle = Input("SQUIRCLE", "SQRCL", &squircle);

    Analog aMorph = Analog("MORPH", 0, WAVETABLESPERVOICE - 1, 0, true, linMap, &iMorph);
    Analog aTuning = Analog("TUNING", -0.5, 0.5, 0, true, linMap, &iTuning);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);
    Analog aPhaseoffset = Analog("PHASE OFFSET", -1, 1, 0, true, linMap, &iPhaseOffset);
    // Analog aSquircle = Analog("SQUIRCLE", 0, 0.5, 1, true, linMap, &iSamplecrusher);

    Digital dOctave = Digital("OCT", -4, 4, 0, true, nullptr, &iOctave);
    Digital dSync = Digital("SYNC", 0, 1, 0, true, &nlOnOff);

    Digital dSample0 = Digital("WAVE 1", 0, WAVETABLESAMOUNT - 1, 0, true, &nlWavetable);
    Digital dSample1 = Digital("WAVE 2", 0, WAVETABLESAMOUNT - 1, 1, true, &nlWavetable);
    Digital dSample2 = Digital("WAVE 3", 0, WAVETABLESAMOUNT - 1, 2, true, &nlWavetable);
    Digital dSample3 = Digital("WAVE 4", 0, WAVETABLESAMOUNT - 1, 3, true, &nlWavetable);

    RenderBuffer note;
    RenderBuffer morph;
    RenderBuffer bitcrusher;
    RenderBuffer samplecrusher;
    RenderBuffer phaseoffset;
    // RenderBuffer squircle;

    bool newPhase[VOICESPERCHIP] = {false};
    // vec<VOICESPERCHIP> cacheOscAPhase;
    // vec<VOICESPERCHIP> phaseWavetableLower;
    // vec<VOICESPERCHIP> phaseWavetableUpper;
};

class Sub : public BaseModule {
  public:
    Sub(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iShape);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iSamplecrusher);

        knobs.push_back(&aShape);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);

        switches.push_back(&dOctaveSwitch);

        renderBuffer.push_back(&shape);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&samplecrusher);
        moduleType = MODULE_SUB;
    }

    Output out = Output("OUT");

    Input iShape = Input("SHAPE", "SHAPE", &shape);
    Input iBitcrusher = Input("BITCRUSH", "BCRUSH", &bitcrusher);
    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH", &samplecrusher);

    Analog aShape = Analog("SHAPE", 0.01f, 1, 0.01f, true, linMap, &iShape);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);

    Digital dOctaveSwitch = Digital("OscA", 0, 1, 0, true, &nlSubOctaves);

    RenderBuffer shape;
    RenderBuffer bitcrusher;
    RenderBuffer samplecrusher;

    // vec<VOICESPERCHIP> phase;
    // vec<VOICESPERCHIP> oscApreviousPhase;
};

// TODO bitrusher
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

    Input iSamplecrusher = Input("SAMPLECRUSH", "SCRUSH");

    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);

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

        moduleType = MODULE_MIX;
    }

    Input iOSCALevel = Input("OSC A", "OSC A", &oscALevel);
    Input iOSCBLevel = Input("OSC B", "OSC B", &oscBLevel);
    Input iSUBLevel = Input("SUB", "SUB", &subLevel);
    Input iNOISELevel = Input("NOISE", "NOISE", &noiseLevel);

    Analog aOSCALevel = Analog("OSC A", 0, 1, 0.8, true, logMap, &iOSCALevel);
    Analog aOSCBLevel = Analog("OSC B", 0, 1, 0, true, logMap, &iOSCBLevel);
    Analog aSUBLevel = Analog("SUB", 0, 1, 0, true, logMap, &iSUBLevel);
    Analog aNOISELevel = Analog("NOISE", 0, 1, 0, true, logMap, &iNOISELevel);

    Digital dOSCADestSwitch = Digital("OSC A", 0, 3, 1, true, &nlVCFDest);
    Digital dOSCBDestSwitch = Digital("OSC B", 0, 3, 1, true, &nlVCFDest);
    Digital dSUBDestSwitch = Digital("SUB", 0, 3, 1, true, &nlVCFDest);
    Digital dNOISEDestSwitch = Digital("NOISE", 0, 3, 1, true, &nlVCFDest);

    RenderBuffer oscALevelSteiner;
    RenderBuffer oscALevelLadder;
    RenderBuffer oscBLevelSteiner;
    RenderBuffer oscBLevelLadder;
    RenderBuffer subLevelSteiner;
    RenderBuffer subLevelLadder;
    RenderBuffer noiseLevelSteiner;
    RenderBuffer noiseLevelLadder;

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
        // knobs.push_back(&aADSR);
        knobs.push_back(&aLevel);
        knobs.push_back(&aParSer);

        switches.push_back(&dMode);

        renderBuffer.push_back(&resonance);
        renderBuffer.push_back(&level);
        renderBuffer.push_back(&cutoff);
        renderBuffer.push_back(&toLadder);

        moduleType = MODULE_VCF;
    }
    Input iCutoff = Input("CUTOFF", "CUT", &cutoff);
    Input iResonance = Input("RESONANCE", "RES", &resonance);
    Input iLevel = Input("LEVEL", "LEVEL", &level);

    Analog aCutoff = Analog("CUTOFF", 0, 1, 1, true, linMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, linMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, antilogMap, &iLevel);
    Analog aParSer = Analog("PAR/SER", 0, 1, 0, true, antilogMap);
    // Analog aADSR = Analog("ADSR", -1, 1, 0, true, linMap);

    Digital dMode = Digital("MODE", 0, 3, 0, true, &nlSteinerModes);

    RenderBuffer resonance;
    RenderBuffer level;
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
        renderBuffer.push_back(&cutoff);

        moduleType = MODULE_VCF;
    }

    Input iCutoff = Input("CUTOFF", "CUT", &cutoff);
    Input iResonance = Input("RESONANCE", "RES", &resonance);
    Input iLevel = Input("LEVEL", "LEVEL", &level);

    Analog aCutoff = Analog("CUTOFF", 0, 1, 1, true, linMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, linMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, antilogMap, &iLevel);
    // Analog aADSR = Analog("ADSR", -1, 1, 0, true, linMap);

    Digital dSlope = Digital("SLOPE", 0, 3, 0, true, &nlLadderSlopes);

    RenderBuffer resonance;
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

        moduleType = MODULE_LFO;
    }
    Output out = Output("OUT");

    Input iFreq = Input("FM", "FM", &frequency);
    Input iShape = Input("SHAPE", "SHAPE", &shape);
    Input iAmount = Input("AMOUNT", "AMOUNT", &amount);

    Analog aFreq = Analog("FREQ", 0, 1, 0.6, true, linMap, &iFreq);
    Analog aShape = Analog("SHAPE", 0, 6, 0, true, linMap, &iShape);
    Analog aAmount = Analog("AMOUNT", 0, 1, 1, true, linMap, &iAmount);

    // TODO Freq also as Digital knob??
    Digital dFreq = Digital("FREQ", 0, 22, 0, false, &nlClockSteps);
    Digital dFreqSnap = Digital("SNAP", 0, 1, 0, true, &nlOnOff);
    Digital dGateTrigger = Digital("SYNC G", 0, 1, 0, true, &nlOnOff);
    Digital dClockTrigger = Digital("SYNC C", 0, 1, 0, false, &nlOnOff);
    Digital dClockStep = Digital("CLOCK", 0, 22, 0, false, &nlClockSteps);
    Digital dAlignLFOs = Digital("ALIGN", 0, 1, 0, true, &nlOnOff);

    RenderBuffer frequency;
    RenderBuffer shape;
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
        if (dGateTrigger.valueMapped) {
            resetPhase(voice);
        }
    }
};

class ADSR : public BaseModule {
  public:
    ADSR(const char *name, const char *shortName) : BaseModule(name, shortName) {

        outputs.push_back(&out);

        inputs.push_back(&iDelay);
        inputs.push_back(&iAttack);
        inputs.push_back(&iDecay);
        inputs.push_back(&iSustain);
        inputs.push_back(&iRelease);
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

        moduleType = MODULE_ADSR;
    }

    Output out = Output("OUT");

    Input iDelay = Input("DELAY", "DELAY");
    Input iAttack = Input("ATTACK", "ATTACK");
    Input iDecay = Input("DECAY", "DECAY");
    Input iSustain = Input("SUSTAIN", "SUSTAIN");
    Input iRelease = Input("RELEASE", "RELEASE");
    Input iAmount = Input("AMOUNT", "AMOUNT", &amount);

    Analog aDelay = Analog("DELAY", 0, 5, 0, true, logMap, &iDelay);
    Analog aAttack = Analog("ATTACK", 0.001, 200, 5, true, logMap, &iAttack);
    Analog aDecay = Analog("DECAY", 0.005, 200, 5, true, logMap, &iDecay);
    Analog aSustain = Analog("SUSTAIN", 0, 1, 1, true, antilogMap, &iSustain);
    Analog aRelease = Analog("RELEASE", 0.001, 200, 5, true, logMap, &iRelease);
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
    vec<VOICESPERCHIP> sustain;
    ADSR_State currentState[VOICESPERCHIP] = {OFF};

    vec<VOICESPERCHIP> attackRate;
    vec<VOICESPERCHIP> decayRate;
    vec<VOICESPERCHIP> releaseRate;
    vec<VOICESPERCHIP> attackCoef;
    vec<VOICESPERCHIP> decayCoef;
    vec<VOICESPERCHIP> releaseCoef;
    vec<VOICESPERCHIP> attackBase;
    vec<VOICESPERCHIP> decayBase;
    vec<VOICESPERCHIP> releaseBase;
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

    Input iGlide = Input("GLIDE", "GLIDE");
    Input iDetune = Input("DETUNE", "DETUNE");

    Output oSpread = Output("SPREAD");

    Analog aGlide = Analog("GLIDE", 0.0001, 10, 0, true);
    Analog aDetune = Analog("DETUNE", 0, 1, 0, true);
    Analog aSpread = Analog("SPREAD", 0, 1, 0, true, linMap);
    Analog aImperfection = Analog("HUMANIZE", 0, 1, 0.5, true, linMap);

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
        knobs.push_back(&aADSR);
        knobs.push_back(&aPan);
        knobs.push_back(&aPanSpread);
        knobs.push_back(&aMaster);

        renderBuffer.push_back(&distort);
        renderBuffer.push_back(&left);
        renderBuffer.push_back(&right);

        moduleType = MODULE_OUT;
    }

    Input iDistort = Input("DRIVE", "DRIVE");
    Input iVCA = Input("VCA", "VCA");
    Input iPan = Input("PAN", "PAN");

    Analog aDistort = Analog("DRIVE", 0, 1, 0, true, linMap, &iDistort);
    Analog aVCA = Analog("VCA", 0, 1, 0, true, linMap, &iVCA);
    Analog aADSR = Analog("ADSR", -1, 1, 1, true, linMap);
    Analog aPan = Analog("PAN", -1, 1, 0, true, linMap, &iPan);
    Analog aPanSpread = Analog("PANSPREAD", 0, 1, 0, true);
    Analog aMaster = Analog("MASTER", 0, 1, 0.8, true, linMap);

    RenderBuffer left;
    RenderBuffer right;

    RenderBuffer distort;
};

class Waveshaper : public BaseModule {
  public:
    Waveshaper(const char *name, const char *shortName) : BaseModule(name, shortName) {

        inputs.push_back(&iShape1);
        inputs.push_back(&iShape2);
        inputs.push_back(&iShape3);
        inputs.push_back(&iShape4);
        inputs.push_back(&iPoint1X);
        inputs.push_back(&iPoint1Y);
        inputs.push_back(&iPoint2X);
        inputs.push_back(&iPoint2Y);
        inputs.push_back(&iPoint3X);
        inputs.push_back(&iPoint3Y);
        inputs.push_back(&iDryWet);

        knobs.push_back(&aShape1);
        knobs.push_back(&aShape2);
        knobs.push_back(&aShape3);
        knobs.push_back(&aShape4);
        knobs.push_back(&aPoint1X);
        knobs.push_back(&aPoint1Y);
        knobs.push_back(&aPoint2X);
        knobs.push_back(&aPoint2Y);
        knobs.push_back(&aPoint3X);
        knobs.push_back(&aPoint3Y);
        knobs.push_back(&aDryWet);

        renderBuffer.push_back(&shape1);
        renderBuffer.push_back(&shape2);
        renderBuffer.push_back(&shape3);
        renderBuffer.push_back(&shape4);
        renderBuffer.push_back(&Point1X);
        renderBuffer.push_back(&Point1Y);
        renderBuffer.push_back(&Point2X);
        renderBuffer.push_back(&Point2Y);
        renderBuffer.push_back(&Point3X);
        renderBuffer.push_back(&Point3Y);
        renderBuffer.push_back(&DryWet);
    }

    Input iShape1 = Input("Shape 1");
    Input iShape2 = Input("Shape 2");
    Input iShape3 = Input("Shape 3");
    Input iShape4 = Input("Shape 4");
    Input iPoint1X = Input("P1 X");
    Input iPoint1Y = Input("P1 Y");
    Input iPoint2X = Input("P2 X");
    Input iPoint2Y = Input("P2 Y");
    Input iPoint3X = Input("P3 X");
    Input iPoint3Y = Input("P3 Y");
    Input iDryWet = Input("Dry/Wet");

    Analog aShape1 = Analog("Shape 1", 0.000001f, 0.999999f, 0.5, true, linMap, &iShape1);
    Analog aShape2 = Analog("Shape 2", 0.000001f, 0.999999f, 0.5, true, linMap, &iShape2);
    Analog aShape3 = Analog("Shape 3", 0.000001f, 0.999999f, 0.5, true, linMap, &iShape3);
    Analog aShape4 = Analog("Shape 4", 0.000001f, 0.999999f, 0.5, true, linMap, &iShape4);
    Analog aPoint1X = Analog("P1 X", 0, 1, 0.25, true, linMap, &iPoint1X);
    Analog aPoint1Y = Analog("P1 Y", 0, 1, 0.25, true, linMap, &iPoint1Y);
    Analog aPoint2X = Analog("P2 X", 0, 1, 0.5, true, linMap, &iPoint2X);
    Analog aPoint2Y = Analog("P2 Y", 0, 1, 0.5, true, linMap, &iPoint2Y);
    Analog aPoint3X = Analog("P3 X", 0, 1, 0.75, true, linMap, &iPoint3X);
    Analog aPoint3Y = Analog("P3 Y", 0, 1, 0.75, true, linMap, &iPoint3Y);
    Analog aDryWet = Analog("Dry/Wet", 0, 1, 1, true, linMap, &iDryWet);

    RenderBuffer shape1;
    RenderBuffer shape2;
    RenderBuffer shape3;
    RenderBuffer shape4;
    RenderBuffer Point1X;
    RenderBuffer Point1Y;
    RenderBuffer Point2X;
    RenderBuffer Point2Y;
    RenderBuffer Point3X;
    RenderBuffer Point3Y;
    RenderBuffer DryWet;
};

class LayerSetting : public BaseModule {

  public:
    LayerSetting(const char *name, const char *shortName) : BaseModule(name, shortName) {
        switches.push_back(&dPitchbendRange);
    }

    Digital dPitchbendRange = Digital("PITCHBEND RANGE", 1, 24, 1, true, nullptr, nullptr);
};
