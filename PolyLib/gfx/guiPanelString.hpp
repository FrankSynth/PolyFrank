#pragma once

#include "guiPanelBase.hpp"

class GUIPanelString : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0);
    void registerPanelSettings();
    void setPanel(uint8_t subPanelID);
    void Draw();

  private:
    // Boxes

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    uint8_t subPanelID = 0;
};