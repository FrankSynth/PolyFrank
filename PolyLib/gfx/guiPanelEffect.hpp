#pragma once

#include "guiPanelBase.hpp"
#include "livedata/liveData.hpp"

typedef enum { OSCA, OSCB, NOISE, NOMODULE } moduleSelect;
typedef enum { WAVE, PHASE, CRUSH, NOEFFECT } effectSelect;

class GUIPanelEffect : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0, uint8_t pathVisible = 1);
    void Draw();

    void updateEntrys(BaseModule *module);
    void drawWaveShaperPanel(int8_t *renderedWave, Waveshaper *module);
    void drawPhaseShaperPanel(int8_t *renderedWave, Phaseshaper *module);
    void drawCrushPanel(int8_t *renderedWave);

    void updateModuleSelection();
    void updateEffectSelection();

    void registerElements();

    void registerPanelSettings();
    void registerOverviewEntrys();

    void selectModule(moduleSelect module);
    void selectEffect(effectSelect effect);

  private:
    // Boxes

    moduleSelect moduleType = NOMODULE;
    effectSelect effectType = NOEFFECT;

    moduleSelect newModuleType = (moduleSelect)0;
    effectSelect newEffectType = (effectSelect)0;

    int32_t moduleSelected[3];
    int32_t effectSelected[3];

    int32_t overview = false;

    bool updateEffect = false;
    bool updateModule = false;

    std::vector<Analog *> entrys;

    uint16_t EntrysPerElement = 4;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scroll = Scroller(LIVEPANELENTRYS);

    Effect_PanelElement effectPanelElements[LIVEPANELENTRYS];
    EffectAmount_PanelElement overviewPanelElements[3];
};