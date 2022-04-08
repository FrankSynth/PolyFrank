#pragma once
#include "guiPanelBase.hpp"
// PANEL Path Type
class GUIPanelState {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0);

    void Draw();

  private:
    // Boxes
    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    const GUI_FONTINFO *font = &GUI_FontBahnschrift24_FontInfo;
};
