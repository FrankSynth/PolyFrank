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
    actionHandler.registerActionRight({std::bind(&Error::resetError, &globalSettings.error), "IGNORE"}, // Reset Error
                                      {nullptr, ""}, {nullptr, ""});
    // register Panel Seetings LEFT
    actionHandler.registerActionLeft({nullptr, ""}, {nullptr, ""}, {nullptr, ""});
    drawString(globalSettings.error.errorMessage, cFont_Deselect, panelWidth / 2, panelHeight / 2, fontMedium, CENTER);
}

#endif