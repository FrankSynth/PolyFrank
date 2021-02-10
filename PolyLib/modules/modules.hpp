#pragma once

#include "datacore/datacore.hpp"
#include <string>
#include <vector>

// NameLists for switches
extern const std::vector<std::string> nlOnOff;
extern const std::vector<std::string> nlVCFDest;
extern const std::vector<std::string> nlSteinerModes;
extern const std::vector<std::string> nlLadderSlopes;
extern const std::vector<std::string> nlADSRShapes;
extern const std::vector<std::string> nlClockSteps;

// Basemodule
class BaseModule {
  public:
    BaseModule(const char *name) { this->name = name; }

    inline const std::string &getName() { return name; }
    inline std::vector<Output *> &getOutputs() { return outputs; }
    inline std::vector<Input *> &getInputs() { return inputs; }
    inline std::vector<Analog *> &getPotis() { return knobs; }
    inline std::vector<Digital *> &getSwitches() { return switches; }
    inline std::vector<Setting *> &getSettings() { return settings; }

    uint8_t id;
    uint8_t layerId;

    std::string name;
    std::vector<Output *> outputs;
    std::vector<Input *> inputs;
    std::vector<Analog *> knobs;
    std::vector<Digital *> switches;
    std::vector<Setting *> settings;

    inline virtual void resetPhase(uint16_t voice) {}

  protected:
};

//////////////////////////////// MODULES ///////////////////////////////////////////////

class Midi : public BaseModule {
  public:
    Midi(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&oMod);
        outputs.push_back(&oAftertouch);
        outputs.push_back(&oPitchbend);
        outputs.push_back(&oVeloctiy);
        outputs.push_back(&oGate);
        outputs.push_back(&oNote);

        knobs.push_back(&aMod);
        knobs.push_back(&aAftertouch);
        knobs.push_back(&aPitchbend);

        // switches.push_back(&dGate);
    }

    Output oMod = Output("MOD");
    Output oAftertouch = Output("AFTERTOUCH");
    Output oPitchbend = Output("PITCHBEND");
    Output oVeloctiy = Output("VELOCITY");
    Output oGate = Output("GATE");
    Output oNote = Output("NOTE");

    Analog aMod = Analog("MOD", 0, 1, 0, true, linMap);
    Analog aAftertouch = Analog("AFTERTOUCH", 0, 1, 0, true, linMap);
    Analog aPitchbend = Analog("PITCHBEND", -1, 1, 0, true, linMap);

    // Digital dGate = Digital("GATE", 0, 1, 0, true);

    uint8_t rawNote[VOICESPERCHIP];
    uint8_t rawVelocity[VOICESPERCHIP];
    uint8_t rawGate[VOICESPERCHIP];

    inline void gateOn(uint16_t voice) { rawGate[voice] = 1; }
    inline void gateOff(uint16_t voice) { rawGate[voice] = 0; }
    inline void setNote(uint16_t voice, uint8_t note, uint8_t velocity) {
        rawNote[voice] = note;
        rawVelocity[voice] = velocity;
    }
    inline void resetAllNotes() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            setNote(voice, 21, 64);
        }
    }
};

class OSC_A : public BaseModule {
  public:
    OSC_A(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iFM);
        inputs.push_back(&iMorph);
        inputs.push_back(&iLevel);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iOctave);

        knobs.push_back(&aMasterTune);
        knobs.push_back(&aMorph);
        knobs.push_back(&aLevel);
        knobs.push_back(&aBitcrusher);

        switches.push_back(&dOctave);
        switches.push_back(&dVcfDestSwitch);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&levelSteiner);
        renderBuffer.push_back(&levelLadder);
    }

    Output out = Output("OUT");

    Input iFM = Input("FM");
    Input iMorph = Input("MORPH");
    Input iLevel = Input("LEVEL");
    Input iBitcrusher = Input("BITCRUSH");
    Input iOctave = Input("OCTAVE");

    Analog aMasterTune = Analog("MASTERTUNE", -1, 1, 0, true, logMap);
    Analog aMorph = Analog("MORPH", 0, 1, 0, true, linMap, &iMorph);
    Analog aLevel = Analog("LEVEL", 0, 1, 0.5, true, logMap, &iLevel);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 23, 0, true, linMap, &iBitcrusher);

    Digital dOctave = Digital("OCTAVE", -4, 4, 0, true, nullptr, &iOctave);
    Digital dVcfDestSwitch = Digital("VCF OUT", 0, 3, 0, true, &nlVCFDest);

    // render shizzle

    std::vector<RenderBuffer *> renderBuffer;

    RenderBuffer note;
    RenderBuffer morph;
    RenderBuffer bitcrusher;
    RenderBuffer levelSteiner;
    RenderBuffer levelLadder;

    bool newPhase[VOICESPERCHIP] = {false};
    float stepWavetableA[VOICESPERCHIP] = {0};
    float stepWavetableB[VOICESPERCHIP] = {0};

    inline void gateOn(uint16_t voice) {
        // TODO gate on OSC A
    }
};

