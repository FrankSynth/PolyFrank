#pragma once

#include "guiPanelBase.hpp"
#include "livedata/liveData.hpp"

class GUIPanelVoice : public GUIPanelBase {
  public:
    void init(uint8_t layerID, uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0);
    void Draw();

  private:
    // Boxes

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    uint16_t elementWidth;
    uint16_t elementHeigth;
    uint8_t layerID = 0;
};
