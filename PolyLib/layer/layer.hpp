#pragma once

#include "datacore/datacore.hpp"
#include "modules/modules.hpp"
#include "storage/loadStoredData.hpp"
#include <algorithm>
#include <list>
#include <string>
#include <vector>

#ifdef POLYRENDER
#include "render/renderAudioDef.h"
#include "render/renderCVDef.h"
#endif

const uint32_t LAYER_STORESIZE = 4096;

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
        // modules.push_back(&layersettings);
        modules.push_back(&tune);

        lfos.push_back(&lfoA);
        lfos.push_back(&lfoB);
        adsrs.push_back(&envA);
        adsrs.push_back(&envF);

        initID();

#ifdef POLYCONTROL
        oscA.storeID = 0x00;
        oscB.storeID = 0x01;
        waveshaperA.storeID = 0x02;
        phaseshaperA.storeID = 0x03;
        waveshaperB.storeID = 0x04;
        phaseshaperB.storeID = 0x05;
        noise.storeID = 0x06;
        mixer.storeID = 0x07;
        steiner.storeID = 0x08;
        ladder.storeID = 0x09;
        envA.storeID = 0x0A;
        envF.storeID = 0x0B;
        lfoA.storeID = 0x0C;
        lfoB.storeID = 0x0D;
        out.storeID = 0x0E;
        midi.storeID = 0x0F;
        feel.storeID = 0x10;
        // layersettings.storeID = 0x11;
        tune.storeID = 0x12;

        renderedAudioWavesOscA = renderedAudioWaves;
        renderedAudioWavesOscB = &renderedAudioWaves[100];
#endif
    }

    inline std::vector<BaseModule *> &getModules() { return modules; } // return modules

    // init Layer
    void initID();
    void resetLayer();

    // patch stuff
    void clearPatches();
    void addPatchInOut(Output &sourceOut, Input &targetIn, float amount = 0);
    void addPatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void updatePatchInOut(PatchElement &patch, float amount = 0);
    void updatePatchInOutById(uint8_t outputId, uint8_t inputId, float amount = 0);
    void removePatchInOut(PatchElement &patch);
    void removePatchInOutById(uint8_t outputId, uint8_t inputId);
    inline std::list<PatchElement> &getPatchesInOut() { return patchesInOut; }

#ifdef POLYCONTROL
    void loadDefaultPatches();

    void layerServiceRoutine();
    void resendLayerConfig();

    void getLayerConfiguration(int32_t *buffer, bool noFilter);
    void setLayerConfigration(int32_t *buffer, bool noFilter);
    void clearPresetLocks();

    void setClearMarker();
    void setResetMarker();

    uint32_t writeLayer(uint32_t blockStartIndex);

    int8_t renderedAudioWaves[200];
    int8_t *renderedAudioWavesOscA; // size 100
    int8_t *renderedAudioWavesOscB; // size 100

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
    void fillNoteImperfection();

    volatile uint8_t chipID = 0;

    float bpm = 120;

#endif

    uint8_t id;

    Midi midi = Midi("MIDI INTERFACE", "MIDI");
    OSC_A oscA = OSC_A("OSCILLATOR A", "OSC A");
    OSC_B oscB = OSC_B("OSCILLATOR B", "OSC B");
    Waveshaper waveshaperA = Waveshaper("WAVESHAPER A", "WAVE A");
    Waveshaper waveshaperB = Waveshaper("WAVESHAPER B", "WAVE B");
    Phaseshaper phaseshaperA = Phaseshaper("PHASESHAPER A", "PHASE A");
    Phaseshaper phaseshaperB = Phaseshaper("PHASESHAPER B", "PHASE B");
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
    Tune tune = Tune("TUNE", "TUNE");

    std::vector<BaseModule *> modules; //  vector of all modules
    std::vector<Input *> inputs;       //  vector of all inputs
    std::vector<Output *> outputs;     //  vector of all outputs

    // for clocking purposes
    std::vector<ADSR *> adsrs;
    std::vector<LFO *> lfos;

    std::list<PatchElement> patchesInOut;

    bool clearPatchesMarker = false;
    bool resetMarker = true;
    bool removePatchMarker = true;
    bool layerState = false;

  private:
};