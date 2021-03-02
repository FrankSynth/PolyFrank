#pragma once

#include "../datacore/datacore.hpp"
#include "gfx.hpp"
#include "guiActionHandler.hpp"
#include "layer/layer.hpp"
#include <functional>
#include <string>

extern uint32_t cSelect;
extern uint32_t cDeselect;
extern uint32_t cFont_Select;
extern uint32_t cFont_Deselect;
extern uint32_t cClear;
extern uint32_t cBlack;
extern uint32_t cGrey;

extern uint32_t cGreyLight;
extern uint32_t cGreyDark;

extern uint32_t cWhite;
extern uint32_t cWhiteLight;
extern uint32_t cWhiteMedium;
extern uint32_t cHighlight;
extern uint32_t cWhiteBright;

// responsive sizes
#define HEADERHEIGHT 36
#define FOOTERHEIGHT 44
#define FOCUSHEIGHT 30
#define SPACER 8
#define SCROLLBARWIDTH 4

#define BOARDERWIDTH 80
#define VOICEHEIGHT 100

#define CENTERWIDTH LCDWIDTH - BOARDERWIDTH * 2
#define CENTERHEIGHT LCDHEIGHT - HEADERHEIGHT - FOOTERHEIGHT - FOCUSHEIGHT - SPACER - SPACER - SPACER

typedef enum {
    FOCUSLAYER,
    FOCUSMODULE,
    FOCUSCONFIG,
    FOCUSINPUT,
    FOCUSOUTPUT,
    NOFOCUS

} FOCUSMODE;

typedef enum {
    ANALOG,
    DIGITAL,
    MODULE,
    PATCHINPUT,
    PATCHOUTPUT,
    // PATCHOUTOUT,
    EMPTY,
    SETTING

} DATAELEMENTTYPE;
typedef struct {
    uint8_t layer = 0xff;
    uint8_t modul = 0xff;
    uint8_t id = 0xff;
    FOCUSMODE type;

} location;

typedef struct {

    DATAELEMENTTYPE type;
    Analog *analog = nullptr;
    Digital *digital = nullptr;
    Setting *setting = nullptr;

    PatchElement *patch = nullptr;

    BaseModule *modules = nullptr;
    actionHandle functionCW;
    actionHandle functionCCW;
    actionHandle functionPush;

} entryStruct;

typedef struct {
    DATAELEMENTTYPE type;
    PatchElement *patch = nullptr;

} patchEntryStruct;

uint16_t drawBoxWithText(std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText, uint16_t x,
                         uint16_t y, uint16_t heigth, uint16_t space, uint16_t champfer = 0,
                         FONTALIGN alignment = CENTER);

uint16_t drawBoxWithTextFixWidth(std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText,
                                 uint16_t x, uint16_t y, uint16_t width, uint16_t heigth, uint16_t space,
                                 uint16_t champfer, FONTALIGN alignment);
void drawScrollBar(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth, uint16_t scroll, uint16_t entrys,
                   uint16_t viewable);

class GUIPanelBase {
  public:
    GUIPanelBase(std::string name = "", uint8_t id = 0) {
        this->name = name;
        this->id = id;
    }
    virtual void Draw(){};

    void setActive(uint16_t active) { this->active = active; };

    std::string name;
    uint8_t id;
    uint8_t active;
    uint8_t pathVisible = 0;

  private:
};

class Scroller {
  public:
    Scroller(uint16_t maxEntrysVisible) { this->maxEntrysVisible = maxEntrysVisible; }

    void scroll(int16_t change);
    void checkScroll() { scroll(0); }
    void resetScroll() {
        position = 0;
        offset = 0;
    };

    uint16_t position;
    uint16_t offset;
    uint16_t entrys;
    uint16_t maxEntrysVisible;
    uint16_t relPosition;
};

void nextLayer();
void focusUp();
void focusDown(location newFocus);

void Todo();

extern location currentFocus;
extern location newFocus;