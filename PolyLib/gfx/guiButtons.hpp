#pragma once

#include "guiPanelBase.hpp"

///////DRAW ELEMENTS//////
// GUIHeader Box for Panel Selection
class Header_PanelBox {
  public:
    Header_PanelBox(uint32_t x, uint32_t y, uint32_t width, uint32_t heigth) {
        this->x = x;
        this->y = y;
        this->width = width;
        this->heigth = heigth;
    }

    void Draw(GUIPanelBase *panel, uint8_t active);

  private:
    const GUI_FONTINFO *font = &GUI_FontBahnschrift24_FontInfo;

    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t heigth;
    uint8_t selected = 0;
};
class Footer_PanelBox {
  public:
    Footer_PanelBox(actionHandle *mainFunction, actionHandle *secondaryFunction, uint32_t x, uint32_t y, uint32_t width,
                    uint32_t heigth) {
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
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t heigth;
    uint32_t fontShift = 0;
};

class Side_PanelBox {
  public:
    Side_PanelBox(ButtonActionHandle *actionHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t heigth) {
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
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t heigth;
};

// Header
class GUIHeader {
  public:
    void init(std::vector<GUIPanelBase *> *panels, uint8_t *activePanelID, uint32_t width, uint32_t height,
              uint32_t x = 0, uint32_t y = 0);
    void Draw();

  private:
    // Boxes
    uint8_t *activePanelID = nullptr;
    std::vector<GUIPanelBase *> *panels;
    std::vector<Header_PanelBox> boxes;
    uint32_t panelWidth;
    uint32_t panelHeight;
    uint32_t panelCount;
    uint32_t panelAbsX;
    uint32_t panelAbsY;
};

// FOOTER
class GUIFooter {
  public:
    void init(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = LCDHEIGHT);
    void Draw();

  private:
    // Boxes
    std::vector<Footer_PanelBox> boxes;

    uint32_t panelWidth;
    uint32_t panelHeight;
    uint32_t panelCount = 4;
    uint32_t panelAbsX;
    uint32_t panelAbsY;
};

// SIDE
class GUISide {
  public:
    void init(uint32_t width = BOARDERWIDTH, uint32_t height = LCDHEIGHT, uint32_t y = 0, uint32_t x = 0);
    void Draw();

  private:
    // Boxes
    std::vector<Side_PanelBox> boxes;

    uint32_t panelWidth;
    uint32_t panelHeight;
    uint32_t panelCount = 3;
    uint32_t panelAbsX;
    uint32_t panelAbsY;
};
