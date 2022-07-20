#ifdef POLYCONTROL

#include "guiPanelStart.hpp"

void GUIPanelStart::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
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