#pragma once

#include "GUIPanelData.hpp"
#include "datacore/datacore.hpp"
#include "gfx.hpp"
#include "guiActionHandler.hpp"
#include "guiBase.hpp"
#include "layer/layer.hpp"
#include "tim.h"
#include <functional>
#include <string>

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

//////PANELS/////
// PANEL Path Type
class GUIPanelPath {
  public:
    void init(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0);

    void Draw();

  private:
    // Boxes
    uint16_t panelWidth = 0;
    uint16_t panelHeight = 0;
    uint16_t panelAbsX = 0;
    uint16_t panelAbsY = 0;
    const GUI_FONTINFO *font = &GUI_FontBahnschriftSemiBold28_FontInfo;
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
    void Init();
    void Draw();
    void Clear();

    void setFocus(location newFocus);

    // PanelSelect
    void setPanel1Active();

    void setPanel2Active();

    void setPanel3Active();

    void setPanel4Active();
    void setPanelFocusActive();

    void setActivePanel(uint8_t id);
    GUIPanelBase *activePanel = nullptr;
    std::vector<Layer *> layers;

  private:
    GUIPanelConfig guiPanel_1 = GUIPanelConfig("LIVEMODE", 0);
    GUIPanelConfig guiPanel_2 = GUIPanelConfig("PATCH", 1);
    GUIPanelConfig guiPanel_3 = GUIPanelConfig("PRESET", 2);
    GUIPanelConfig guiPanel_4 = GUIPanelConfig("CONFIG", 3);

    std::vector<GUIPanelBase *> panels;

    GUIPanelData guiPanelData;

    GUIPanelPath guiPath;

    GUISIDE guiSide;

    GUIFOOTER guiFooter;
    GUIHeader guiHeader;
};

// GUI
extern GUI ui;
