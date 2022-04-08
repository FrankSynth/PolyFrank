#pragma once

#include "guiPanelBase.hpp"

class GUIPanelConfig : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 0);
    void Draw();

    void updateEntrys();

    void registerGlobalSettings();

    void registerPanelSettings();

    void registerPanelElements(categoryStruct &settingsCategory);

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = CONFIGPANELENTRYS;

    uint16_t EntrysPerElement = 4;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scroll = Scroller(CONFIGPANELENTRYS);

    uint16_t elementIndex = 0;

    Data_PanelElement panelElements[CONFIGPANELENTRYS];
};