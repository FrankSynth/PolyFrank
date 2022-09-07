#ifdef POLYCONTROL

#include "guiBase.hpp"

location currentFocus;
location newFocus;

// dont allow full color -> FF not allowed -> use F1 --> this reduce the display burning
//  DMA2D Colors
uint32_t cSelect = 0xFFF1F1F1;
uint32_t cDeselect = 0xFF080808;
uint32_t cFont_Select = 0xFF080808;
uint32_t cFont_Deselect = 0xFFF1F1F1;

uint32_t cWhite = 0xFFF1F1F1;
uint32_t cWhiteDark = 0xFF404040;
uint32_t cGreyLight = 0xFF202020;
uint32_t cGrey = 0xFF101010;
uint32_t cGreyDark = 0xFF080808;
uint32_t cBlack = 0xFF000000;

uint32_t cHighlight = 0xFFF1da47;
uint32_t cWarning = 0xFFF10000;

// Wavedraw
uint16_t c4444dot = 0xFfd4;

uint16_t c4444wavecolor = 0xFFFF;
uint16_t c4444wavecolorTrans = 0x8FFF;

uint16_t c4444gridcolor = 0x3FFF;
uint16_t c4444framecolor = 0xAFFF;

uint32_t drawBoxWithText(const std::string &text, const GUI_FONTINFO *font, uint32_t colorBox, uint32_t colorText,
                         uint32_t x, uint32_t y, uint32_t heigth, uint32_t space, uint32_t champfer,
                         FONTALIGN alignment) {
    uint32_t width = getStringWidth(text, font) + space; // text witdh + space

    drawRectangleChampfered(colorBox, x, y, width, heigth, champfer); // draw Box
    drawString(text, colorText, x + space / 2, y + (-(font->size) + heigth) / 2, font,
               LEFT); // draw text, height centered

    return width;
}
uint32_t drawBoxWithTextFixWidth(const std::string &text, const GUI_FONTINFO *font, uint32_t colorBox,
                                 uint32_t colorText, uint32_t x, uint32_t y, uint32_t width, uint32_t heigth,
                                 uint32_t space, uint32_t champfer, FONTALIGN alignment) {

    drawRectangleChampfered(colorBox, x - width, y, width, heigth, champfer); // draw Box
    drawString(text, colorText, x + space / 2 - width, y + (-(font->size) + heigth) / 2, font,
               LEFT); // draw text, height centered

    return width;
}
void drawScrollBar(uint32_t x, uint32_t y, uint32_t width, uint32_t heigth, uint32_t scroll, uint32_t entrys,
                   uint32_t viewable) {

    if (viewable >= entrys) {
        return;
    }
    drawRectangleChampfered(cGreyLight, x, y, width, heigth, 1); // draw Box
    float entryHeight = heigth / (float)entrys;
    uint32_t scrollBarHeight = entryHeight * viewable;
    uint32_t scrollBarPositionY = entryHeight * scroll;

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

void focusPatch(location focus) {

    if (focus.type == NOFOCUS) {
        return;
    }

    currentFocus.type = focus.type;
    currentFocus.modul = focus.modul;
    if (focus.type == FOCUSINPUT) {
        currentFocus.id = focus.id;
    }

    setPanelActive(1);
}

void Scroller::scroll(int32_t change) {

    if (position + change != 0) {
        position = std::clamp(position + change, (int32_t)0, (int32_t)entrys - 1);
    }
    else
        position = 0;

    if (position >= (int32_t)entrys) {
        position = 0;
        offset = 0;
    }

    if (entrys <= maxEntrysVisible) {
        offset = 0;
    }
    else if (position == (int32_t)entrys - 1) {
        offset = entrys - maxEntrysVisible;
    }
    else if (position == 0) {
        offset = 0;
    }

    else if (position >= (int32_t)(maxEntrysVisible + offset - 1)) {
        offset++;
    }
    else if (position < (offset + 1)) {
        offset--;
    }

    relPosition = position - offset;
}

void Scroller::setScroll(int32_t scrollPosition) {

    if (scrollPosition == 0) { // new position = 0
        position = 0;
        offset = 0;
    }
    else if (entrys == 0) { // zero entrys?
        position = 0;
        offset = 0;
    }
    else {
        position = testInt(scrollPosition, 0, entrys - 1);

        if (position - offset >= (int32_t)maxEntrysVisible) { // scroll
            offset = position - maxEntrysVisible / 2;
        }
        if (position - offset <= 0) { // scroll
            offset = position - maxEntrysVisible / 2;
        }
        offset = testInt(offset, 0, testInt(entrys - maxEntrysVisible, 0, 0xFF));
    }

    relPosition = position - offset;
}

// PanelSelect

uint8_t activePanelID = 0;
uint8_t oldActivePanelID = 0;
uint8_t panelChanged = 0;

void setPanelActive(uint8_t panelID) {

    // zurück zum letzen panel
    if (activePanelID == panelID) {
        activePanelID = oldActivePanelID;
    }
    else {
        oldActivePanelID = activePanelID;
        activePanelID = panelID;
    }

    panelChanged = 1;
}

#endif