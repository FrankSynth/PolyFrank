#pragma once

#include "guiPanelBase.hpp"
#include "livedata/liveData.hpp"
extern void clearPotiState(uint32_t layer);

class GUIPanelLive : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0, std::string name = "",
              uint8_t pathVisible = 1, uint8_t subPanel = 0);
    void Draw();

    void updateEntrys();

    void registerSettingsElements();

    void registerPanelSettings();

    void selectSubPanel(uint8_t subPanelSelect);

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = LIVEPANELENTRYS;

    int32_t subPanelSelect = 0;
    int32_t subPanelSelection[6] = {1, 0, 0, 0, 0, 0};

    uint16_t EntrysPerElement = 4;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scroll = Scroller(LIVEPANELENTRYS);

    std::vector<Setting *> *settings = nullptr;
    std::vector<Setting *> entryPointer;
    Live_PanelElement panelElements[LIVEPANELENTRYS];
};