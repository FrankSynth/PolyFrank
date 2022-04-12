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

    inline virtual void resetPhase(uint16_t voice) {}

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
        outputs.push_back(&oVeloctiy);
        outputs.push_back(&oGate);
        outputs.push_back(&oNote);

        knobs.push_back(&aMod);
        knobs.push_back(&aAftertouch);
        knobs.push_back(&aPitchbend);
    }

    Output oMod = Output("MOD");
    Output oAftertouch = Output("AFTERTOUCH");
    Output oPitchbend = Output("PITCHBEND");
    Output oVeloctiy = Output("VELOCITY");
    Output oGate = Output("GATE");
    Output oNote = Output("NOTE");

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
        inputs.push_back(&iSquircle);

        knobs.push_back(&aMasterTune);
        knobs.push_back(&aMorph);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);
        knobs.push_back(&aSquircle);

        switches.push_back(&dSample0);
        switches.push_back(&dSample1);
        switches.push_back(&dSample2);
        switches.push_back(&dSample3);
        switches.push_back(&dOctave);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&samplecrusher);
        renderBuffer.push_back(&squircle);
    }

    Output out = Output("OUT");

    Input iFM = Input("FM", logMap);
    Input iMorph = Input("MORPH");
    Input iBitcrusher = Input("BITCRUSH");
    Input iOctave = Input("OCTAVE");
    Input iSamplecrusher = Input("SAMPLECRUSH");
    Input iSquircle = Input("SQUIRCLE");

    Analog aMasterTune = Analog("MASTERTUNE", -7, 7, 0, true, linMap, &iFM);
    Analog aMorph = Analog("MORPH", 0, WAVETABLESPERVOICE - 1, 0, true, linMap, &iMorph);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);
    Analog aSquircle = Analog("SQUIRCLE", 0, 1, 0.5, true, linMap, &iSamplecrusher);

    Digital dSample0 = Digital("WAVE 1", 0, WAVETABLESAMOUNT, 0, true, &nlWavetable);
    Digital dSample1 = Digital("WAVE 2", 0, WAVETABLESAMOUNT, 1, true, &nlWavetable);
    Digital dSample2 = Digital("WAVE 3", 0, WAVETABLESAMOUNT, 2, true, &nlWavetable);
    Digital dSample3 = Digital("WAVE 4", 0, WAVETABLESAMOUNT, 3, true, &nlWavetable);

    Digital dOctave = Digital("OCTAVE", -4, 4, 0, true, nullptr);

    RenderBuffer note;
    RenderBuffer morph;
    RenderBuffer bitcrusher;
    RenderBuffer samplecrusher;
    RenderBuffer squircle;

    bool newPhase[VOICESPERCHIP] = {false};
    vec<VOICESPERCHIP> phaseWavetableLower;
    vec<VOICESPERCHIP> phaseWavetableUpper;
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
        inputs.push_back(&iSquircle);

        knobs.push_back(&aMorph);
        knobs.push_back(&aTuning);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aSamplecrusher);
        knobs.push_back(&aPhaseoffset);
        knobs.push_back(&aSquircle);

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
        renderBuffer.push_back(&squircle);
    }

    Output out = Output("OUT");

    Input iFM = Input("FM", logMap);
    Input iMorph = Input("MORPH");
    Input iTuning = Input("TUNING");
    Input iBitcrusher = Input("BITCRUSH");
    Input iOctave = Input("OCTAVE");
    Input iSamplecrusher = Input("SAMPLECRUSH");
    Input iPhaseOffset = Input("PHASE OFFSET");
    Input iSquircle = Input("SQUIRCLE");

    Analog aMorph = Analog("MORPH", 0, WAVETABLESPERVOICE - 1, 0, true, linMap, &iMorph);
    Analog aTuning = Analog("TUNING", -7, 7, 0, true, linMap, &iTuning);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);
    Analog aPhaseoffset = Analog("PHASE OFFSET", -1, 1, 0, true, linMap, &iPhaseOffset);
    Analog aSquircle = Analog("SQUIRCLE", 0, 0.5, 1, true, linMap, &iSamplecrusher);

    Digital dOctave = Digital("OCT", -4, 4, 0, true, nullptr, &iOctave);
    Digital dSync = Digital("SYNC", 0, 1, 0, true, &nlOnOff);

    Digital dSample0 = Digital("WAVE 1", 0, WAVETABLESAMOUNT, 0, true, &nlWavetable);
    Digital dSample1 = Digital("WAVE 2", 0, WAVETABLESAMOUNT, 1, true, &nlWavetable);
    Digital dSample2 = Digital("WAVE 3", 0, WAVETABLESAMOUNT, 2, true, &nlWavetable);
    Digital dSample3 = Digital("WAVE 4", 0, WAVETABLESAMOUNT, 3, true, &nlWavetable);

    RenderBuffer note;
    RenderBuffer morph;
    RenderBuffer bitcrusher;
    RenderBuffer samplecrusher;
    RenderBuffer phaseoffset;
    RenderBuffer squircle;

    bool newPhase[VOICESPERCHIP] = {false};
    vec<VOICESPERCHIP> cacheOscAPhase;
    vec<VOICESPERCHIP> phaseWavetableLower;
    vec<VOICESPERCHIP> phaseWavetableUpper;
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
    }

    Output out = Output("OUT");

    Input iShape = Input("SHAPE");
    Input iBitcrusher = Input("BITCRUSH");
    Input iSamplecrusher = Input("SAMPLECRUSH");

    Analog aShape = Analog("SHAPE", 0.01f, 1, 0.01f, true, linMap, &iShape);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 1, 0, true, linMap, &iBitcrusher);
    Analog aSamplecrusher = Analog("SAMPLECRUSH", 0, 960, 0, true, logMap, &iSamplecrusher);

    Digital dOctaveSwitch = Digital("OscA", 0, 1, 0, true, &nlSubOctaves);

    RenderBuffer shape;
    RenderBuffer bitcrusher;
    RenderBuffer samplecrusher;

    vec<VOICESPERCHIP> phase;
    vec<VOICESPERCHIP> oscApreviousPhase;
};

