#pragma once

#include "datacore/datacore.hpp"
#include <string>
#include <vector>

class BaseModule {
  public:
    BaseModule(const char *name) { this->name = name; }

    const std::string &getName() { return name; }
    std::vector<Output *> &getOutputs() { return outputs; }
    std::vector<Input *> &getInputs() { return inputs; }
    std::vector<Analog *> &getPotis() { return knobs; }
    std::vector<Digital *> &getSwitches() { return switches; }
    std::vector<Setting *> &getSettings() { return settings; }

    uint8_t id;
    uint8_t layerId;

  protected:
    std::string name;
    std::vector<Output *> outputs;
    std::vector<Input *> inputs;
    std::vector<Analog *> knobs;
    std::vector<Digital *> switches;
    std::vector<Setting *> settings;
};

//////////////////////////////// MODULES ///////////////////////////////////////////////

class ADSR : public BaseModule {
  public:
    ADSR(const char *name) : BaseModule(name) { // call subclass

        outputs.push_back(&out);

        inputs.push_back(&iDelay);
        inputs.push_back(&iAttack);
        inputs.push_back(&iDecay);
        inputs.push_back(&iSustain);
        inputs.push_back(&iRelease);

        knobs.push_back(&aDelay);
        knobs.push_back(&aAttack);
        knobs.push_back(&aDecay);
        knobs.push_back(&aSustain);
        knobs.push_back(&aRelease);

        knobs.push_back(&aKeytrack);
        knobs.push_back(&aVelocity);
        knobs.push_back(&aShape);

        switches.push_back(&dLoop);
    }

    Output out = Output("OUT");

    Input iDelay = Input("DELAY");
    Input iAttack = Input("ATTACK");
    Input iDecay = Input("DECAY");
    Input iSustain = Input("SUSTAIN");
    Input iRelease = Input("RELEASE");

    Analog aDelay = Analog("DELAY", 0.001, 10, true, logMap);
    Analog aAttack = Analog("ATTACK", 0.001, 10, true, logMap);
    Analog aDecay = Analog("DECAY", 0.001, 10, true, logMap);
    Analog aSustain = Analog("SUSTAIN", 0.001, 10, true, logMap);
    Analog aRelease = Analog("RELEASE", 0.001, 10, true, logMap);

    Analog aKeytrack = Analog("KEYTRACK", 0, 1, true, linMap);
    Analog aVelocity = Analog("VELOCITY", 0, 1, true, linMap);
    Analog aShape = Analog("SHAPE", 0, 1, true, linMap);

    std::vector<std::string> valueNames{"OFF", "ON"};

    Digital dLoop = Digital("LOOP", 0, 1, true, &valueNames);

    void render();
};

class OSC_A : public BaseModule {
  public:
    OSC_A(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iFm);
        inputs.push_back(&iPwm);
        inputs.push_back(&iMorph);
        inputs.push_back(&iGlide);
        inputs.push_back(&iLevel);
        inputs.push_back(&iSpread);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iOctave);

        knobs.push_back(&aPwm);
        knobs.push_back(&aMorph);
        knobs.push_back(&aGlide);
        knobs.push_back(&aLevel);
        knobs.push_back(&aSpread);
        knobs.push_back(&aBitcrusher);
        knobs.push_back(&aDetune);

        switches.push_back(&dNote);
        switches.push_back(&dOctave);
        switches.push_back(&dVcfSwitch);
    }

    Output out = Output("OUT");

    Input iFm = Input("FM");
    Input iPwm = Input("PWM");
    Input iMorph = Input("MORPH");
    Input iGlide = Input("GLIDE");
    Input iLevel = Input("LEVEL");
    Input iSpread = Input("SPREAD");
    Input iBitcrusher = Input("BITCRUSHER");
    Input iOctave = Input("OCTAVE");

    Analog aPwm = Analog("PWM", 0, 1, true, linMap);
    Analog aMorph = Analog("MORPH", 0, 1, true, linMap);
    Analog aGlide = Analog("GLIDE", 0, 10, true, logMap);
    Analog aLevel = Analog("LEVEL", 0, 1, true, logMap);
    Analog aSpread = Analog("SPREAD", 0, 1, true, logMap);
    Analog aBitcrusher = Analog("BITCRUSHER", 0, 24, true, linMap);
    Analog aDetune = Analog("DETUNE", 0, 1, true, logMap);

    Digital dNote = Digital("NOTE", 22, 108, false);
    Digital dOctave = Digital("OCTAVE", -4, 4, true);
    Digital dVcfSwitch = Digital("VCF OUT", 0, 2, true);
};

