#ifdef POLYCONTROL

#include "guiPanelConfig.hpp"

void GUIPanelConfig::registerGlobalSettings() {
    uint16_t size;

    uint16_t dataIndex = 0;
    uint16_t elementIndex = 0;
    uint8_t exit = false;

    size = globalSettings.__globSettingsMIDI.settings.size();

    while (true) {
        if (elementIndex >= CONFIGPANELENTRYS) {
            break;
        }
        panelElements[elementIndex].setName(&globalSettings.__globSettingsMIDI.category);

        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = globalSettings.__globSettingsMIDI.settings[dataIndex];

                panelElements[elementIndex].addSettingsEntry(
                    settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                    {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
                    {std::bind(&Setting::resetValue, settingsElement), "RESET"});
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
    size = globalSettings.__globSettingsSystem.settings.size();
    dataIndex = 0;
    exit = false;

    while (true) {
        if (elementIndex >= CONFIGPANELENTRYS) {
            break;
        }
        panelElements[elementIndex].setName(&globalSettings.__globSettingsSystem.category);

        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = globalSettings.__globSettingsSystem.settings[dataIndex];

                panelElements[elementIndex].addSettingsEntry(
                    settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                    {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
                    {std::bind(&Setting::resetValue, settingsElement), "RESET"});
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
    size = globalSettings.__globSettingsDisplay.settings.size();
    dataIndex = 0;
    exit = false;

    while (true) {
        if (elementIndex >= CONFIGPANELENTRYS) {
            break;
        }
        panelElements[elementIndex].setName(&globalSettings.__globSettingsDisplay.category);

        for (int x = 0; x < EntrysPerElement; x++) {

            if (dataIndex < size) {
                Setting *settingsElement = globalSettings.__globSettingsDisplay.settings[dataIndex];

                panelElements[elementIndex].addSettingsEntry(
                    settingsElement, {std::bind(&Setting::increase, settingsElement, 1), "NEXT"},
                    {std::bind(&Setting::decrease, settingsElement, -1), "NEXT"},
                    {std::bind(&Setting::resetValue, settingsElement), "RESET"});
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

    panelElements[scroll.position - scroll.offset].select = 1;
}

void GUIPanelConfig::updateEntrys() {
    entrys = 0;
    entrys += ceil((float)globalSettings.__globSettingsSystem.settings.size() / EntrysPerElement);

    entrys += ceil((float)globalSettings.__globSettingsDisplay.settings.size() / EntrysPerElement);
    entrys += ceil((float)globalSettings.__globSettingsMIDI.settings.size() / EntrysPerElement);

    scroll.entrys = entrys;
    scroll.checkScroll();
}

void GUIPanelConfig::Draw() {
    // register Panel Seetings.settings.
    registerPanelSettings();

    // update number ob entrys
    updateEntrys();

    registerGlobalSettings();

    for (int i = 0; i < CONFIGPANELENTRYS; i++) {
        panelElements[i].Draw();
    }

    drawScrollBar(panelAbsX + panelWidth - SCROLLBARWIDTH, panelAbsY, SCROLLBARWIDTH, panelHeight, scroll.offset,
                  entrys, CONFIGPANELENTRYS);
}

void GUIPanelConfig::registerPanelSettings() {
    actionHandler.registerActionEncoder1({std::bind(&Scroller::scroll, &(this->scroll), 1), "SCROLL"},
                                         {std::bind(&Scroller::scroll, &(this->scroll), -1), "SCROLL"}, {nullptr, ""});

    // register Panel Seetings Left
    actionHandler.registerActionLeft({nullptr, "RESET"}, // RESET
                                     {nullptr, ""}, {nullptr, ""});

    // register Panel Seetings Rigth
    actionHandler.registerActionRight({std::bind(&GlobalSettings::saveGlobalSettings, &globalSettings), "SAVE"}, // SAVE
                                      {nullptr, ""}, {nullptr, ""});
}

void GUIPanelConfig::init(uint16_t width, uint16_t height, uint16_t x, uint16_t y, std::string name, uint8_t id,
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
    uint16_t elementHeight = (height - (CONFIGPANELENTRYS - 2) * elementSpace) / CONFIGPANELENTRYS;

    // init Elements
    for (int i = 0; i < CONFIGPANELENTRYS; i++) {
        panelElements[i].init(panelAbsX, panelAbsY + (elementHeight + elementSpace) * i, elementWidth, elementHeight);
    }
}

#endif