#pragma once

#include "guiPanelBase.hpp"

class GUIPanelFocus : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0, uint8_t pathVisible = 1);
    void Draw();

    void collectEntrys();

    void registerModuleSettings(Data_PanelElement *dataPanelElements);
    void registerModulePatchIn();
    void registerModulePatchOut();
    void registerLayerModules();

    void registerPanelSettings();

    void updateEntrys();

    Analog *getAnalogEntry();
    Digital *getDigitalEntry();
    BaseModule *getModuleEntry();
    PatchElement *getPatchEntry();

  private:
    // Boxes
    uint16_t focusChanged;

    uint16_t customControlsY;
    uint16_t customControlsHeight;

    uint16_t analogIndex;
    uint16_t digitalIndex;
    uint16_t moduleIndex;
    uint16_t patchIndex;

    FOCUSMODE mode;

    uint16_t entrys = FOCUSPANELENTRYS;

    location newPanelFocus;

    std::vector<BaseModule *> module;
    std::vector<Analog *> analog;
    std::vector<Digital *> digital;
    std::vector<PatchElement *> patch;

    uint16_t SwitchEntrysPerElement = 4;
    uint16_t AnalogEntrysPerElement = 1;
    uint16_t PatchEntrysPerElement = 1;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    location oldFocus;

    Scroller scrollModule = Scroller(FOCUSPANELENTRYS);
    Scroller scrollLayer = Scroller(FOCUSPANELENTRYS);
    // Scroller scroll = Scroller(FOCUSPANELENTRYS);

    Scroller *scroll = nullptr;

    Data_PanelElement panelElements[FOCUSPANELENTRYS];
    Data_PanelElement panelElementsWave[FOCUSPANELENTRYSWAVE];
};
