#ifdef POLYCONTROL

#include "guiBase.hpp"

location focus;

// colors
uint32_t cSelect = 0xD0FFFFFF;
uint32_t cDeselect = 0x00000000;
uint32_t cFont_Select = 0xFF000000;
uint32_t cFont_Deselect = 0xFFFFFFFF;

uint32_t cClear = 0x00000000;
uint32_t cBlack = 0xFF000000;

uint32_t cGreyLight = 0x10FFFFFF;
uint32_t cGreyDark = 0x60202020;

uint32_t cGrey = 0x10808080;

uint32_t cWhite = 0xFFFFFFFF;
uint32_t cWhiteLight = 0xd0FFFFFF;

uint16_t drawBoxWithText(std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText, uint16_t x,
                         uint16_t y, uint16_t heigth, uint16_t space, uint16_t champfer, FONTALIGN alignment) {
    uint16_t width = getStringWidth(text, font) + space; // text witdh + space

    drawRectangleChampfered(colorBox, x, y, width, heigth, champfer); // draw Box
    drawString(text, colorText, x + space / 2, y + (-font->size + heigth) / 2, font,
               LEFT); // draw text, height centered

    return width;
}
void drawScrollBar(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth, uint16_t scroll, uint16_t entrys,
                   uint16_t viewable) {

    if (viewable >= entrys) {
        return;
    }
    drawRectangleChampfered(cGreyDark, x, y, width, heigth, 1); // draw Box
    float entryHeight = heigth / (float)entrys;
    uint16_t scrollBarHeight = entryHeight * viewable;
    uint16_t scrollBarPositionY = entryHeight * scroll;

    drawRectangleChampfered(cWhite, x, y + scrollBarPositionY, width, scrollBarHeight, 1); // draw Box
}

void Todo(){};

void nextLayer() {
    focus.layer = changeIntLoop(focus.layer, 1, 0, 1); // anzahl der Layer festgelegt
}

void focusUp() {

    if (focus.type == FOCUSINPUT || focus.type == FOCUSOUTPUT) {
        focus.type = FOCUSMODULE;
    }
    else if (focus.type == FOCUSMODULE) {
        focus.type = FOCUSLAYER;
    }
}

void focusDown(location newFocus) {
    if (newFocus.type == NOFOCUS) {
        return;
    }

    focus.type = newFocus.type;
    if (newFocus.type == FOCUSINPUT) {
        focus.id = newFocus.id;
    }
    focus.modul = newFocus.modul;
}

#endif