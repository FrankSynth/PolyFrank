#pragma once

#define MATRIXROWS 6
#define MATRIXCOLUMN 7

#include "guiPanelBase.hpp"

class GUIPanelPatchMatrix : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0,
              uint8_t pathVisible = 1);
    void Draw();

    void collectModules();
    void collectInputs();
    void collectOutputs();

    void registerElements();
    void registerPanelSettings();

    void addCurrentPatch();
    void removeCurrentPatch();
    void clearPatches();

    void activate();

    void scrollModulePosition(int16_t scroll);
    void scrollInPosition(int16_t scroll);
    void scrollOutPosition(int16_t scroll);

    void setFocus(FOCUSMODE focusMode) {

        newFocus.type = focusMode;
        if (focusMode == FOCUSMODULE) {
            newFocus.modul = allModules[scrollModule.relPosition]->id;
            newFocus.layer = allModules[scrollModule.relPosition]->layerId;
        }

        else if (focusMode == FOCUSOUTPUT) {
            newFocus.id = allOutputs[scrollOut.relPosition]->id;
            newFocus.modul = allOutputs[scrollOut.relPosition]->moduleId;
            newFocus.layer = allOutputs[scrollOut.relPosition]->layerId;
        }
        else if (focusMode == FOCUSINPUT) {
            newFocus.id = allInputs[scrollIn.relPosition]->id;
            newFocus.modul = allInputs[scrollIn.relPosition]->moduleId;
            newFocus.layer = allInputs[scrollIn.relPosition]->layerId;
        }
    }

    void toggleFilterdView() { filteredView = !filteredView; }
    void toggleMidiView() { midiView = !midiView; }

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrysModule = 0;
    uint16_t entrysIn = 0;
    uint16_t entrysOut = 0;

    std::vector<BaseModule *> allModules;
    std::vector<Input *> allInputs;
    std::vector<Output *> allOutputs;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scrollModule = Scroller(MATRIXROWS);
    Scroller scrollIn = Scroller(MATRIXROWS);
    Scroller scrollOut = Scroller(MATRIXCOLUMN);

    int32_t filteredView = 0;
    int32_t midiView = 0;

    MatrixModule_PanelElement panelElementsModule[MATRIXROWS];
    MatrixIn_PanelElement panelElementsIn[MATRIXROWS];
    MatrixPatch_PanelElement panelElementsPatch[MATRIXCOLUMN][MATRIXROWS];
    MatrixOut_PanelElement panelElementsOut[MATRIXCOLUMN];
};
