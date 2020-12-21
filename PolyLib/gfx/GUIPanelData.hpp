#pragma once

#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "guiActionHandler.hpp"
#include "guiBase.hpp"
#include "layer/layer.hpp"
#include "poly.hpp"
#include <functional>
#include <string>

#define DATAPANELENTRYS 5
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

    void addAnalogEntry(Analog *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] = entryStruct{ANALOG, data, nullptr, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addDigitalEntry(Digital *data, actionHandle functionCW, actionHandle functionCCW, actionHandle functionPush) {

        entrys[numberEntrys] = entryStruct{DIGITAL, nullptr, data, nullptr, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }

    void addPatchInputEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                            actionHandle functionPush) {

        entrys[numberEntrys] = entryStruct{PATCHINPUT, nullptr, nullptr, patch, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addPatchOutOutEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                             actionHandle functionPush) {

        entrys[numberEntrys] = entryStruct{PATCHOUTOUT, nullptr, nullptr, patch, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addPatchOutputEntry(PatchElement *patch, actionHandle functionCW, actionHandle functionCCW,
                             actionHandle functionPush) {

        entrys[numberEntrys] = entryStruct{PATCHOUTPUT, nullptr, nullptr, patch, functionCW, functionCCW, functionPush};

        numberEntrys++;
        visible = 1;
    }
    void addEmptyEntry() {

        entrys[numberEntrys] =
            entryStruct{EMPTY, nullptr, nullptr, nullptr, {nullptr, ""}, {nullptr, ""}, {nullptr, ""}};

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

class GUIPanelData : public GUIPanelBase {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0);
    void Draw();

    uint16_t updateEntrys();

    void registerModuleSettings();
    void registerModulePatchIn();
    void registerModulePatchOut();

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

    Data_PanelElement panelElements[DATAPANELENTRYS];
};
