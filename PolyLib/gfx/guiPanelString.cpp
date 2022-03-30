#ifdef POLYCONTROL

#include "guiPanelString.hpp"
#include "hardware/device.hpp"

extern devManager deviceManager;

void GUIPanelString::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
}

void GUIPanelString::Draw() {
    drawConsole(deviceManager.report(), 3, panelAbsX, panelAbsY, panelWidth, panelHeight);
}

#endif