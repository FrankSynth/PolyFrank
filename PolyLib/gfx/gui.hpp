#pragma once

#include "../datacore/datacore.hpp"
#include "gfx.hpp"
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

// responsive sizes
#define HEADERHEIGHT 50
#define FOOTERHEIGHT 50
#define FOCUSHEIGHT 35
#define SPACER 20

#define BOARDERWIDTH 150
#define CENTERWIDTH LCDWIDTH - BOARDERWIDTH * 2
#define CENTERHEIGHT LCDHEIGHT - HEADERHEIGHT - FOOTERHEIGHT - FOCUSHEIGHT - SPACER - SPACER - SPACER

typedef enum {
    FOCUSLAYER,
    FOCUSMODULE,
    FOCUSINPUT,
    FOCUSOUTPUT,
    FOCUSSETTING

} FOCUSMODE;
typedef struct {
    uint8_t layer = 0xff;
    uint8_t modul = 0xff;
    uint8_t id = 0xff;
    FOCUSMODE type;

} location;

typedef struct {
    std::function<void()> functionPointer = nullptr;
    std::string name = "";
} actionHandle;

class actionMapping {
  public:
    void registerActionHeader(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                              actionHandle handle3 = {nullptr, ""}, actionHandle handle4 = {nullptr, ""});

    void registerActionLeft(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                            actionHandle handle3 = {nullptr, ""});

    void registerActionRight(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                             actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder1(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});
    void registerActionEncoder2(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder3(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder4(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void callActionHeader1();

    void callActionHeader2();

    void callActionHeader3();

    void callActionHeader4();

    void callActionLeft1();

    void callActionLeft2();

    void callActionLeft3();

    void callActionRight1();

    void callActionRight2();

    void callActionRight3();

    void callActionEncoder_1_CW();

    void callActionEncoder_1_CCW();

    void callActionEncoder_1_Push();
    void callActionEncoder_2_CW();

    void callActionEncoder_2_CCW();

    void callActionEncoder_2_Push();
    void callActionEncoder_3_CW();
    void callActionEncoder_3_CCW();
    void callActionEncoder_3_Push();
    void callActionEncoder_4_CW();
    void callActionEncoder_4_CCW();

    void callActionEncoder_4_Push();

    actionHandle buttonHeader_1;
    actionHandle buttonHeader_2;
    actionHandle buttonHeader_3;
    actionHandle buttonHeader_4;

    actionHandle buttonLeft_1;
    actionHandle buttonLeft_2;
    actionHandle buttonLeft_3;

    actionHandle buttonRight_1;
    actionHandle buttonRight_2;
    actionHandle buttonRight_3;

    actionHandle encoder1_CW;
    actionHandle encoder1_CCW;
    actionHandle encoder1_Push;

    actionHandle encoder2_CW;
    actionHandle encoder2_CCW;
    actionHandle encoder2_Push;

    actionHandle encoder3_CW;
    actionHandle encoder3_CCW;
    actionHandle encoder3_Push;

    actionHandle encoder4_CW;
    actionHandle encoder4_CCW;
    actionHandle encoder4_Push;
};

extern actionMapping actionHandler;

uint16_t drawBoxWithText(std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText, uint16_t x,
                         uint16_t y, uint16_t heigth, uint16_t space, uint16_t champfer = 0,
                         FONTALIGN alignment = CENTER);
// base class for the Panels

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

  private:
};

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection
class Header_PanelBox {
  public:
    Header_PanelBox(GUIPanelBase *panel, uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->pSource = panel;
        this->x = x;
        this->y = y;
        this->width = width;
        this->heigth = heigth;
    }

    void Draw();

  private:
    const GUI_FONTINFO *font = &GUI_FontBahnschriftSemiBold28_FontInfo;

    GUIPanelBase *pSource;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t heigth;
    uint8_t selected = 0;
};
class Footer_PanelBox {
  public:
    Footer_PanelBox(actionHandle *mainFunction, actionHandle *secondaryFunction, uint16_t x, uint16_t y, uint16_t width,
                    uint16_t heigth) {
        this->main = mainFunction;
        this->secondary = secondaryFunction;

        this->x = x;
        this->y = y;
        this->width = width;
        this->heigth = heigth;
    }

    void Draw();

  private:
    const GUI_FONTINFO *fontBig = &GUI_FontBahnschriftSemiBold28_FontInfo;
    const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift24_FontInfo;

    actionHandle *main;
    actionHandle *secondary;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t heigth;
    uint16_t fontShift = 13;
};

class Side_PanelBox {
  public:
    Side_PanelBox(actionHandle *Function, uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->main = Function;

        this->x = x;
        this->y = y;
        this->width = width;
        this->heigth = heigth;
    }

    void Draw();

  private:
    const GUI_FONTINFO *fontBig = &GUI_FontBahnschriftSemiBold28_FontInfo;
    const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift24_FontInfo;

    actionHandle *main;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t heigth;
};

// GUIHeader Box for Panel Selection
class Focus_PanelDataElement {
  public:
    Focus_PanelDataElement(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->panelAbsX = x;
        this->panelAbsY = y;
        this->width = width;
        this->heigth = heigth;
    }

    void Draw();

    uint8_t selected = 0;
    Analog *pSource;

  private:
    const GUI_FONTINFO *font = &GUI_FontBahnschriftSemiBold28_FontInfo;

    uint16_t panelAbsX;
    uint16_t panelAbsY;

    uint16_t width;
    uint16_t heigth;

    uint16_t nameWidth = 140;
};

//////PANELS/////
// PANEL Path Type
class GUIPanelPath {
  public:
    void init(std::vector<Layer *> *layers, location *path, uint16_t width, uint16_t height, uint16_t x = 0,
              uint16_t y = 0);

    void Draw();

  private:
    // Boxes
    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    location *pLocation = nullptr;
    std::vector<Layer *> *layers;
    const GUI_FONTINFO *font = &GUI_FontBahnschriftSemiBold28_FontInfo;
};

// PANEL Path Type
class GUIPanelFocus : public GUIPanelBase {
  public:
    void init(std::vector<Layer *> *layers, location *path, uint16_t width, uint16_t height, uint16_t x = 0,
              uint16_t y = 0);
    void Draw();

    uint16_t updateEntrys();

    void registerModuleSettings();
    void registerModuleOutputs();
    void registerModuleInputs();

    void registerPanelSettings();

    void changeScroll(int16_t change);
    void checkScroll();
    void resetScroll();

  private:
    // Boxes
    uint16_t elementsNb = 5;

    FOCUSMODE mode;

    uint16_t entrys = 0;

    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    location *pLocation = nullptr;
    location oldLocation;
    uint16_t scroll = 0;
    uint8_t scrollOffset = 0;

    std::vector<Layer *> *layers;

    std::vector<Focus_PanelDataElement> elementsModule;
    std::vector<Focus_PanelDataElement> elementsOutput;
    std::vector<Focus_PanelDataElement> elementsInput;
};

// PANEL Config Type
class GUIPanelConfig : public GUIPanelBase {
  public:
    GUIPanelConfig(std::string name, uint8_t id) : GUIPanelBase(name, id){};

    void Reset();

    void Draw() {}
    void registerActions();

  private:
};

// Header
class GUIHeader {
  public:
    void init(std::vector<GUIPanelBase *> *panels, uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0);
    void Draw();

  private:
    // Boxes
    std::vector<Header_PanelBox> boxes;
    uint16_t panelWidth;
    uint16_t panelHeight;
    uint16_t panelCount;
    uint16_t panelAbsX;
    uint16_t panelAbsY;
};

// FOOTER
class GUIFOOTER {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = LCDHEIGHT);
    void Draw();

  private:
    // Boxes
    std::vector<Footer_PanelBox> boxes;

    uint16_t panelWidth;
    uint16_t panelHeight;
    uint16_t panelCount = 4;
    uint16_t panelAbsX;
    uint16_t panelAbsY;
};

// FOOTER
class GUISIDE {
  public:
    void init(uint16_t width = BOARDERWIDTH, uint16_t height = LCDHEIGHT, uint16_t y = 0);
    void Draw();

