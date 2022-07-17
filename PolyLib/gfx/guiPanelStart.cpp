#ifdef POLYCONTROL

#include "guiPanelStart.hpp"

void GUIPanelStart::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelStart::Draw() {
    drawRectangleFill(cBlack, 0, 0, LCDWIDTH, LCDHEIGHT);
    copyBitmapToBuffer(bmstartscreen01, cHighlight, 0, 0);
}

#endif