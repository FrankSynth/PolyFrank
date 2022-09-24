#ifdef POLYCONTROL

#include "guiPanelLive.hpp"

void GUIPanelLive::registerSettingsElements() {
    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;
    uint8_t exit = false;

    size = pCategory->settings.size();

    while (true) {
        if (elementIndex >= LIVEPANELENTRYS) {
            break;
        }
        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = (pCategory->settings)[dataIndex];
                if (settingsElement->disable == 1) {
                    panelElements[elementIndex].addSettingsEntry(settingsElement, {nullptr, ""}, {nullptr, ""},
                                                                 {nullptr, ""});
                }
                else {
                    panelElements[elementIndex].addSettingsEntry(
                        settingsElement,
                        {std::bind(&Setting::increase, settingsElement, 1), settingsElement->getName()},
                        {std::bind(&Setting::decrease, settingsElement, -1), settingsElement->getName()},
                        {std::bind(&Setting::resetValue, settingsElement), "RESET"});
                }
                dataIndex++;
            }
            else {
                panelElements[elementIndex].addEmptyEntry(); // empty entry
                exit = true;
            }
        }
        elementIndex++;
        if (exit)
            break;
    }
    panelElements[scroll.relPosition].select = 1;
}

void GUIPanelLive::updateEntrys() {

    if (subPanelSelect == 0) {
        pCategory = &liveData.__liveSettingsLivemode;
    }
    if (subPanelSelect == 1) {
        pCategory = &liveData.arps[currentFocus.layer].__liveSettingsArp;
    }
    if (subPanelSelect == 2) {
        pCategory = &liveData.__liveSettingsLivemode;
    }

    entrys = ceil((float)pCategory->settings.size() / EntrysPerElement);

    scroll.entrys = entrys;
    scroll.checkScroll();
}

void GUIPanelLive::selectSubPanel(uint8_t subPanelSelect) {
    for (uint8_t i = 0; i < 6; i++) {
        subPanelSelection[i] = subPanelSelect == i;
    }
    this->subPanelSelect = subPanelSelect;
}

void GUIPanelLive::Draw() {
    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    registerSettingsElements();

    for (int i = 0; i < LIVEPANELENTRYS; i++) {
        if (subPanelSelect == 0) // live settings
        {
            panelElements[i].keyColor = cWhite;
        }
        else { // arp settings -> show layer color
            panelElements[i].keyColor = cLayer;
        }

        panelElements[i].Draw();
    }

    // drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scroll.offset,
    //               entrys, CONFIGPANELENTRYS);
}

void GUIPanelLive::registerPanelSettings() {
    actionHandler.registerActionEncoder(4, {std::bind(&Scroller::scroll, &(this->scroll), 1), "SCROLL"},
                                        {std::bind(&Scroller::scroll, &(this->scroll), -1), "SCROLL"}, {nullptr, ""});

    actionHandler.registerActionRightData(0);
    actionHandler.registerActionRightData(1);
    actionHandler.registerActionRightData(2);

    actionHandler.registerActionLeft(0, {std::bind(&LiveData::saveLiveDataSettings, &liveData), "SAVE"}, 1);
    actionHandler.registerActionLeft(1, {std::bind(&LiveData::loadLiveDataSettings, &liveData), "LOAD"}, 1);
    actionHandler.registerActionLeft(2);
}

void GUIPanelLive::init(uint32_t width, uint32_t height, uint32_t x, uint32_t y, std::string name, uint8_t pathVisible,
                        uint8_t subPanel) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->pathVisible = pathVisible;
    subPanelSelect = subPanel;

    // elements Sizes
    uint16_t elementWidth = width;
    uint16_t elementSpace = 2;
    uint16_t elementHeight = (height - (LIVEPANELENTRYS - 2) * elementSpace) / LIVEPANELENTRYS;

    // init Elements
    for (int i = 0; i < LIVEPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }
}

#endif