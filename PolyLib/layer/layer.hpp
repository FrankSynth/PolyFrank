#pragma once

#include "datacore/datacore.hpp"
#include "modules/modules.hpp"
#include "preset/preset.hpp"
#include <list>
#include <string>
#include <vector>

extern uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
extern uint8_t sendDeleteAllPatches(uint8_t layerId);

// extern uint8_t sendCreatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount,
//                                      float offset);
// extern uint8_t sendUpdatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount,
//                                      float offset);
// extern uint8_t sendDeletePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId);

typedef struct {
    uint8_t sourceID;
    uint8_t targetID;
    float amount;
} patchSaveStruct;

class Layer {
  public:
    Layer(uint32_t id) : id(id) {
        // add Modules
        modules.push_back(&oscA);
        modules.push_back(&oscB);
        modules.push_back(&sub);
        modules.push_back(&noise);
        modules.push_back(&mixer);
        modules.push_back(&steiner);
        modules.push_back(&ladder);
        modules.push_back(&envA);
        modules.push_back(&envF);
        modules.push_back(&lfoA);
        modules.push_back(&lfoB);
        modules.push_back(&distort);
        modules.push_back(&globalModule);
        modules.push_back(&midi);
        modules.push_back(&imperfect);

        // skip layer settings?

        initID();
    }

    inline std::vector<BaseModule *> &getModules() { return modules; } // return modules

    // init Layer
    void initID();

    void resetLayer();

    // Patch stuff
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendCreatePatchInOut;
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t)> sendDeletePatchInOut;
    // static std::function<uint8_t(uint8_t)> sendDeleteAllPatches;

    // clear all Layer patchesInOut
    void clearPatches();
    void addPatchInOut(Output &sourceOut, Input &targetIn, float amount = 0);
    void addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void updatePatchInOutWithoutMapping(PatchElement &patch, float amount = 0);
    void updatePatchInOutByIdWithoutMapping(uint8_t outputId, uint8_t inputId, float amount = 0);
    void removePatchInOut(PatchElement &patch);
    void removePatchInOutById(uint8_t outputId, uint8_t inputId);

    void collectLayerConfiguration();

    // void addPatchOutOut(Output &sourceOut, Output &targetOut, float amount = 0, float offset = 0);
    // void addPatchOutOutById(uint8_t outputOutId, uint8_t outputInId, float amount = 0, float offset = 0);
    // void updatePatchOutOutWithoutMapping(PatchElementOutOut &patch, float amount = 0, float offset = 0);
    // void updatePatchOutOutByIdWithoutMapping(uint8_t outputOutId, uint8_t outputInId, float amount = 0,
    //                                          float offset = 0);
    // void removePatchOutOut(PatchElementOutOut &patch);
    // void removePatchOutOutById(uint8_t outputOutId, uint8_t outputInId);

#ifdef POLYCONTROL
    void saveLayerToPreset(presetStruct *preset, std::string firstName, std::string secondName, std::string thirdName);
    void loadLayerFromPreset(presetStruct *preset);
#endif

    inline std::list<PatchElement> &getPatchesInOut() { return patchesInOut; }
    // inline std::list<PatchElementOutOut> &getPatchesOutOut() { return patchesOutOut; }

#ifdef POLYRENDER
    inline void gateOn(uint16_t voice) {
        lfoA.gateOn(voice);
        lfoB.gateOn(voice);
        envA.gateOn(voice);
        envF.gateOn(voice);
        // oscA.gateOn(voice);
        // oscB.gateOn(voice);
        midi.gateOn(voice);
    }

    inline void gateOff(uint16_t voice) { midi.gateOff(voice); }

    inline void setNote(uint16_t voice, uint8_t note, uint8_t velocity) { midi.setNote(voice, note, velocity); }

    inline void allGatesOff() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            gateOff(voice);
        }
    }

#endif

    uint8_t id;

    Midi midi = Midi("MIDI INTERFACE", "MIDI");
    OSC_A oscA = OSC_A("OSCILLATOR A", "OSC A");
    OSC_B oscB = OSC_B("OSCILLATOR B", "OSC B");
    Sub sub = Sub("SUB OSCILLATOR (A)", "SUB");
    Noise noise = Noise("NOISE GENERATOR", "NOISE");
    Mixer mixer = Mixer("MIXER", "MIXER");
    Steiner steiner = Steiner("STEINER FILTER", "STEINER");
    Ladder ladder = Ladder("LADDER FILTER", "LADDER");
    Distortion distort = Distortion("DISTORTION", "DIST");
    LFO lfoA = LFO("LFO A", "LFO A");
    LFO lfoB = LFO("LFO B", "LFO B");
    ADSR envA = ADSR("ENVELOPE VCA", "ENV A");
    ADSR envF = ADSR("ENVELOPE VCF", "ENV F");
    GlobalModule globalModule = GlobalModule("GLOBAL", "GLOBAL");
    Imperfection imperfect = Imperfection("FEEL", "FEEL"); // vll auch feel, defect

    std::vector<BaseModule *> modules; //  vector of all modules
    std::vector<Input *> inputs;       //  vector of all inputs
    std::vector<Output *> outputs;     //  vector of all outputs
    std::list<PatchElement> patchesInOut;
    // std::list<PatchElementOutOut> patchesOutOut;

    Setting LayerState = Setting("LayerState", 0, 1, 10, false, binary);

  private:
};