class OSC_B : public BaseModule {
  public:
    OSC_B(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iFm);
        inputs.push_back(&iPwm);
        inputs.push_back(&iMorph);
        inputs.push_back(&iTuning);
        inputs.push_back(&iLevel);
        inputs.push_back(&iBitcrusher);
        inputs.push_back(&iOctave);
        inputs.push_back(&iSync);

        knobs.push_back(&aPwm);
        knobs.push_back(&aMorph);
        knobs.push_back(&aTuning);
        knobs.push_back(&aLevel);
        knobs.push_back(&aBitcrusher);

        switches.push_back(&dNote);
        switches.push_back(&dOctave);
        switches.push_back(&dVcfSwitch);
    }

    Output out = Output("OUT");

    Input iFm = Input("FM");
    Input iPwm = Input("PWM");
    Input iMorph = Input("MORPH");
    Input iTuning = Input("TUNING");
    Input iLevel = Input("LEVEL");
    Input iBitcrusher = Input("BITCRUSHER");
    Input iOctave = Input("OCTAVE");
    Input iSync = Input("OCTAVE");

    Analog aPwm = Analog("PWM", 0, 1, true, linMap);
    Analog aMorph = Analog("MORPH", 0, 1, true, linMap);
    Analog aTuning = Analog("TUNING", 0, 10, true, logMap);
    Analog aLevel = Analog("LEVEL", 0, 1, true, logMap);
    Analog aBitcrusher = Analog("BITCRUSHER", 0, 24, true, linMap);

    Digital dNote = Digital("NOTE", 22, 108, false);
    Digital dOctave = Digital("OCTAVE", -4, 4, true);
    Digital dVcfSwitch = Digital("VCF OUT", 0, 2, true);
};

class LFO : public BaseModule {
  public:
    LFO(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        inputs.push_back(&iFm);
        inputs.push_back(&iSync);

        knobs.push_back(&aFreq);
        knobs.push_back(&aShape);

        switches.push_back(&dFreq);
        switches.push_back(&dSync);
        switches.push_back(&dTempoSync);
        switches.push_back(&dLockPhase);
    }
    Output out = Output("OUT");
    Input iFm = Input("FM");
    Input iSync = Input("SYNC");
    Analog aFreq = Analog("FREQ", 0.1, 100, true, logMap);
    Analog aShape = Analog("SHAPE", 0.1, 100, true, linMap);
    Digital dFreq = Digital("FREQ", 0, 22, true);
    Digital dSync = Digital("SYNC", 0, 1, true);
    Digital dTempoSync = Digital("T-SYNC", 0, 1, false);
    Digital dLockPhase = Digital("PHASE", 0, 1, false);
};

class TEST : public BaseModule {
  public:
    TEST(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&out);

        knobs.push_back(&aCutoff);
        knobs.push_back(&aResonance);
        knobs.push_back(&aDistort);
        knobs.push_back(&aFreq);

        switches.push_back(&dSelectFilter);
    }
    Output out = Output("OUT");

    Analog aCutoff = Analog("CUTOFF", 0, 1, true, logMap);
    Analog aResonance = Analog("RES", 0, 1, true, linMap);
    Analog aDistort = Analog("DIST", 0, 1, true, linMap);
    Analog aFreq = Analog("FREQ", 0, 1, true, linMap);

    Digital dSelectFilter = Digital("dB", 0, 3, true);
};

