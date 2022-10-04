#pragma once

#define MATRIXROWS 12
#define PATCHMATRIXROWS 6

#define MATRIXCOLUMN 6

#include "guiPanelBase.hpp"

typedef enum { MIDIVIEW, AUDIOVIEW, ENVVIEW } ViewMode;

class GUIPanelPatchMatrix : public GUIPanelBase {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0, std::string name = "", uint8_t id = 0,
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

    void scrollModulePosition(int32_t scroll);
    void scrollInPosition(int32_t scroll);
    void scrollOutPosition(int32_t scroll);

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
    void setMidiView() {
        viewMode = MIDIVIEW;
        isEnv = 0;
        isMidi = 1;
        isAudio = 0;
    }
    void setEnvView() {
        viewMode = ENVVIEW;
        isEnv = 1;
        isMidi = 0;
        isAudio = 0;
    }
    void setAudioView() {
        viewMode = AUDIOVIEW;
        isEnv = 0;
        isMidi = 0;
        isAudio = 1;
    }

  private:
    // Boxes

    FOCUSMODE mode;

    ViewMode viewMode = ENVVIEW;
    uint16_t entrysModule = 0;
    uint16_t entrysIn = 0;
    uint16_t entrysOut = 0;

    std::vector<BaseModule *> allModules;
    std::vector<Analog *> allInputs;
    std::vector<Output *> allOutputs;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;

    Scroller scrollModule = Scroller(MATRIXROWS);
    Scroller scrollIn = Scroller(PATCHMATRIXROWS);
    Scroller scrollOut = Scroller(MATRIXCOLUMN);

    int32_t filteredView = 0;
    int32_t midiView = 0;

    int32_t isEnv = 1;
    int32_t isAudio = 0;
    int32_t isMidi = 0;

    location newLocation;
    uint8_t updateLocation = false;

    MatrixModule_PanelElement panelElementsModule[MATRIXROWS];
    MatrixIn_PanelElement panelElementsIn[MATRIXROWS];
    MatrixPatch_PanelElement panelElementsPatch[MATRIXCOLUMN][PATCHMATRIXROWS];
    MatrixOut_PanelElement panelElementsOut[MATRIXCOLUMN];
};
