#pragma once

#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "globalSettings/globalSettings.hpp"
#include "guiActionHandler.hpp"
#include "guiBase.hpp"
#include "layer/layer.hpp"
#include "livedata/liveData.hpp"
#include "poly.hpp"
#include "preset/preset.hpp"
#include <functional>
#include <string>


#define FOCUSPANELENTRYS 6
#define CONFIGPANELENTRYS 5
#define PATCHPANELENTRYS 7
#define PRESETPANELENTRYS 7
#define LIVEPANELENTRYS 3

extern const GUI_FONTINFO *fontSmall;
extern const GUI_FONTINFO *fontMedium;
extern const GUI_FONTINFO *fontBig;

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

void drawPresetElemet(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawModuleElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawPatchInOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawPatchOutOutElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawDigitalElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawAnalogElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint8_t select);

void drawSettingElement(entryStruct *entry, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t select,
                        uint8_t hugeFont = 0);

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
class Live_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->heigth = heigth;
        this->select = select;
    }
    void Draw();

    void addSettingsEntry(Setting *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] =
            entryStruct{SETTING, nullptr, nullptr, data, nullptr, nullptr, functionCW, functionCCW, functionPush};

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
    void addPatchEntry(PatchElementInOut *patch) { this->patch = patch; }
    void addEntry(BasePatch *entry) {

        this->entry = entry;
        active = 1;
    }

    uint8_t patched = 0;
    uint8_t active = 0;
    uint8_t select = 0;
    uint16_t panelAbsX;

    BasePatch *entry = nullptr;

    PatchElementInOut *patch = nullptr;

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

// PresetPanelElemnt for Presets
class Preset_PanelElement {
  public:
    void init(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->entryWidth = width;
        this->entryHeight = heigth;
    }
    void Draw();
    void addEntry(presetStruct *entry) {

        this->entry = entry;
        active = 1;
    }

    uint8_t active = 0;
    uint8_t select = 0;

    presetStruct *entry = nullptr;

  private:
    uint16_t panelAbsY;
    uint16_t panelAbsX;

    uint16_t entryWidth;
    uint16_t entryHeight;
};

// GUIHeader Box for Panel Selection