class OSC_B : public BaseModule {
  public:
    OSC_B(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iFM);
        inputs.push_back(&iMorph);
        inputs.push_back(&iTuning);
        inputs.push_back(&iLevel);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iOctave);

        knobs.push_back(&aMorph);
        knobs.push_back(&aTuning);
        knobs.push_back(&aLevel);
        knobs.push_back(&aBitcrusher);

        switches.push_back(&dOctave);
        switches.push_back(&dVcfDestSwitch);
        switches.push_back(&dSync);

        renderBuffer.push_back(&note);
        renderBuffer.push_back(&morph);
        renderBuffer.push_back(&bitcrusher);
        renderBuffer.push_back(&levelSteiner);
        renderBuffer.push_back(&levelLadder);
    }

    Output out = Output("OUT");

    Input iFM = Input("FM");
    Input iMorph = Input("MORPH");
    Input iTuning = Input("TUNING");
    Input iLevel = Input("LEVEL");
    Input iBitcrusher = Input("BITCRUSH");
    Input iOctave = Input("OCTAVE");
    // Input iSync = Input("SYNC");

    Analog aMorph = Analog("MORPH", 0, 1, 0, true, linMap, &iMorph);
    Analog aTuning = Analog("TUNING", -1, 1, 0, true, logMap, &iTuning);
    Analog aLevel = Analog("LEVEL", 0, 1, 0.5, true, logMap, &iLevel);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 23, 0, true, linMap, &iBitcrusher);

    // Digital dNote = Digital("NOTE", 22, 108, 22, false, nullptr, nullptr, false);
    Digital dOctave = Digital("OCTAVE", -4, 4, 0, true, nullptr, &iOctave);
    Digital dVcfDestSwitch = Digital("VCF DEST", 0, 3, 0, true, &nlVCFDest);
    Digital dSync = Digital("SYNC OSC A", 0, 1, 0, true, &nlOnOff);

    // render shizzle

    std::vector<RenderBuffer *> renderBuffer;

    RenderBuffer note;
    RenderBuffer morph;
    RenderBuffer bitcrusher;
    RenderBuffer levelSteiner;
    RenderBuffer levelLadder;

    bool newPhase[VOICESPERCHIP] = {false};
    float cacheOscAstep[VOICESPERCHIP] = {false};
    float stepWavetableA[VOICESPERCHIP] = {0};
    float stepWavetableB[VOICESPERCHIP] = {0};

    inline void gateOn(uint16_t voice) {
        // TODO gate on OSC B
    }
};

class Sub : public BaseModule {
  public:
    Sub(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iShape);
        inputs.push_back(&iLevel);
        inputs.push_back(&iBitcrusher);

        knobs.push_back(&aShape);
        knobs.push_back(&aLevel);
        knobs.push_back(&aBitcrusher);

        switches.push_back(&dVcfDestSwitch);

        renderBuffer.push_back(&shape);
        renderBuffer.push_back(&levelSteiner);
        renderBuffer.push_back(&levelLadder);
        renderBuffer.push_back(&bitcrusher);
    }

    Output out = Output("OUT");

    Input iShape = Input("SHAPE");
    Input iLevel = Input("LEVEL");
    Input iBitcrusher = Input("BITCRUSH");

    Analog aShape = Analog("SHAPE", 0, 1, 0, true, logMap, &iShape);
    Analog aLevel = Analog("LEVEL", 0, 1, 0, true, logMap, &iLevel);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 23, 0, true, linMap, &iBitcrusher);

    Digital dVcfDestSwitch = Digital("VCF Dest", 0, 3, 0, true, &nlVCFDest);

    // render shizzle

    std::vector<RenderBuffer *> renderBuffer;

    RenderBuffer shape;
    RenderBuffer levelSteiner;
    RenderBuffer levelLadder;
    RenderBuffer bitcrusher;
};