  private:
    // Boxes
    std::vector<Side_PanelBox> boxes;

    uint16_t panelWidth;
    uint16_t panelHeight;
    uint16_t panelCount = 3;
    uint16_t panelAbsX;
    uint16_t panelAbsY;
};

/// GUI///
class GUI {
  public:
    void Init(std::vector<Layer *> &layers);
    void Draw();
    void Clear();

    void setFocus(location newFocus);

    // PanelSelect
    void setPanel1Active();

    void setPanel2Active();

    void setPanel3Active();

    void setPanel4Active();
    void setPanelFocusActive();

    void nextLayer() { focus.layer = changeIntLoop(focus.layer, 1, 0, layers.size() - 1); }

    void setActivePanel(uint8_t id);
    GUIPanelBase *activePanel = nullptr;
    std::vector<Layer *> layers;

  private:
    GUIPanelConfig guiPanel_1 = GUIPanelConfig("LIVEMODE", 0);
    GUIPanelConfig guiPanel_2 = GUIPanelConfig("PATCH", 1);
    GUIPanelConfig guiPanel_3 = GUIPanelConfig("PRESET", 2);
    GUIPanelConfig guiPanel_4 = GUIPanelConfig("CONFIG", 3);

    std::vector<GUIPanelBase *> panels;

    GUIPanelFocus guiPanelFocus;

    GUIPanelPath guiPath;

    GUISIDE guiSide;

    GUIFOOTER guiFooter;
    GUIHeader guiHeader;

    location focus;
};

// GUI
extern GUI ui;
