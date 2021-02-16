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

    void setFocus(FOCUSMODE focusMode) {

        newFocus.type = focusMode;
        if (focusMode == FOCUSMODULE) {
            newFocus.modul = panelElementsModule[scrollModule.position - scrollModule.offset].entry->id;
            newFocus.layer = panelElementsModule[scrollModule.position - scrollModule.offset].entry->layerId;
        }

        else if (focusMode == FOCUSOUTPUT) {
            newFocus.id = panelElementsSource[scrollSource.position - scrollModule.offset].entry->id;
            newFocus.modul = panelElementsSource[scrollSource.position - scrollModule.offset].entry->moduleId;
            newFocus.layer = panelElementsSource[scrollSource.position - scrollModule.offset].entry->layerId;
        }
        else if (focusMode == FOCUSINPUT) {
            newFocus.id = panelElementsTarget[scrollTarget.position - scrollModule.offset].entry->id;
            newFocus.modul = panelElementsTarget[scrollTarget.position - scrollModule.offset].entry->moduleId;
            newFocus.layer = panelElementsTarget[scrollTarget.position - scrollModule.offset].entry->layerId;
        }
    }

    void toggleFlipView() { flipView = !flipView; }
    void toggleFilterdView() { filteredView = !filteredView; }

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
    uint8_t filteredView = 0;

    uint16_t absXPositions[3];

    Patch_PanelElement panelElementsSource[PATCHPANELENTRYS];
    Patch_PanelElement panelElementsTarget[PATCHPANELENTRYS];
    Module_PanelElement panelElementsModule[PATCHPANELENTRYS];
};
