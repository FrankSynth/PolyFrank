#pragma once

#include "guiPanelBase.hpp"

class GUIPanelFocus : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, uint8_t pathVisible = 1);
    void Draw();

    void updateEntrys();
    void registerModuleSettings();
    void registerModulePatchIn();
    void registerModulePatchOut();
    void registerLayerModules();

    void registerPanelSettings();

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = FOCUSPANELENTRYS;

    location newPanelFocus;

    uint16_t SwitchEntrysPerElement = 3;
    uint16_t AnalogEntrysPerElement = 1;
    uint16_t PatchEntrysPerElement = 1;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    location oldLocation;

    Scroller scrollModules = Scroller(FOCUSPANELENTRYS);
    Scroller scrollLayer = Scroller(FOCUSPANELENTRYS);
    Scroller scroll = Scroller(FOCUSPANELENTRYS);

    Data_PanelElement panelElements[FOCUSPANELENTRYS];
};
