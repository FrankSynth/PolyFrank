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

    reportString.reserve(2048);
}

void GUIPanelString::registerPanelSettings() {

    actionHandler.registerActionRight(0, {std::bind(&GUIPanelString::setPanel, this, 0), "DEVICE"});
    actionHandler.registerActionRight(1, {std::bind(&GUIPanelString::setPanel, this, 1), "CONSOLE"});
    actionHandler.registerActionRight(2, {std::bind(&GUIPanelString::setPanel, this, 2), "STATUS"});

    actionHandler.registerActionLeft(0, {std::bind(rebootToBooloader), "BOOT"}, true);
    actionHandler.registerActionLeft(1);
    actionHandler.registerActionLeft(2);

    actionHandler.registerActionFooter(0);
    actionHandler.registerActionFooter(1);
    actionHandler.registerActionFooter(2);
    actionHandler.registerActionFooter(3);

    actionHandler.registerActionEncoder(0);
    actionHandler.registerActionEncoder(1);
    actionHandler.registerActionEncoder(2);
    actionHandler.registerActionEncoder(3);
    actionHandler.registerActionEncoder(4);
    actionHandler.registerActionEncoder(5);
}
void GUIPanelString::Draw() {

    registerPanelSettings();

    if (subPanelID == 0) {
        deviceManager.report(reportString);
        drawConsoleString(&reportString, 3, panelAbsX, panelAbsY, panelWidth, panelHeight);
    }

    if (subPanelID == 1)
        drawConsole(consoleBuffer, 3, panelAbsX, panelAbsY, panelWidth, panelHeight);

    if (subPanelID == 2) {
        globalSettings.statusReport(reportString);
        drawConsoleString(&reportString, 3, panelAbsX, panelAbsY, panelWidth, panelHeight);
    }
}

void GUIPanelString::setPanel(uint8_t subPanelID) {
    this->subPanelID = subPanelID;
}

#endif