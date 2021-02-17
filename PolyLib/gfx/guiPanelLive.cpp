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
                        settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                        {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
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
        pCategory = &liveData.arps[currentFocus.layer]->__liveSettingsArp;
    }
    if (subPanelSelect == 2) {
        pCategory = &liveData.__liveSettingsLivemode;
    }

    entrys = ceil((float)pCategory->settings.size() / EntrysPerElement);

    scroll.entrys = entrys;
    scroll.checkScroll();
}

void GUIPanelLive::selectSubPanel(uint8_t subPanelSelect) {
    this->subPanelSelect = subPanelSelect;
}

void GUIPanelLive::Draw() {
    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    registerSettingsElements();

    for (int i = 0; i < LIVEPANELENTRYS; i++) {
        panelElements[i].Draw();
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scroll.offset,
                  entrys, CONFIGPANELENTRYS);
}

void GUIPanelLive::registerPanelSettings() {
    actionHandler.registerActionEncoder1({std::bind(&Scroller::scroll, &(this->scroll), 1), "SCROLL"},
                                         {std::bind(&Scroller::scroll, &(this->scroll), -1), "SCROLL"}, {nullptr, ""});

    // register Panel Seetings Rigth
    actionHandler.registerActionRight(
        {std::bind(&GUIPanelLive::selectSubPanel, this, 0), liveData.__liveSettingsLivemode.category},
        {std::bind(&GUIPanelLive::selectSubPanel, this, 1),
         liveData.arps[currentFocus.layer]->__liveSettingsArp.category},
        {std::bind(&GUIPanelLive::selectSubPanel, this, 2), globalSettings.__globSettingsDisplay.category});
    // register Panel Seetings Left

    if (liveData.voiceHandler.livemodeMergeLayer.value == 0 && globalSettings.multiLayer.value == 1) {
        actionHandler.registerActionLeft({nullptr, "SAVE"}, // SAVE
                                         {nullptr, ""}, {std::bind(nextLayer), "LAYER"});
    }
    else {
        actionHandler.registerActionLeft({nullptr, "SAVE"}, // SAVE
                                         {nullptr, ""}, {nullptr, ""});
    }
}

void GUIPanelLive::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id,
                        uint8_t pathVisible) {
    panelWidth = width;
    panelHeight = height;
    panelAbsX = x;
    panelAbsY = y;
    this->name = name;
    this->id = id;
    this->pathVisible = pathVisible;

    // elements Sizes
    uint16_t elementWidth = width - SCROLLBARWIDTH - 2;
    uint16_t elementSpace = 3;
    uint16_t elementHeight = (height - (LIVEPANELENTRYS - 2) * elementSpace) / LIVEPANELENTRYS;

    // init Elements
    for (int i = 0; i < LIVEPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }
}

#endif