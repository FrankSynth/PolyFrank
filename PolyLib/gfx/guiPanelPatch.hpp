#pragma once

#include "guiPanelBase.hpp"
class GUIPanelPatch : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 1);
    void Draw();

    void updateEntrys();

    void registerElements();

    void registerPanelSettings();

    void addCurrentPatch();
    void removeCurrentPatch();
    void clearPatches();

    void toggleFlipView() { flipView = !flipView; }

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrysModule = 0;
    uint16_t entrysSource = 0;
    uint16_t entrysTarget = 0;

    const uint16_t maxEntrys = PATCHPANELENTRYS;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scrollModule = Scroller(PATCHPANELENTRYS);
    Scroller scrollSource = Scroller(PATCHPANELENTRYS);
    Scroller scrollTarget = Scroller(PATCHPANELENTRYS);

    uint8_t flipView = 0;
    uint16_t absXPositions[3];

    Patch_PanelElement panelElementsSource[PATCHPANELENTRYS];
    Patch_PanelElement panelElementsTarget[PATCHPANELENTRYS];
    Module_PanelElement panelElementsModule[PATCHPANELENTRYS];
};
