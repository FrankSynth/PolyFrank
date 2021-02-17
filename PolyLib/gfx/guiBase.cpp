#ifdef POLYCONTROL

#include "guiBase.hpp"

location currentFocus;
location newFocus;

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
uint32_t cWhiteMedium = 0x40FFFFFF;
uint32_t cWhiteBright = 0x80FFFFFF;
uint32_t cWhiteLight = 0x10FFFFFF;
uint32_t cHighlight = 0xF0db0000;

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
    currentFocus.layer = changeIntLoop(currentFocus.layer, 1, 0, 1); // anzahl der Layer festgelegt
}

void focusUp() {

    if (currentFocus.type == FOCUSINPUT || currentFocus.type == FOCUSOUTPUT) {
        newFocus = currentFocus;
        newFocus.type = FOCUSMODULE;
    }
    else if (currentFocus.type == FOCUSMODULE) {
        newFocus = currentFocus;
        newFocus.type = FOCUSLAYER;
    }
}

void focusDown(location focus) {

    if (focus.type == NOFOCUS) {
        return;
    }

    newFocus.type = focus.type;
    newFocus.modul = focus.modul;
    if (focus.type == FOCUSINPUT) {
        newFocus.id = focus.id;
    }
}

void Scroller::scroll(int16_t change) {

    if (position + change != 0) {
        position = testInt(position + change, 0, entrys - 1);
    }
    else
        position = 0;

    if (position >= entrys) {
        position = 0;
    }

    if (entrys <= maxEntrysVisible) {
        offset = 0;
    }
    else if (position == entrys - 1) {
        offset = entrys - maxEntrysVisible;
    }
    else if (position == 0) {
        offset = 0;
    }

    else if (position >= (maxEntrysVisible + offset - 1)) {
        offset++;
    }
    else if (position < (offset + 1)) {
        offset--;
    }

    relPosition = position - offset;
}

#endif