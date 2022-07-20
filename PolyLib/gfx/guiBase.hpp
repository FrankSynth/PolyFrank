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
extern uint32_t cWhiteMedium;
extern uint32_t cHighlight;

extern uint32_t cWarning;

extern uint16_t c4444dot;
extern uint16_t c4444wavecolor;
extern uint16_t c4444wavecolorTrans;

extern uint16_t c4444gridcolor;
extern uint16_t c4444framecolor;

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

uint32_t drawBoxWithText(const std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText,
                         uint32_t x, uint32_t y, uint32_t heigth, uint32_t space, uint32_t champfer = 0,
                         FONTALIGN alignment = CENTER);

uint32_t drawBoxWithTextFixWidth(const std::string &text, const GUI_FONTINFO *font, uint32_t colorBox,
                                 uint32_t colorText, uint32_t x, uint32_t y, uint32_t width, uint32_t heigth,
                                 uint32_t space, uint32_t champfer, FONTALIGN alignment);

void drawScrollBar(uint32_t x, uint32_t y, uint32_t width, uint32_t heigth, uint32_t scroll, uint32_t entrys,
                   uint32_t viewable);

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
    Scroller(uint32_t maxEntrysVisible = 0) { this->maxEntrysVisible = maxEntrysVisible; }

    void scroll(int32_t change);

    void setScroll(int32_t scrollPosition);

    void checkScroll() { scroll(0); }
    void resetScroll() {
        position = 0;
        offset = 0;
    };

    // TODO these give comparison warnings, can they ever be negative? Should be uint maybe
    int32_t position = 0;
    int32_t offset = 0;
    uint32_t entrys = 0;
    uint32_t maxEntrysVisible = 0;
    int32_t relPosition = 0;
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