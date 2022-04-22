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

extern uint32_t cGreyLight2;
extern uint32_t cGreyLight;
extern uint32_t cGreyDark;

extern uint32_t cWhite;
extern uint32_t cWhiteLight;
extern uint32_t cWhiteMedium;
extern uint32_t cHighlight;
extern uint32_t cWhiteBright;

extern uint32_t cBlue;
extern uint32_t cRed;

extern uint32_t cPatch;
extern uint32_t cWarning;

// responsive sizes
#define HEADERHEIGHT 36
#define FOOTERHEIGHT 44
#define FOCUSHEIGHT 30
#define SPACER 8
#define SCROLLBARWIDTH 4

#define BOARDERWIDTH 30
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
    virtual void activate(){};

    std::string name;
    uint8_t id;
    uint8_t active;
    uint8_t pathVisible = 0;

  private:
};

class Scroller {
  public:
    Scroller(uint16_t maxEntrysVisible = 0) { this->maxEntrysVisible = maxEntrysVisible; }

    void scroll(int16_t change);

    void setScroll(int16_t scrollPosition);

    void checkScroll() { scroll(0); }
    void resetScroll() {
        position = 0;
        offset = 0;
    };

    // TODO these give comparison warnings, can they ever be negative? Should be uint maybe
    int16_t position = 0;
    int16_t offset = 0;
    uint16_t entrys = 0;
    uint16_t maxEntrysVisible = 0;
    int16_t relPosition = 0;
};

// PanelSelect
void setPanelActive(uint8_t panelID);
void nextLayer();
void focusUp();
void focusDown(location newFocus);
void focusPatch(location focus);

void Todo();

extern uint8_t activePanelID;
extern uint8_t oldActivePanelID;

extern uint8_t panelChanged;

extern location currentFocus;
extern location newFocus;