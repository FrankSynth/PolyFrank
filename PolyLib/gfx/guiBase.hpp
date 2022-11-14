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

extern uint32_t cWhite;
extern uint32_t cWhiteLight;
extern uint32_t cWhiteDark;

extern uint32_t cGreyLight;
extern uint32_t cGrey;
extern uint32_t cGreyDark;

extern uint32_t cBlack;

extern uint32_t cHighlight;
extern uint32_t cWarning;

extern uint32_t cLayerA;
extern uint32_t cLayerB;
extern uint32_t cLayer;

extern uint32_t cBackground;

extern uint16_t c4444dot;
extern uint16_t c4444wavecolor;
extern uint16_t c4444wavecolorTrans;

extern uint16_t c4444gridcolor;
extern uint16_t c4444framecolor;

extern PatchElement *selectedPatch;
extern std::vector<Layer *> allLayers;

// LOCATION
typedef enum {
    FOCUSLAYER,
    FOCUSMODULE,
    FOCUSCONFIG,
    FOCUSINPUT,
    FOCUSOUTPUT,
    NOFOCUS

} FOCUSMODE;

typedef struct {
    uint8_t layer = 0xff;
    uint8_t modul = 0xff;
    uint8_t id = 0xff;
    FOCUSMODE type;

} location;

extern location cachedFocus;
// responsive sizes
#define HEADERHEIGHT 34
#define FOOTERHEIGHT 34
#define FOCUSHEIGHT 30
#define SPACER 2
#define SCROLLBARWIDTH 4

#define BOARDERWIDTH 25
#define VOICEHEIGHT 100

#define CENTERWIDTH LCDWIDTH - BOARDERWIDTH * 2
#define CENTERHEIGHT LCDHEIGHT - HEADERHEIGHT - FOOTERHEIGHT - FOCUSHEIGHT - SPACER - SPACER - SPACER

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
} quickViewStruct;

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

    int32_t position = 0;
    int32_t offset = 0;
    uint32_t entrys = 0;
    uint32_t maxEntrysVisible = 0;
    int32_t relPosition = 0;
};

inline Layer *getCachedLayer() {
    return allLayers[cachedFocus.layer];
}

inline std::vector<BaseModule *> &getCachedModules() {
    return allLayers[cachedFocus.layer]->modules;
}

inline BaseModule *getCachedModule() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul];
}

inline std::vector<Analog *> &getCachedAnalog() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->knobs;
}

inline std::vector<Digital *> &getCachedDigital() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->switches;
}

inline std::vector<PatchElement *> &getCachedInputs() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->inputs[cachedFocus.id]->patchesInOut;
}

inline std::vector<PatchElement *> &getCachedOutputs() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->outputs[cachedFocus.id]->patchesInOut;
}

inline uint32_t getCachedOutputsSize() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->outputs.size();
}

inline uint32_t getCachedInputsSize() {
    return allLayers[cachedFocus.layer]->modules[cachedFocus.modul]->inputs.size();
}
// PanelSelect
void setPanelActive(uint8_t panelID);
void nextLayer();
void focusUp();
void focusDown(location newFocus);
void focusPatch(location focus);

void setGUIColor(int32_t *colorSelection);

void Todo();

extern uint8_t activePanelID;
extern uint8_t oldActivePanelID;

extern uint8_t panelChanged;

extern location cachedFocus;
extern location currentFocus;
extern location newFocus;

extern quickViewStruct quickView;
extern elapsedMillis quickViewTimer;

extern uint32_t quickViewTimeout;
