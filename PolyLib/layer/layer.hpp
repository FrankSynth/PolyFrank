#pragma once

#include <list>
#include <string>
#include <vector>

#include "datacore/datacore.hpp"
#include "modules/layersettings.hpp"
#include "modules/modules.hpp"

extern uint8_t sendCreatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendUpdatePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId, float amount);
extern uint8_t sendDeletePatchInOut(uint8_t layerId, uint8_t outputId, uint8_t inputId);
extern uint8_t sendDeleteAllPatches(uint8_t layerId);

extern uint8_t sendCreatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount,
                                     float offset);
extern uint8_t sendUpdatePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId, float amount,
                                     float offset);
extern uint8_t sendDeletePatchOutOut(uint8_t layerId, uint8_t outputOutId, uint8_t outputInId);

class Layer {
  public:
    Layer(uint32_t id) : id(id) {
        // add Modules
        modules.push_back(&adsrA);
        modules.push_back(&adsrB);
        modules.push_back(&midi);
        // skip layer settings?

        initID();
    }

    std::vector<BaseModule *> &getModules() { return modules; } // return modules

    // init Layer
    void initID();

    // Patch stuff
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t, float)> sendCreatePatchInOut;
    // static std::function<uint8_t(uint8_t, uint8_t, uint8_t)> sendDeletePatchInOut;
    // static std::function<uint8_t(uint8_t)> sendDeleteAllPatches;

    // clear all Layer patchesInOut
    void clearPatches();
    void addPatchInOut(Output &sourceOut, Input &targetIn, float amount = 0);
    void addPatchInOut(uint8_t outputId, uint8_t inputId, float amount = 0);
    void updatePatchInOut(PatchElementInOut &patch, float amount = 0);
    void updatePatchInOut(uint8_t outputId, uint8_t inputId, float amount = 0);
    void removePatchInOut(PatchElementInOut &patch);
    void removePatchInOut(uint8_t outputId, uint8_t inputId);

    void addPatchOutOut(Output &sourceOut, Output &targetOut, float amount = 0, float offset = 0);
    void addPatchOutOut(uint8_t outputOutId, uint8_t outputInId, float amount = 0, float offset = 0);
    void updatePatchOutOut(PatchElementOutOut &patch, float amount = 0, float offset = 0);
    void updatePatchOutOut(uint8_t outputOutId, uint8_t outputInId, float amount = 0, float offset = 0);
    void removePatchOutOut(PatchElementOutOut &patch);
    void removePatchOutOut(uint8_t outputOutId, uint8_t outputInId);

    std::list<PatchElementInOut> &getPatchesInOut() { return patchesInOut; }
    std::list<PatchElementOutOut> &getPatchesOutOut() { return patchesOutOut; }

    // ID for
    LayerSettings layerSettings = LayerSettings("LayerSettings");

    uint8_t id;


  private:
    std::vector<BaseModule *> modules; //  vector of all modules
    std::vector<Input *> inputs;       //  vector of all inputs
    std::vector<Output *> outputs;     //  vector of all outputs
    std::list<PatchElementInOut> patchesInOut;
    std::list<PatchElementOutOut> patchesOutOut;

    // sendout?

    // modules, don't forget them to push into modules vector in constructor
    // LFO lfo_1 = LFO("LFO1");
    // LFO lfo_2 = LFO("LFO2");
    ADSR adsrA = ADSR("ADSR A");
    ADSR adsrB = ADSR("ADSR B");
    Midi midi = Midi("MIDI");
};