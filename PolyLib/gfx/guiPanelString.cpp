#ifdef POLYCONTROL

#include "guiPanelString.hpp"
#include "circularbuffer/circularbuffer.hpp"
#include "hardware/device.hpp"

extern devManager deviceManager;
extern CircularBuffer<char, 1024> consoleBuffer;

void GUIPanelString::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;

    name = "DEBUG";
}

void GUIPanelString::registerPanelSettings() {

    actionHandler.registerActionRight(0, {std::bind(&GUIPanelString::setPanel, this, 0), "DEVICE"});
    actionHandler.registerActionRight(1, {std::bind(&GUIPanelString::setPanel, this, 1), "CONSOLE"});
    actionHandler.registerActionRight(2);

    actionHandler.registerActionLeft(0);
    actionHandler.registerActionLeft(1);
    actionHandler.registerActionLeft(2);
}
void GUIPanelString::Draw() {

    registerPanelSettings();

    if (subPanelID == 0)
        drawDeviceManager(deviceManager.report(), 3, panelAbsX, panelAbsY, panelWidth, panelHeight);

    if (subPanelID == 1)
        drawConsole(consoleBuffer, 3, panelAbsX, panelAbsY, panelWidth, panelHeight);
}

void GUIPanelString::setPanel(uint8_t subPanelID) {
    this->subPanelID = subPanelID;
}

#endif