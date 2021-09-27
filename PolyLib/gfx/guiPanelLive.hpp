#pragma once

#include "guiPanelBase.hpp"
#include "livedata/liveData.hpp"

class GUIPanelLive : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 1);
    void Draw();

    void updateEntrys();

    void registerSettingsElements();

    void registerPanelSettings();

    void selectSubPanel(uint8_t subPanelSelect);

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = LIVEPANELENTRYS;

    uint8_t subPanelSelect = 0;

    uint16_t EntrysPerElement = 4;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scroll = Scroller(LIVEPANELENTRYS);

    categoryStruct *pCategory = nullptr;

    Live_PanelElement panelElements[LIVEPANELENTRYS];
};