class Noise : public BaseModule {
  public:
    Noise(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iLevel);
        inputs.push_back(&iBitcrusher);

        knobs.push_back(&aLevel);
        knobs.push_back(&aBitcrusher);

        switches.push_back(&dVcfDestSwitch);

        renderBuffer.push_back(&levelSteiner);
        renderBuffer.push_back(&levelLadder);
        renderBuffer.push_back(&bitcrusher);
    }

    Output out = Output("OUT");

    Input iLevel = Input("LEVEL");
    Input iBitcrusher = Input("BITCRUSH");

    Analog aLevel = Analog("LEVEL", 0, 1, 0, true, logMap, &iLevel);
    Analog aBitcrusher = Analog("BITCRUSH", 0, 23, 0, true, linMap, &iBitcrusher);

    Digital dVcfDestSwitch = Digital("VCF Dest", 0, 3, 0, true, &nlVCFDest);

    // render shizzle

    std::vector<RenderBuffer *> renderBuffer;

    RenderBuffer levelSteiner;
    RenderBuffer levelLadder;
    RenderBuffer bitcrusher;
};

class Steiner : public BaseModule {
  public:
    Steiner(const char *name) : BaseModule(name) { // call subclass
        inputs.push_back(&iCutoff);
        inputs.push_back(&iResonance);
        inputs.push_back(&iLevel);

        knobs.push_back(&aCutoff);
        knobs.push_back(&aResonance);
        knobs.push_back(&aLevel);

        switches.push_back(&dMode);
    }
    Input iCutoff = Input("CUTOFF");
    Input iResonance = Input("RESONANCE");
    Input iLevel = Input("LEVEL");

    Analog aCutoff = Analog("CUTOFF", 0, 20000, 20000, true, logMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, logMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, logMap, &iLevel);

    Digital dMode = Digital("MODE", 0, 3, 0, false, &nlSteinerModes);
};

class Ladder : public BaseModule {
  public:
    Ladder(const char *name) : BaseModule(name) { // call subclass
        inputs.push_back(&iCutoff);
        inputs.push_back(&iResonance);
        inputs.push_back(&iLevel);

        knobs.push_back(&aCutoff);
        knobs.push_back(&aResonance);
        knobs.push_back(&aLevel);
        knobs.push_back(&aParSer);

        switches.push_back(&dSlope);
    }

    Input iCutoff = Input("CUTOFF");
    Input iResonance = Input("RESONANCE");
    Input iLevel = Input("LEVEL");

    Analog aCutoff = Analog("CUTOFF", 0, 20000, 20000, true, logMap, &iCutoff);
    Analog aResonance = Analog("RESONANCE", 0, 1, 0, true, logMap, &iResonance);
    Analog aLevel = Analog("LEVEL", 0, 1, 1, true, logMap, &iLevel);
    Analog aParSer = Analog("PAR/SER", 0, 1, 1, true, linMap);

    Digital dSlope = Digital("SLOPE", 0, 3, 0, false, &nlLadderSlopes);
};

class Distortion : public BaseModule {
  public:
    Distortion(const char *name) : BaseModule(name) { // call subclass

        inputs.push_back(&iDrive);

        knobs.push_back(&aDrive);
    }

    Input iDrive = Input("DRIVE");

    Analog aDrive = Analog("DRIVE", 0, 1, 0, true, logMap, &iDrive);
};

class LFO : public BaseModule {
  public:
    LFO(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iFreq);

        knobs.push_back(&aFreq);
        knobs.push_back(&aShape);

