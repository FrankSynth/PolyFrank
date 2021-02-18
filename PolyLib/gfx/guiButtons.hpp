#pragma once

#include "guiPanelBase.hpp"

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

    void Draw(uint8_t active);

  private:
    const GUI_FONTINFO *font = &GUI_FontBahnschrift24_FontInfo;

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
    const GUI_FONTINFO *fontBig = &GUI_FontBahnschrift24_FontInfo;
    const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift12_FontInfo;

    actionHandle *main;
    actionHandle *secondary;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t heigth;
    uint16_t fontShift = 0;
};

class Side_PanelBox {
  public:
    Side_PanelBox(ButtonActionHandle *actionHandle, uint16_t x, uint16_t y, uint16_t width, uint16_t heigth) {
        this->actionHandle = actionHandle;

        this->x = x;
        this->y = y;
        this->width = width;
        this->heigth = heigth;
    }

    void Draw();

  private:
    const GUI_FONTINFO *fontBig = &GUI_FontBahnschriftSemiBold28_FontInfo;
    const GUI_FONTINFO *fontSmall = &GUI_FontBahnschrift24_FontInfo;

    ButtonActionHandle *actionHandle;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t heigth;
};

// Header
class GUIHeader {
  public:
    void init(std::vector<GUIPanelBase *> *panels, uint8_t *activePanelID, uint16_t width, uint16_t height,
              uint16_t x = 0, uint16_t y = 0);
    void Draw();

  private:
    // Boxes
    uint8_t *activePanelID = nullptr;
    std::vector<Header_PanelBox> boxes;
    uint16_t panelWidth;
    uint16_t panelHeight;
    uint16_t panelCount;
    uint16_t panelAbsX;
    uint16_t panelAbsY;
};

// FOOTER
class GUIFooter {
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

// SIDE
class GUISide {
  public:
    void init(uint16_t width = BOARDERWIDTH, uint16_t height = LCDHEIGHT, uint16_t y = 0, uint16_t x = 0);
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
