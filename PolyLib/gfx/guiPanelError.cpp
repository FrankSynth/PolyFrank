#ifdef POLYCONTROL

#include "guiPanelError.hpp"

void GUIPanelError::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelError::Draw() {
    // register Action

    // register Panel Seetings Rigth
    actionHandler.registerActionRight(0,
                                      {std::bind(&Error::resetError, &globalSettings.error), "IGNORE"}); // Reset Error
    actionHandler.registerActionRight(1);
    actionHandler.registerActionRight(2);

    // register Panel Seetings LEFT
    actionHandler.registerActionLeft(0);
    actionHandler.registerActionLeft(1);
    actionHandler.registerActionLeft(2);
    drawString(globalSettings.error.errorMessage, cFont_Deselect, panelWidth / 2, panelHeight / 2, fontMedium, CENTER);
}

#endif