// TODO bitrusher
class Noise : public BaseModule {
  public:
    Noise(const char *name, const char *shortName) : BaseModule(name, shortName) {
        outputs.push_back(&out);

        inputs.push_back(&iSamplecrusher);

        knobs.push_back(&aSamplecrusher);

        renderBuffer.push_back(&samplecrusher);
    }

    Output out = Output("OUT");

    Input iSamplecrusher = Input("SAMPLECRUSH");

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
    }

    Input iOSCALevel = Input("OSC A");
    Input iOSCBLevel = Input("OSC B");
    Input iSUBLevel = Input("SUB");
    Input iNOISELevel = Input("NOISE");

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
};

class Steiner : public BaseModule {

  public:
    Steiner(const char *name, const char *shortName) : BaseModule(name, shortName) {
        inputs.push_back(&iCutoff);
        inputs.push_back(&iResonance);
        inputs.push_back(&iLevel);

        knobs.push_back(&aCutoff);
        knobs.push_back(&aResonance);
        knobs.push_back(&aADSR);
        knobs.push_back(&aLevel);
        knobs.push_back(&aParSer);

        switches.push_back(&dMode);

        renderBuffer.push_back(&resonance);
        renderBuffer.push_back(&level);
        renderBuffer.push_back(&cutoff);
        renderBuffer.push_back(&toLadder);
    }
    Input iCutoff = Input("CUTOFF");
    Input iResonance = Input("RESONANCE");
    Input iLevel = Input("LEVEL");

    Analog aCutoff = Analog("CUTOFF", 0, 1, 1, true, linMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, linMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, linMap, &iLevel);
    Analog aParSer = Analog("PAR/SER", 0, 1, 0, true, linMap);
    Analog aADSR = Analog("ADSR", -1, 1, 0, true, linMap);

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
        knobs.push_back(&aADSR);
        knobs.push_back(&aLevel);

        switches.push_back(&dSlope);