// class VCFSteiner : public BaseModule {
//   public:
//     VCFSteiner(const char *name) : BaseModule(name) { // call subclass

//         inputs.push_back(Input("CUTOFF"));
//         inputs.push_back(Input("RESONANCE"));
//         inputs.push_back(Input("LEVEL"));
//         inputs.push_back(Input("MODE"));

//         knobs.push_back(Analog("CUTOFF", 10, 20000, true, logMap));
//         knobs.push_back(Analog("RESONANCE", 0, 1, true, linMap));
//         knobs.push_back(Analog("LEVEL", 0, 1, true, logMap));
//         knobs.push_back(Analog("ENV AMT", 0, 1, true, logMap));

//         switches.push_back(Digital("MODE", 0, 3, true));
//     }
// };

// class VCFLadder : public BaseModule {
//   public:
//     VCFLadder(const char *name) : BaseModule(name) { // call subclass

//         inputs.push_back(Input("CUTOFF"));
//         inputs.push_back(Input("RESONANCE"));
//         inputs.push_back(Input("LEVEL"));
//         inputs.push_back(Input("SLOPE"));
//         inputs.push_back(Input("SER/PAR"));

//         knobs.push_back(Analog("CUTOFF", 10, 20000, true, logMap));
//         knobs.push_back(Analog("RESONANCE", 0, 1, true, linMap));
//         knobs.push_back(Analog("LEVEL", 0, 1, true, logMap));
//         knobs.push_back(Analog("ENV AMT", 0, 1, true, logMap));
//         knobs.push_back(Analog("SER/PAR", 0, 1, true, linMap));

//         switches.push_back(Digital("SLOPE", 0, 3, true));
//     }
// };

// class Distortion : public BaseModule {
//   public:
//     Distortion(const char *name) : BaseModule(name) { // call subclass

//         inputs.push_back(Input("AMOUNT"));

//         knobs.push_back(Analog("AMOUNT", 0, 1, true, logMap));
//     }
// };

// class Noise : public BaseModule {
//   public:
//     Noise(const char *name) : BaseModule(name) { // call subclass

//         inputs.push_back(Input("COLOR"));
//         inputs.push_back(Input("LEVEL"));

//         knobs.push_back(Analog("COLOR", 0, 1, true, logMap));
//         knobs.push_back(Analog("LEVEL", 0, 1, true, logMap));
//     }
// };

class Sub : public BaseModule {
  public:
    Sub(const char *name) : BaseModule(name) { // call subclass

        inputs.push_back(&iShape);
        inputs.push_back(&iLevel);

        knobs.push_back(&aShape);
        knobs.push_back(&aLevel);
    }

    Input iShape = Input("SHAPE");
    Input iLevel = Input("LEVEL");

    Analog aShape = Analog("SHAPE", 0, 1, true, logMap);
    Analog aLevel = Analog("LEVEL", 0, 1, true, logMap);
};

class Midi : public BaseModule {
  public:
    Midi(const char *name) : BaseModule(name) { // call subclass
        outputs.push_back(&oMod);
        outputs.push_back(&oAftertouch);
        outputs.push_back(&oPitchbend);
        outputs.push_back(&oVeloctiy);
        outputs.push_back(&oGate);

        knobs.push_back(&aMod);
        knobs.push_back(&aAftertouch);
        knobs.push_back(&aPitchbend);
        knobs.push_back(&aVelocity);
    }

    Output oMod = Output("MOD");
    Output oAftertouch = Output("AFTERTOUCH");
    Output oPitchbend = Output("PITCHBEND");
    Output oVeloctiy = Output("VELOCITY");
    Output oGate = Output("GATE");

    Analog aMod = Analog("MOD", 0, 1, true, linMap);
    Analog aAftertouch = Analog("AFTERTOUCH", 0, 1, true, linMap);
    Analog aPitchbend = Analog("PITCHBEND", -1, 1, true, linMap);
    Analog aVelocity = Analog("VELOCITY", 0, 1, true, linMap);

    void render();
};