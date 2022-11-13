#ifdef POLYCONTROL

#include "guiBase.hpp"

location cachedFocus;

location currentFocus;
location newFocus;

PatchElement *selectedPatch = nullptr;

quickViewStruct quickView;
elapsedMillis quickViewTimer;
uint32_t quickViewTimeout = 1000;

///////////COLOR///////////
//  DMA2D Colors  //RGB8888  // dont allow full color -> FF not allowed -> use F1 --> this reduce the display burning

uint32_t cWhite = 0x0;
uint32_t cWhiteLight = 0x0;
uint32_t cWhiteDark = 0x0;
uint32_t cGreyLight = 0x0;
uint32_t cGrey = 0x0;
uint32_t cGreyDark = 0x0;
uint32_t cBlack = 0x0;

uint32_t cBackground = 0x0;

uint32_t cHighlight = 0x0;
uint32_t cWarning = 0x0;

uint32_t cLayerA = 0x0;
uint32_t cLayerB = 0x0;
uint32_t cLayer = 0x0;

uint32_t cSelect = 0x0;
uint32_t cDeselect = 0x0;
uint32_t cFont_Select = 0x0;
uint32_t cFont_Deselect = 0x0;

///////WaveDraw//////// RGB4444
uint16_t c4444dot;

uint16_t c4444wavecolor;
uint16_t c4444wavecolorTrans;

uint16_t c4444gridcolor;
uint16_t c4444framecolor;

void setGUIColor(int32_t *colorSelection) {

    switch (*colorSelection) {
        case 0: // DEFAULT
            cWhite = 0xFFF1F1F1;
            cWhiteLight = 0xFF808080;
            cWhiteDark = 0xFF404040;
            cGreyLight = 0xC0201C20;
            cGrey = 0xb0100C10;
            cGreyDark = 0x80100C10;
            cBlack = 0xFF000000;

            cBackground = 0xff000000;

            cHighlight = 0xFFf8ca50;
            cWarning = 0xFFF10000;

            cLayerA = 0xFFe80052;
            cLayerB = 0xFF0093e8;
            cLayer = 0x00000000;

            cSelect = cWhite;
            cDeselect = cGreyDark;
            cFont_Select = cGreyDark;
            cFont_Deselect = cWhite;

            c4444dot = 0xFfc5;

            c4444wavecolor = 0xFDDD;
            c4444wavecolorTrans = 0x8FFF;

            c4444gridcolor = 0x3FFF;
            c4444framecolor = 0xAFFF;

            break;

        case 1: // NICE
            cWhite = 0xFF000000;
            cWhiteLight = 0xFFB0B0B0;
            cWhiteDark = 0xFFB0B0B0;
            cGreyLight = 0xFFD8D8D8;
            cGrey = 0xFFE8E8E8;
            cGreyDark = 0xFFF1F1F1;
            cBlack = 0xFFF1F1F1;

            cBackground = 0xFFF1F1F1;

            cHighlight = 0xFFd20023; // dark red
            // cHighlight = 0xFFEB8C32; // orange
            cWarning = 0xFFF10000;

            cLayerA = 0xffB0B0B0;
            cLayerB = 0xffB0B0B0;
            cLayer = 0x00000000;

            cSelect = cBlack;
            cDeselect = cWhite;
            cFont_Select = cBlack;
            cFont_Deselect = cWhite;

            c4444dot = 0Xfd02;

            c4444wavecolor = 0xF000;
            c4444wavecolorTrans = 0x8000;

            c4444gridcolor = 0xE000;
            c4444framecolor = 0xE000;

            break;
        default: PolyError_Handler("wrong color selection"); break;
    }
}

///////////////////////////

bool layerMergeMode = false;

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
    if (layerMergeMode) {
        currentFocus.layer = 0;
    }
    else {

        currentFocus.layer = changeIntLoop(cachedFocus.layer, 1, 0, 1);
    }
}

void focusUp() {

    if (cachedFocus.type == FOCUSINPUT || cachedFocus.type == FOCUSOUTPUT) {
        newFocus = cachedFocus;
        newFocus.type = FOCUSMODULE;
    }
    else if (cachedFocus.type == FOCUSMODULE) {
        newFocus = cachedFocus;
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

    setPanelActive(2);
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