        renderBuffer.push_back(&resonance);
        renderBuffer.push_back(&level);
        renderBuffer.push_back(&cutoff);
    }

    Input iCutoff = Input("CUTOFF");
    Input iResonance = Input("RESONANCE");
    Input iLevel = Input("LEVEL");

    Analog aCutoff = Analog("CUTOFF", 0, 1, 1, true, linMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, linMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, linMap, &iLevel);
    Analog aADSR = Analog("ADSR", -1, 1, 0, true, linMap);

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

        // switches.push_back(&dFreq);
        switches.push_back(&dFreqSnap);
        switches.push_back(&dGateSync);
        switches.push_back(&dClockSync);
        switches.push_back(&dClockStep);
        switches.push_back(&dAlignLFOs);
    }
    Output out = Output("OUT");

    Input iFreq = Input("FM", logMap);
    Input iShape = Input("SHAPE");
    Input iAmount = Input("AMOUNT");

    Analog aFreq = Analog("FREQ", 0, 1, 0.6, true, linMap, &iFreq);
    Analog aShape = Analog("SHAPE", 0, 6, 0, true, linMap, &iShape);
    Analog aAmount = Analog("AMOUNT", 0, 1, 1, true, linMap, &iAmount);

    // TODO Freq also as Digital knob??
    // Digital dFreq = Digital("FREQ", 0, 22, 0, true);
    Digital dFreqSnap = Digital("SNAP", 0, 1, 0, false, &nlOnOff);
    Digital dGateSync = Digital("SYNC G", 0, 1, 0, true, &nlOnOff);
    Digital dClockSync = Digital("SYNC C", 0, 1, 0, false, &nlOnOff);
    Digital dClockStep = Digital("CLOCK", 0, 22, 0, false, &nlClockSteps);
    Digital dAlignLFOs = Digital("ALIGN", 0, 1, 0, true, &nlOnOff);

    vec<VOICESPERCHIP> currentTime;
    bool newPhase[VOICESPERCHIP] = {false};
    vec<VOICESPERCHIP> currentRandom;

    bool alignedRandom = false;
    uint32_t randSeed = 1;

    inline void resetPhase(uint16_t voice) {
        currentTime[voice] = 1;
        newPhase[voice] = true;
    }

    inline void resetAllPhases() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetPhase(voice);
        }
    }

    inline void gateOn(uint16_t voice) {
        if (dGateSync.valueMapped) {
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
    }

    Output out = Output("OUT");

    Input iDelay = Input("DELAY");
    Input iAttack = Input("ATTACK");
    Input iDecay = Input("DECAY");
    Input iSustain = Input("SUSTAIN");
    Input iRelease = Input("RELEASE");
    Input iAmount = Input("AMOUNT");

    Analog aDelay = Analog("DELAY", 0, 10, 0, true, logMap, &iDelay);
    Analog aAttack = Analog("ATTACK", 0.001, 10, 0.1, true, logMap, &iAttack);
    Analog aDecay = Analog("DECAY", 0.001, 10, 0.1, true, logMap, &iDecay);
    Analog aSustain = Analog("SUSTAIN", 0, 1, 0.5, true, linMap, &iSustain);
    Analog aRelease = Analog("RELEASE", 0.001, 10, 0.1, true, logMap, &iRelease);
    Analog aAmount = Analog("AMOUNT", 0, 1, 1, true, linMap, &iAmount);

    Analog aKeytrack = Analog("KEYTRACK", 0, 1, 0, true, linMap);
    Analog aVelocity = Analog("VELOCITY", 0, 1, 0, true, linMap);
    Analog aShape = Analog("SHAPE", 0, 1, 0, true, linMap);

    Digital dLoop = Digital("LOOP", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dLatch = Digital("LATCH", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dReset = Digital("RESET", 0, 1, 0, true, &nlOnOff, nullptr);

    inline void resetPhase(uint16_t voice) { resetADSR(voice); }

    inline void resetAllPhases() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetPhase(voice);
        }
    }
    inline void resetADSR(uint16_t voice) {
        level[voice] = 0;
        currentTime[voice] = 0;
        currentState[voice] = OFF;
    }

    inline void resetAllADSRs() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetADSR(voice);
        }
    }

    inline void gateOn(uint16_t voice) {
        if (dReset.valueMapped)
            resetADSR(voice);
    }

    enum ADSR_State { OFF, DELAY, ATTACK, DECAY, SUSTAIN, RELEASE };

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
    }

    Input iGlide = Input("GLIDE");
    Input iDetune = Input("DETUNE");

    Output oSpread = Output("SPREAD");

    Analog aGlide = Analog("GLIDE", 0.0001, 10, 0, true, logMap);
    Analog aDetune = Analog("DETUNE", 0, 1, 0, true, logMap);
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
    }

    Input iDistort = Input("DRIVE");
    Input iVCA = Input("VCA", logMap);
    Input iPan = Input("PAN");

    Analog aDistort = Analog("DRIVE", 0, 1, 0, true, linMap, &iDistort);
    Analog aVCA = Analog("VCA", 0, 1, 0, true, linMap, &iVCA);
    Analog aADSR = Analog("ADSR", -1, 1, 1, true, linMap);
    Analog aPan = Analog("PAN", -1, 1, 0, true, linMap, &iPan);
    Analog aPanSpread = Analog("PANSPREAD", 0, 1, 0, true, logMap);
    Analog aMaster = Analog("MASTER", 0, 1, 0.8, true, linMap);

    RenderBuffer left;
    RenderBuffer right;

    RenderBuffer distort;
};

class LayerSetting : public BaseModule {

  public:
    LayerSetting(const char *name, const char *shortName) : BaseModule(name, shortName) {
        switches.push_back(&dPitchbendRange);
    }

    Digital dPitchbendRange = Digital("PITCHBEND RANGE", 1, 24, 1, true, nullptr, nullptr);
};
