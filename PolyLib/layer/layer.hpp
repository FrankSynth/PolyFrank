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
        modules.push_back(&waveshaperA);
        modules.push_back(&phaseshaperA);
        modules.push_back(&waveshaperB);
        modules.push_back(&phaseshaperB);
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

        // for (size_t i = 0; i < VOICESPERCHIP; i++) {
        //     std::string name = "L: VOICE " + std::to_string(i);
        //     LadderTune.push_back(Analog(name.c_str(), 0.9, 1, 1, true));
        // }
        // for (size_t i = 0; i < VOICESPERCHIP; i++) {
        //     std::string name = "S: VOICE " + std::to_string(i);
        //     SteinerTune.push_back(Analog(name.c_str(), 0.9, 1, 1, true));
        // }

        // skip layer settings?

        initID();

#ifdef POLYCONTROL
        renderedAudioWavesOscA = renderedAudioWaves;
        renderedAudioWavesOscB = &renderedAudioWaves[100];
        renderedAudioWavesSub = &renderedAudioWaves[200];
#endif
    }

    inline std::vector<BaseModule *> &getModules() { return modules; } // return modules

    // init Layer
    void initID();

    void resetLayer();

    // patch stuff
    void loadDefaultPatches();
    void clearPatches();
    void addPatchInOut(Output &sourceOut, Input &targetIn, float amount = 0);
    void addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void updatePatchInOutWithoutMapping(PatchElement &patch, float amount = 0);
    void updatePatchInOutByIdWithoutMapping(uint8_t outputId, uint8_t inputId, float amount = 0);
    void removePatchInOut(PatchElement &patch);
    void removePatchInOutById(uint8_t outputId, uint8_t inputId);
    inline std::list<PatchElement> &getPatchesInOut() { return patchesInOut; }

#ifdef POLYCONTROL

    void resendLayerConfig();

    void collectLayerConfiguration(int32_t *buffer, bool noFilter);
    void writeLayerConfiguration(int32_t *buffer, bool noFilter);
    void clearPresetLocks();

    int8_t renderedAudioWaves[300];
    int8_t *renderedAudioWavesOscA; // size 100
    int8_t *renderedAudioWavesOscB; // size 100
    int8_t *renderedAudioWavesSub;  // size 100

#endif

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
        for (uint32_t voice = 0; voice < VOICESPERCHIP; voice++) {
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
    Waveshaper waveshaperA = Waveshaper("WAVESHAPER A", "WAVE A");
    Waveshaper waveshaperB = Waveshaper("WAVESHAPER B", "WAVE B");
    Phaseshaper phaseshaperA = Phaseshaper("PHASESHAPER A", "PHASE A");
    Phaseshaper phaseshaperB = Phaseshaper("PHASESHAPER B", "PHASE B");
    Sub sub = Sub("SUB OSCILLATOR (A)", "SUB");
    Noise noise = Noise("NOISE GENERATOR", "NOISE");
    Mixer mixer = Mixer("MIXER", "MIXER");
    Steiner steiner = Steiner("STEINER FILTER", "STEINER");
    Ladder ladder = Ladder("LADDER FILTER", "LADDER");
    LFO lfoA = LFO("LFO A", "LFO A");
    LFO lfoB = LFO("LFO B", "LFO B");
    ADSR envA = ADSR("ENVELOPE VCA", "ENV A");
    ADSR envF = ADSR("ENVELOPE VCF", "ENV F");
    Out out = Out("MASTER", "MASTER");
    Feel feel = Feel("FEEL", "FEEL");
    LayerSetting layersettings = LayerSetting("LAYERSETTINGS", "LYRSET");

    std::vector<BaseModule *> modules; //  vector of all modules
    std::vector<Input *> inputs;       //  vector of all inputs
    std::vector<Output *> outputs;     //  vector of all outputs

    // for clocking purposes

    std::vector<ADSR *> adsrs;
    std::vector<LFO *> lfos;

    std::list<PatchElement> patchesInOut;

    // std::vector<Analog> LadderTune;
    // std::vector<Analog> SteinerTune;

    Setting layerState = Setting("layerState", 0, 1, 0, false, binary);

  private:
};