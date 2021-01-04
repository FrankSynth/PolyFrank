#pragma once

#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "globalSettings/globalSettings.hpp"
#include "guiActionHandler.hpp"
#include "guiBase.hpp"
#include "layer/layer.hpp"
#include "poly.hpp"
#include <functional>
#include <string>

#define DATAPANELENTRYS 6
#define CONFIGPANELENTRYS 5
#define PATCHPANELENTRYS 7

/*Aufbau Data Panel

  - GUIPanelData
    -registriert Panel im ActionHandler
    -legt n Data_PanelElement an

    -> Data_PanelElement[n]
        -registriert Entry im ActionHandler

      - besteht aus m data entrys verschiedener Typen
      - entscheidet was gerendert wird

        -> entrys halten die information jedes Datenfelds
        - zu registrierende functionen
        - pointer zum daten element



*/

void drawModuleElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawPatchInOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawPatchOutOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawDigitalElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawAnalogElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

// GUIHeader Box for Panel Selection
class Data_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->heigth = heigth;
        this->select = select;
    }
    void Draw();

    void setName(std::string *name);

    void addAnalogEntry(Analog *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{ANALOG, data, nullptr, nullptr, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addModuleEntry(BaseModule *data, actionHandle functionCW, actionHandle functionCCW,
                        actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{MODULE, nullptr, nullptr, nullptr, nullptr, data, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addDigitalEntry(Digital *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{DIGITAL, nullptr, data, nullptr, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addSettingsEntry(Setting *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{SETTING, nullptr, nullptr, data, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addPatchInputEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                            actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{PATCHINPUT, nullptr, nullptr, nullptr, patch, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addPatchOutOutEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                             actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{PATCHOUTOUT, nullptr, nullptr, nullptr, patch, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addPatchOutputEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                             actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{PATCHOUTPUT, nullptr, nullptr, nullptr, patch, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addEmptyEntry() {

        entrys[numberEntrys] = entryStruct{EMPTY,   nullptr,       nullptr,       nullptr,      nullptr,
                                           nullptr, {nullptr, ""}, {nullptr, ""}, {nullptr, ""}};

        numberEntrys++;
        visible = 1;
    }

    uint8_t select = 0;
    uint8_t visible = 0;
    uint16_t numberEntrys = 0;

    std::string *panelElementName; // custom Name

  private:
    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;

    entryStruct entrys[4];

    uint16_t width;
    uint16_t heigth;
};

// GUIHeader Box for Panel Selection
class Patch_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->entryWidth = width;
        this->entryHeight = heigth;
    }
    void Draw();

    void addEntry(BasePatch *patch) {

        entry = patch;
        active = 1;
    }

    uint8_t patched = 0;
    uint8_t active = 0;
    uint8_t select = 0;
    uint16_t panelAbsX;

    BasePatch *entry;

  private:
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;
};

class Module_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->entryWidth = width;
        this->entryHeight = heigth;
        this->select = select;
    }
    void Draw();

    void addEntry(BaseModule *modulePointer) {
        entry = modulePointer;
        active = 1;
    }
    uint8_t patched = 0;
    uint8_t select = 0;
    uint8_t active = 0;
    uint16_t panelAbsX;

    BaseModule *entry;

  private:
    uint16_t panelAbsY;

    uint16_t entryWidth;
    uint16_t entryHeight;
};

// GUIHeader Box for Panel Selection

class GUIPanelData : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0);
    void Draw();

    uint16_t updateEntrys();
    void registerModuleSettings();
    void registerModulePatchIn();
    void registerModulePatchOut();
    void registerLayerModules();

    void registerPanelSettings();

    void changeScroll(int16_t change);
    void checkScroll();
    void resetScroll();

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = DATAPANELENTRYS;

    uint16_t SwitchEntrysPerElement = 3;
    uint16_t AnalogEntrysPerElement = 1;
    uint16_t PatchEntrysPerElement = 1;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    location oldLocation;
    uint16_t scroll = 0;
    uint8_t scrollOffset = 0;

    location newFocusLocation;

    Data_PanelElement panelElements[DATAPANELENTRYS];
};

class GUIPanelConfig : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0);
    void Draw();

    uint16_t updateEntrys();

    void registerGlobalSettings();

    void registerPanelSettings();

    void changeScroll(int16_t change);
    void checkScroll();
    void resetScroll();

  private:
    // Boxes

    FOCUSMODE mode;

    uint16_t entrys = CONFIGPANELENTRYS;

    uint16_t EntrysPerElement = 3;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    uint16_t scroll = 0;
    uint8_t scrollOffset = 0;

    Data_PanelElement panelElements[CONFIGPANELENTRYS];
};

class GUIPanelPatch : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, std::string name = "", uint8_t id = 0);
    void Draw();

    void updateEntrys();

    void registerElements();

    void registerPanelSettings();

    void addCurrentPatch();

    void removeCurrentPatch();
    void clearPatches();
    void changeScroll(int16_t changeModul, int16_t changeTarget, int16_t changeSource);
    void checkScroll();
    void resetScroll();
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

    uint16_t scrollModule = 0;
    uint8_t scrollModuleOffset = 0;

    uint16_t scrollSource = 0;
    uint8_t scrollSourceOffset = 0;

    uint16_t scrollTarget = 0;
    uint8_t scrollTargetOffset = 0;

    uint8_t flipView = 0;
    uint16_t absXPositions[3];

    Patch_PanelElement panelElementsSource[PATCHPANELENTRYS];
    Patch_PanelElement panelElementsTarget[PATCHPANELENTRYS];
    Module_PanelElement panelElementsModule[PATCHPANELENTRYS];
};