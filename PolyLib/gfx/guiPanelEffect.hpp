#pragma once

#include "guiPanelBase.hpp"
#include "livedata/liveData.hpp"

typedef enum { OSCA, OSCB, NOMODULE } moduleSelect;
typedef enum { WAVE, PHASE, NOEFFECT } effectSelect;

#define EFFECTPATCHELEMENTS 4
#define DRYWETINFO 20

class GUIPanelEffect : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0, uint8_t pathVisible = 1);
    void Draw();

    void activate() {
        // clear encoder
        actionHandler.registerActionEncoder(0);
        actionHandler.registerActionEncoder(1);
        actionHandler.registerActionEncoder(2);
        actionHandler.registerActionEncoder(3);
        actionHandler.registerActionEncoder(4);
        actionHandler.registerActionEncoder(5);
    };

    void updateEntrys();
    void drawWaveShaperPanel(int8_t *renderedWave, Waveshaper *module);
    void drawPhaseShaperPanel(int8_t *renderedWave, Phaseshaper *module);
    void drawCrushPanel(int8_t *renderedWave);

    void updateModuleSelection();
    void updateEffectSelection();
    void enableOverview();
    void togglePatchView();

    void registerElements();

    void registerPanelSettings();
    void registerOverviewEntrys();

    void patchToOutput(uint32_t entryID);

    void selectModule(moduleSelect module);
    void selectEffect(effectSelect effect);

  private:
    // Boxes

    BaseModule *module = nullptr;

    moduleSelect moduleType = NOMODULE;
    effectSelect effectType = NOEFFECT;

    moduleSelect newModuleType = (moduleSelect)0;
    effectSelect newEffectType = (effectSelect)0;

    int32_t moduleSelected[2];
    int32_t effectSelected[2];

    int32_t overview = false;

    int32_t patch = false;

    bool updateEffect = false;
    bool updateModule = false;
    bool scrollToPatchMax[2];

    Analog *entrys[2];

    uint16_t EntrysPerElement = 4;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scrollModule = Scroller(3);
    Scroller scrollDotsWave = Scroller(4);
    Scroller scrollDotsPhase = Scroller(4);

    Scroller scrollPatches[2];

    Effect_PatchElement effectPatchElements[2][EFFECTPATCHELEMENTS];

    Effect_PanelElement effectPanelElements[2];
    EffectAmount_PanelElement overviewPanelElements[2][3];

    PatchElement *activeEntryPatch[2];
};