        // switches.push_back(&dFreq);
        switches.push_back(&dFreqSnap);
        switches.push_back(&dGateSync);
        switches.push_back(&dClockSync);
        switches.push_back(&dClockStep);
        switches.push_back(&dAlignLFOs);
    }
    Output out = Output("OUT");

    Input iFreq = Input("FM");

    Analog aFreq = Analog("FREQ", 0.1, 100, 1, true, logMap, &iFreq);

    Analog aShape = Analog("SHAPE", 0, 6, 0, true, linMap);

    // Freq also as Digital knob??
    // Digital dFreq = Digital("FREQ", 0, 22, 0, true);
    Digital dFreqSnap = Digital("SNAP SPEED", 0, 1, 0, false, &nlOnOff);
    Digital dGateSync = Digital("SYNC GATE", 0, 1, 0, true, &nlOnOff);
    Digital dClockSync = Digital("SYNC CLOCK", 0, 1, 0, false, &nlOnOff);
    Digital dClockStep = Digital("CLOCK", 0, 22, 0, false, &nlClockSteps);
    Digital dAlignLFOs = Digital("ALIGN", 0, 1, 0, false, &nlOnOff);

    // render shizzle

    float currentTime[VOICESPERCHIP] = {0};
    bool newPhase[VOICESPERCHIP] = {false};

    inline void resetPhase(uint16_t voice) {
        if (voice == 4) {
            resetAllPhases();
        }
        else {
            currentTime[voice] = 0;
            newPhase[voice] = true;
        }
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
    ADSR(const char *name) : BaseModule(name) { // call subclass

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
    Analog aAttack = Analog("ATTACK", 0.001, 10, 0.5, true, logMap, &iAttack);
    Analog aDecay = Analog("DECAY", 0.001, 10, 0.5, true, logMap, &iDecay);
    Analog aSustain = Analog("SUSTAIN", 0, 1, 1, true, logMap, &iSustain);
    Analog aRelease = Analog("RELEASE", 0.001, 10, 0.5, true, logMap, &iRelease);
    Analog aAmount = Analog("AMOUNT", 0, 1, 1, true, logMap, &iAmount);

    Analog aKeytrack = Analog("KEYTRACK", 0, 1, 0, true, linMap);
    Analog aVelocity = Analog("VELOCITY", 0, 1, 0, true, linMap);
    Analog aShape = Analog("SHAPE", 0, 2, 1, true, linMap);

    Digital dLoop = Digital("LOOP", 0, 1, 1, true, &nlOnOff, nullptr);
    Digital dLatch = Digital("LATCH", 0, 1, 0, true, &nlOnOff, nullptr);
    Digital dReset = Digital("RESET", 0, 1, 0, true, &nlOnOff, nullptr);

    // render shizzle

    inline void setStatusOff(uint16_t voice) { currentState[voice] = OFF; }
    inline void setStatusDelay(uint16_t voice) {
        currentState[voice] = DELAY;
        currentTime[voice] = 0;
    }
    inline void setStatusAttack(uint16_t voice) { currentState[voice] = ATTACK; }
    inline void setStatusDecay(uint16_t voice) { currentState[voice] = DECAY; }
    inline void setStatusSustain(uint16_t voice) { currentState[voice] = SUSTAIN; }
    inline void setStatusRelease(uint16_t voice) { currentState[voice] = RELEASE; }

    inline void restartADSR(uint16_t voice) {
        currentLevel[voice] = 0;
        setStatusDelay(voice);
    }

    inline void resetPhase(uint16_t voice) {
        if (voice == 4) {
            resetAllPhases();
        }
        else {
            resetADSR(voice);
        }
    }

    inline void resetAllPhases() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            resetPhase(voice);
        }
    }
    inline void resetADSR(uint16_t voice) {
        currentLevel[voice] = 0;
        setStatusOff(voice);
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
    inline ADSR_State getState(uint16_t voice) { return currentState[voice]; }

    float currentLevel[VOICESPERCHIP] = {0};
    float currentTime[VOICESPERCHIP] = {0};

  private:
    ADSR_State currentState[VOICESPERCHIP] = {OFF};
};

class GlobalModule : public BaseModule {
  public:
    GlobalModule(const char *name) : BaseModule(name) { // call subclass

        inputs.push_back(&iVCA);
        inputs.push_back(&iPan);

        knobs.push_back(&aVCA);
        knobs.push_back(&aGlide);
        knobs.push_back(&aPan);
        knobs.push_back(&aSpread);
        knobs.push_back(&aDetune);
    }

    Input iVCA = Input("VCA");
    Input iPan = Input("LEVEL");

    Analog aVCA = Analog("VCA", 0, 1, 0, true, logMap, &iVCA);
    Analog aGlide = Analog("GLIDE", 0, 1, 0, true, logMap);
    Analog aPan = Analog("PAN", 0, 1, 0, true, logMap, &iPan);
    Analog aSpread = Analog("SPREAD", 0, 1, 0, true, logMap);
    Analog aDetune = Analog("DETUNE", 0, 1, 0, true, logMap);
};
