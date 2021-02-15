#pragma once

#include "guiPanelBase.hpp"
#include "livedata/liveData.hpp"

class GUIPanelVoice : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 1);
    void Draw();

  private:
    // Boxes

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    uint16_t elementWidth;
    uint16_t elementHeigth;
};

