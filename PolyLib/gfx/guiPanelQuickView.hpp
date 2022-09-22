#pragma once

#include "guiPanelBase.hpp"

class GUIPanelQuickView : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0);
    void Draw();

  private:
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t x = 0;
    uint32_t y = 0;
};
