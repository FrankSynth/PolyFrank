#pragma once

#include "datacore/datacore.hpp"
#include "modules/modules.hpp"
#include "preset/preset.hpp"
#include <list>
#include <string>
#include <vector>

#ifdef POLYRENDER
#include "render/renderAudioDef.h"
#include "render/renderCVDef.h"
#endif

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
        modules.push_back(&out);
        modules.push_back(&midi);
        modules.push_back(&feel);
        modules.push_back(&layersettings);

        lfos.push_back(&lfoA);
        lfos.push_back(&lfoB);
        adsrs.push_back(&envA);
        adsrs.push_back(&envF);

        // skip layer settings?

        initID();
    }

    inline std::vector<BaseModule *> &getModules() { return modules; } // return modules

    // init Layer
    void initID();

    void resetLayer();

    // patch stuff
    void clearPatches();
    void addPatchInOut(Output &sourceOut, Input &targetIn, float amount = 0);
    void addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void updatePatchInOutWithoutMapping(PatchElement &patch, float amount = 0);
    void updatePatchInOutByIdWithoutMapping(uint8_t outputId, uint8_t inputId, float amount = 0);
    void removePatchInOut(PatchElement &patch);
    void removePatchInOutById(uint8_t outputId, uint8_t inputId);

#ifdef POLYCONTROL
    void collectLayerConfiguration();

    void saveLayerToPreset(presetStruct *preset, std::string firstName, std::string secondName, std::string thirdName);
    void loadLayerFromPreset(presetStruct *preset);
#endif

    inline std::list<PatchElement> &getPatchesInOut() { return patchesInOut; }

#ifdef POLYRENDER
    inline void gateOn(uint16_t voice) {
        lfoA.gateOn(voice);
        lfoB.gateOn(voice);
        envA.gateOn(voice);
        envF.gateOn(voice);
        midi.gateOn(voice);
    }

    inline void gateOff(uint16_t voice) {
        midi.gateOff(voice);

        envA.gateOff(voice);
        envF.gateOff(voice);
    }

    inline void setNote(uint16_t voice, uint8_t note, uint8_t velocity) { midi.setNote(voice, note, velocity); }

    inline void allGatesOff() {
        for (uint16_t voice = 0; voice < VOICESPERCHIP; voice++) {
            gateOff(voice);
        }
    }

    vec<VOICESPERCHIP> spreadValues;
    float noteImperfection[OSCPERVOICE][VOICESPERCHIP][NOTEIMPERFECTIONBUFFERSIZE];
    vec<VOICESPERCHIP> lfoImperfection;
    vec<VOICESPERCHIP> adsrImperfection;

    void initLayer();

    volatile uint8_t chipID = 0;

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
    LFO lfoA = LFO("LFO A", "LFO A");
    LFO lfoB = LFO("LFO B", "LFO B");
    ADSR envA = ADSR("ENVELOPE VCA", "ENV A");
    ADSR envF = ADSR("ENVELOPE VCF", "ENV F");
    Out out = Out("OUTPUT", "OUTPUT");
    Feel feel = Feel("FEEL", "FEEL");
    LayerSetting layersettings = LayerSetting("LAYERSETTINGS", "LA SET");

    std::vector<BaseModule *> modules; //  vector of all modules
    std::vector<Input *> inputs;       //  vector of all inputs
    std::vector<Output *> outputs;     //  vector of all outputs

    // for clocking purposes

    std::vector<ADSR *> adsrs;
    std::vector<LFO *> lfos;

    std::list<PatchElement> patchesInOut;

    Setting layerState = Setting("layerState", 0, 1, 0, false, binary);

  private:
};