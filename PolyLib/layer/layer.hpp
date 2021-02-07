#pragma once

#include "datacore/datacore.hpp"
#include "modules/layersettings.hpp"
#include "modules/modules.hpp"
#include "preset/preset.hpp"
#include <list>
#include <string>
#include <vector>

extern uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
extern uint8_t sendDeleteAllPatches(uint8_t layerId);

extern uint8_t sendCreatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount,
                                     float offset);
extern uint8_t sendUpdatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount,
                                     float offset);
extern uint8_t sendDeletePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId);

typedef struct {
    uint8_t sourceID;
    uint8_t targetID;
    float amount;
} patchSaveStruct;

class Layer {
  public:
    Layer(uint32_t id) : id(id) {
        // add Modules
        modules.push_back(&steiner);
        modules.push_back(&midi);
        modules.push_back(&oscA);
        modules.push_back(&oscB);
        modules.push_back(&sub);
        modules.push_back(&noise);
        modules.push_back(&ladder);
        modules.push_back(&distort);
        modules.push_back(&lfoA);
        modules.push_back(&lfoB);
        modules.push_back(&adsrA);
        modules.push_back(&adsrB);
        modules.push_back(&globalModule);
        modules.push_back(&test);
        // skip layer settings?

        initID();
    }

    inline std::vector<BaseModule *> &getModules() { return modules; } // return modules

    // init Layer
    void initID();

    // Patch stuff
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendCreatePatchInOut;
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t)> sendDeletePatchInOut;
    // static std::function<uint8_t(uint8_t)> sendDeleteAllPatches;

    // clear all Layer patchesInOut
    void clearPatches();
    void addPatchInOut(Output &sourceOut, Input &targetIn, float amount = 0);
    void addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void updatePatchInOut(PatchElementInOut &patch, float amount = 0);
    void updatePatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void removePatchInOut(PatchElementInOut &patch);
    void removePatchInOutById(uint8_t outputId, uint8_t inputId);

    void addPatchOutOut(Output &sourceOut, Output &targetOut, float amount = 0, float offset = 0);
    void addPatchOutOutById(uint8_t outputOutId, uint8_t outputInId, float amount = 0, float offset = 0);
    void updatePatchOutOut(PatchElementOutOut &patch, float amount = 0, float offset = 0);
    void updatePatchOutOutById(uint8_t outputOutId, uint8_t outputInId, float amount = 0, float offset = 0);
    void removePatchOutOut(PatchElementOutOut &patch);
    void removePatchOutOutById(uint8_t outputOutId, uint8_t outputInId);

#ifdef POLYCONTROL
    void saveLayerToPreset(uint32_t presetID, std::string firstName, std::string secondName);
    void loadLayerFromPreset(uint32_t presetID);
#endif

    inline std::list<PatchElementInOut> &getPatchesInOut() { return patchesInOut; }
    inline std::list<PatchElementOutOut> &getPatchesOutOut() { return patchesOutOut; }

    // ID for
    LayerSettings layerSettings = LayerSettings("LayerSettings");

    uint8_t id;

    Midi midi = Midi("MIDI");
    OSC_A oscA = OSC_A("OSC_A");
    OSC_B oscB = OSC_B("OSC_B");
    Sub sub = Sub("SUB");
    Noise noise = Noise("NOISE");
    Steiner steiner = Steiner("STEINER");
    Ladder ladder = Ladder("LADDER");
    Distortion distort = Distortion("DISTORTION");
    LFO lfoA = LFO("LFO A");
    LFO lfoB = LFO("LFO B");
    ADSR adsrA = ADSR("ADSR A");
    ADSR adsrB = ADSR("ADSR B");
    GlobalModule globalModule = GlobalModule("GLOBAL");
    TEST test = TEST("TEST");

    std::vector<BaseModule *> modules; //  vector of all modules
    std::vector<Input *> inputs;       //  vector of all inputs
    std::vector<Output *> outputs;     //  vector of all outputs
    std::list<PatchElementInOut> patchesInOut;
    std::list<PatchElementOutOut> patchesOutOut;

  private:
    // sendout?

    // modules, don't forget them to push into modules vector in constructor
    // LFO lfo_1 = LFO("LFO1");
    // LFO lfo_2 = LFO("LFO2");
};